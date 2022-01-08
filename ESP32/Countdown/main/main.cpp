#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include <cmath>

#include <xnm/neocontroller.h>
#include <xnm/net_helpers.h>
#include <xasin/mqtt.h>

#include <esp_log.h>

#include "main.h"
#include "SegMan.h"

#include <ctime>

#define NYE_UNIX_TIMESTAMP 1640991599

// #define NYE_UNIX_TIMESTAMP 1640966459 // Debug timestamp

volatile long int nye_countdown_time = 0;

using namespace XNM::Neo;

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

NeoController rgbController = NeoController(GPIO_NUM_13, RMT_CHANNEL_0, 3*14);

int8_t sSegScrollPos = 30;

SegMan testMan = SegMan(3, rgbController);

esp_err_t event_handler(void *context, system_event_t *event)
{
    XNM::NetHelpers::event_handler(event);
    return ESP_OK;
}

void animation_thread(void *args) {
	TickType_t delayVariable = 0;

	rgbController.init();

	while(true) {
		nye_countdown_time -= 10;

		testMan.write_countdown_ms(nye_countdown_time);

		testMan.update_tick();
		rgbController.update();

		vTaskDelayUntil(&delayVariable, 6);
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
	// Initialize NVS — it is used to store PHY calibration data
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_event_loop_create_default();
    esp_event_loop_init(event_handler, nullptr);	


    TaskHandle_t animatorHandle;
    xTaskCreatePinnedToCore(&animation_thread, "Animator", 1024*5, nullptr, 10, &animatorHandle, 1);

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

	XNM::NetHelpers::WIFI::set_nvs("TP-LINK_84CDC2.5", "f36eebda48");
	Xasin::MQTT::Handler::set_nvs_uri("mqtts://xaseiresh.hopto.org");

	vTaskDelay(1000);

    XNM::NetHelpers::init_global_r3_ca();
	XNM::NetHelpers::init();

    testMan.transitMode = SegMan::SEGMENTS_DELAYED_PARALLEL;
    testMan.transitSpeed = 200;
    testMan.onColors = Layer(14*3);

    std::time_t curTime;
    std::time(&curTime);

    long int lastTimestamp = 0;

    while (true) {        
		std::time(&curTime);
        if(curTime != lastTimestamp) {
        	lastTimestamp = curTime;
        	nye_countdown_time = (lastTimestamp - NYE_UNIX_TIMESTAMP)*1000;

        	testMan.brightness_tgt.fill(curTime & 1 ? 0x666666 : 0xAAAAAA);
        }

        vTaskDelayUntil(&secondTicks, 60);
    }
}

