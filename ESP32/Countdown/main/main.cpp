#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/apps/sntp.h"

#include "mqtt_client.h"

#include <ctime>

#include <string>
#include <vector>

#include "main.h"
#include "SegMan.h"

#include "IgnMan.h"

#define NYE_UNIX_TIMESTAMP 1546297199

volatile long int nye_countdown_time = 0;

using namespace Peripheral;

NeoController rgbController = NeoController(GPIO_NUM_14, RMT_CHANNEL_0, 3*14);

SegMan testMan = SegMan(3, rgbController);
IgnMan ignition = IgnMan(GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_32);

volatile int mqtt_ign_request = -1;

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    char servName[] = "pool.ntp.org\0";

	switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
	puts("WiFi STA started!");
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	puts("WiFi connected!");
    	esp_mqtt_client_start(ignition.mqtt_client);

        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, servName);
        sntp_init();

    	break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
    	puts("WiFi disconnected!");
    	esp_wifi_connect();
    	break;
    default:  break;
	}

	return ESP_OK;
}

esp_err_t mqtt_evt_handle(esp_mqtt_event_handle_t event) {
	switch(event->event_id) {
	case MQTT_EVENT_CONNECTED: {
		puts("MQTT connected!");

		esp_mqtt_client_subscribe(event->client, "Xasin/NYEv3/#", 2);
		ignition.mqtt_reconnect();
		break;
	}
	case MQTT_EVENT_DATA: {
		std::string topic(event->topic, event->topic_len);
		if(topic == std::string("Xasin/NYEv3/Fire")) {
			ignition.ignite(*reinterpret_cast<int8_t *>(event->data));
		}
		if(topic == std::string("Xasin/NYEv3/Arm")) {
			bool isArmed = *reinterpret_cast<bool *>(event->data);
			ignition.set_arm(isArmed);
		}
	}
	break;
	default: break;
	}

	return ESP_OK;
}

void setup_wifi() {
	nvs_flash_init();
	tcpip_adapter_init();

	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

	wifi_config_t wifi_cfg = {};
	wifi_sta_config_t* sta_cfg = &(wifi_cfg.sta);

	memcpy(sta_cfg->password, WIFI_PASSWD, strlen(WIFI_PASSWD));
	memcpy(sta_cfg->ssid, WIFI_SSID, strlen(WIFI_SSID));

	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg) );
	ESP_ERROR_CHECK( esp_wifi_start() );

	setenv("TZ", "GMT+1", 1);
	tzset();

	esp_mqtt_client_config_t mqtt_cfg = {};
	mqtt_cfg.event_handle = mqtt_evt_handle;
	mqtt_cfg.uri = "mqtt://iot.eclipse.org";

	mqtt_cfg.lwt_topic = "Xasin/NYEv3/Connection";
	mqtt_cfg.lwt_msg_len = 0;
	mqtt_cfg.keepalive = 10;
	mqtt_cfg.lwt_retain = true;

	auto mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
	ignition.mqtt_client = mqtt_handle;
}


void animation_thread(void *args) {
	TickType_t delayVariable = 0;

	while(true) {
		vTaskDelayUntil(&delayVariable, 10);
//        for(int i=0; i<14; i++) {
//        	testMan.onColors[i] = Color::HSV(SegMan::get_led_pos(i).y*10 + xTaskGetTickCount()/30);
//        }

		if(ignition.is_firing())
			testMan.write_countdown_ms(ignition.get_remaining_ms());
		else
			testMan.write_countdown_ms(nye_countdown_time);

		testMan.update_tick();
		rgbController.update();

		continue;
	}
}

void ignition_thread(void *data) {
	ignition.ignition_thread();
}

extern "C"
void app_main(void)
{
	setup_wifi();

    TaskHandle_t animatorHandle;
    xTaskCreatePinnedToCore(&animation_thread, "Animator", 1024*5, nullptr, 10, &animatorHandle, 0);

    TaskHandle_t ignitionHandle;
    xTaskCreate(&ignition_thread, "Ignition", 1024*2, nullptr, 3, &ignitionHandle);

    Color colorTypes[] = {
    		Material::GREEN,
			Material::YELLOW,
			Material::AMBER,
			Material::ORANGE,
			Material::RED
    };

    Color rainbowColor[] = {
    		Material::RED,
			Material::YELLOW,
			Material::GREEN,
			Material::CYAN,
			Material::BLUE,
			Material::PINK,
    };


    TickType_t secondTicks = 0;

    testMan.transitMode = SegMan::SEGMENTS_DELAYED_PARALLEL;
    testMan.transitSpeed = 200;
    testMan.onColors = Layer(14*3);

    ignition.init();

    std::time_t curTime;
    std::time(&curTime);

    long int lastTimestamp = 0;

    while (true) {

    	nye_countdown_time -= 10;
        std::time(&curTime);
        if(curTime != lastTimestamp) {
        	lastTimestamp = curTime;
        	nye_countdown_time = (lastTimestamp - NYE_UNIX_TIMESTAMP)*1000;
        	testMan.beat();
        }

        vTaskDelayUntil(&secondTicks, 6);
    }
}

