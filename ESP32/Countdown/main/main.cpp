#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <vector>

#include "main.h"

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
Layer		  digitLayer = Layer(14);

esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
	puts("WiFi STA started!");
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	puts("WiFi connected!");
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

	//memcpy(sta_cfg->password, WIFI_PASSWD, strlen(WIFI_PASSWD));
	//memcpy(sta_cfg->ssid, WIFI_SSID, strlen(WIFI_SSID));

	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg) );
	ESP_ERROR_CHECK( esp_wifi_start() );
}

void animation_thread(void *args) {
	TickType_t delayVariable = 0;

	while(true) {
		rgbController.colors.merge_overlay(digitLayer);
		rgbController.update();

		vTaskDelayUntil(&delayVariable, 10);
	}
}

void set_digit(uint8_t segCode, Layer &modLayer) {
	for(uint8_t i=0; i<7; i++) {
		if(((segCode >>i) & 1) != 0) {
			modLayer[2*i].alpha = 255<<7;
			modLayer[2*i + 1].alpha = 255<<7;
		}
		else {
			modLayer[2*i].alpha = 0;
			modLayer[2*i + 1].alpha = 0;
		}
	}
}

extern "C"
void app_main(void)
{
	setup_wifi();

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;

    digitLayer.alpha = 10;

    TaskHandle_t animatorHandle;
    xTaskCreate(&animation_thread, "Animator", 1024*5, nullptr, 10, &animatorHandle);

    Layer colorLayer = digitLayer;
    colorLayer.fill(Color(Material::GREEN, 100));

    while (true) {
    	digitLayer.fill(0x110000);
    	set_digit(sSegCodes[level%10], colorLayer);
    	digitLayer.merge_overlay(colorLayer);

        level++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

