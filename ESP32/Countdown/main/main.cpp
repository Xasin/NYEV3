#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/apps/sntp.h"

#include <ctime>

#include <cstring>
#include <vector>

#include "main.h"
#include "SegMan.h"

using namespace Peripheral;

const uint8_t sSegCodes[] = {
		0b0111111,
		0b0001100,
		0b1110110,
		0b1011110,
		0b1001101,
		0b1011011,
		0b1111011,
		0b0001111,
		0b1111111,
		0b1011111
};

NeoController rgbController = NeoController(GPIO_NUM_14, RMT_CHANNEL_0, 14);

Layer	tgtDigitLayer = Layer(14);
Layer	isDigitLayer = Layer(14);

Layer	tgtBControlLayer = Layer(14);
Layer	isBControlLayer = Layer(14);

int8_t sSegScrollPos = 30;

SegMan testMan = SegMan(1, rgbController);

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
}

void animation_thread(void *args) {
	TickType_t delayVariable = 0;

	while(true) {
        for(int i=0; i<14; i++) {
        	testMan.onColors[i] = Color::HSV(SegMan::get_led_pos(i).y*10 + xTaskGetTickCount()/30);
        }

		testMan.update_tick();
		rgbController.update();
		vTaskDelayUntil(&delayVariable, 10);
		continue;

		if(sSegScrollPos < 28) {

			isDigitLayer[sSegScrollPos/2].merge_add(0x101010);
			isBControlLayer[sSegScrollPos/2].merge_add(0x111111);
			tgtDigitLayer[sSegScrollPos/2].alpha = 255;

			sSegScrollPos++;
		}

		isDigitLayer.merge_overlay(tgtDigitLayer);
		isBControlLayer.merge_overlay(tgtBControlLayer);

		rgbController.colors = isDigitLayer;
		rgbController.colors.merge_multiply(isBControlLayer);

		rgbController.update();
	}
}

void set_digit(uint8_t segCode, Layer &modLayer, Color onColor = 0xFFFFFF, Color offColor = 0) {
	for(uint8_t i=0; i<7; i++) {
		if(((segCode >>i) & 1) != 0) {
			modLayer[2*i].merge_overlay(onColor);
			modLayer[2*i + 1].merge_overlay(onColor);
		}
		else {
			modLayer[2*i].merge_overlay(offColor);
			modLayer[2*i + 1].merge_overlay(offColor);
		}
	}
}

extern "C"
void app_main(void)
{
	setup_wifi();

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;

    tgtDigitLayer.alpha = 100;

    tgtBControlLayer.alpha = 4;
    tgtBControlLayer.fill(0x999999);

    TaskHandle_t animatorHandle;
    xTaskCreatePinnedToCore(&animation_thread, "Animator", 1024*5, nullptr, 10, &animatorHandle, 0);

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

    testMan.transitMode = SegMan::SWIPE;
    testMan.transitSpeed = 15;
    testMan.onColors = Layer(14);

    std::time_t curTime;
    std::time(&curTime);
    while (true) {
        std::time(&curTime);
        level = std::localtime(&curTime)->tm_sec;

        testMan.write_number(level);
        testMan.beat();

        vTaskDelayUntil(&secondTicks, 600);
        continue;

        printf("Current time is: %d\n", int(curTime));

    	uint8_t segCode = sSegCodes[(level)%10];

    	set_digit(segCode, tgtDigitLayer, Color::HSV(6*std::localtime(&curTime)->tm_min), 0);
    	for(uint8_t i=0; i<14; i++)
    		tgtDigitLayer[i].alpha = 0;

    	//set_digit(segCode, isDigitLayer, 0x00FF00, Color(0, 0, 0));

    	isBControlLayer.fill(0xBBBBBB);
    	sSegScrollPos = 0;
    }
}

