/*
 * IgnMan.cpp
 *
 *  Created on: 26 Dec 2018
 *      Author: xasin
 */

#include "IgnMan.h"

#include "freertos/task.h"
#include "esp_system.h"

#include <cstring>

IgnMan::IgnMan(gpio_num_t fire, gpio_num_t pwrDetect, gpio_num_t btnPin)
	: fireworksDevice(nullptr),
	  pin_pwr_detect(pwrDetect), pin_btn(btnPin), pin_fire(fire),
	  stdbyOn(0), lastFired(-1), fireRequest(-1), fireCountdown(0),
	  btn_press_time(0),
	  lastConStatus(UNPOWERED),
	  mqtt_client(nullptr),
	  armed(false), armPercent(-1) {
}

void IgnMan::init() {
	spi_bus_config_t busCFG = {
			mosi_io_num: 23,
			miso_io_num: -1,
			sclk_io_num: 18,
			quadwp_io_num: -1,
			quadhd_io_num: -1,
			max_transfer_sz: 4,
	};

	spi_bus_initialize(VSPI_HOST, &busCFG, 0);

	spi_device_interface_config_t deviceCFG = {
			command_bits: 0,
			address_bits: 0,
			dummy_bits: 0,
			mode: 0,
			duty_cycle_pos: 0,
	};

	deviceCFG.flags = 0;
	deviceCFG.queue_size = 2;
	deviceCFG.pre_cb = nullptr;
	deviceCFG.post_cb = nullptr;


	deviceCFG.clock_speed_hz = 100000;
	deviceCFG.spics_io_num = 5;

	spi_bus_add_device(VSPI_HOST, &deviceCFG, &fireworksDevice);

	gpio_set_direction(pin_pwr_detect, GPIO_MODE_INPUT);
	gpio_set_direction(pin_btn, GPIO_MODE_INPUT);

	gpio_set_pull_mode(pin_pwr_detect, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(pin_btn, GPIO_PULLUP_ONLY);


	gpio_set_level(pin_fire, false);
	gpio_set_direction(pin_fire, GPIO_MODE_OUTPUT);

	raw_write(0, 0);
}

void IgnMan::mqtt_reconnect() {
	char c = get_con_status();
	esp_mqtt_client_publish(mqtt_client, "Xasin/NYEv3/Connection", &c, 1, 1, true);

	c = stdbyOn;
	esp_mqtt_client_publish(mqtt_client, "Xasin/NYEv3/StandbyOn", &c, 1, 1, true);
}

void IgnMan::send_fire_pulse() {
	gpio_set_level(pin_fire, true);
	vTaskDelay(6);
	gpio_set_level(pin_fire, false);
}

bool IgnMan::is_pressed() {
	return gpio_get_level(pin_btn) != 0;
}
bool IgnMan::is_powered() {
	return gpio_get_level(pin_pwr_detect) == 0;
}
uint8_t IgnMan::get_con_status() {
	return lastConStatus;
}

void IgnMan::set_arm(bool armStatus) {
	if(armStatus == armed)
		return;
	if(!is_powered() && armStatus)
		return;

	armed = armStatus;

	fireRequest = -1;
	fireCountdown = 0;
	armPercent = -1;

	set_stdby(armed ? 1 : 0);
}

void IgnMan::raw_write(uint8_t neByte, uint8_t peByte) {
	gpio_set_level(pin_fire, false);
	vTaskDelay(1);

	spi_transaction_t spiTrans;

	spiTrans.length = 16;
	spiTrans.tx_data[0] = peByte;
	spiTrans.tx_data[1] = neByte;

	spiTrans.rx_buffer = nullptr;
	spiTrans.rxlength = 0;

	spiTrans.flags = SPI_TRANS_USE_TXDATA;

	spi_device_transmit(fireworksDevice, &spiTrans);
	vTaskDelay(1);
}

void IgnMan::raw_target(uint8_t id) {
	raw_write(1<<(id/8), 1<<(id%8));
}

void IgnMan::raw_ignite(uint8_t id) {
	if(!is_powered())
		return;

	lastFired = id;

	raw_target(id);
	send_fire_pulse();
	return_stdby();
}

void IgnMan::return_stdby() {
	if(stdbyOn != 0) {
		raw_write(1, stdbyOn);
		gpio_set_level(pin_fire, true);
	}
	else
		raw_write(0, 0);
}

void IgnMan::mqtt_repub_status() {
	CONN_STATUS status;

	if(!is_powered())
		status = UNPOWERED;
	else if(!armed)
		status = UNARMED;
	else if(fireRequest == -1)
		status = ARMED;
	else
		status = FIRING;

	if(status != lastConStatus) {
		lastConStatus = status;

		if(mqtt_client != nullptr)
			esp_mqtt_client_publish(mqtt_client, "Xasin/NYEv3/Connection", reinterpret_cast<char *>(&status), 1, 1, true);
	}
}

void IgnMan::set_stdby(uint8_t stdbyBits)  {
	if(stdbyBits == stdbyOn)
		return;

	stdbyOn = stdbyBits;
	return_stdby();

	esp_mqtt_client_publish(mqtt_client, "Xasin/NYEv3/StandbyOn", reinterpret_cast<char *>(&stdbyOn), 1, 1, true);
}

void IgnMan::ignite(int8_t id) {
	if(!is_powered())
		return;
	if(id < -1)
		return;

	if(id == -1)
		id = lastFired+1;

	// Three second ignition fuse
	fireCountdown = -7000;
	fireRequest = id;

	uint16_t cDownTime = (-fireCountdown)*10 / 6;

	esp_mqtt_client_publish(mqtt_client, "Xasin/NYEv3/Countdown", reinterpret_cast<char *>(&cDownTime), 2, 0, false);
}

bool IgnMan::is_firing() {
	return fireRequest != -1;
}
long int IgnMan::get_remaining_ms() {
	return fireCountdown;
}

void IgnMan::ignition_thread() {
	TickType_t lastTick;

	bool was_powered = false;
	while(1) {
		vTaskDelayUntil(&lastTick, 60);

		if(is_powered() != was_powered) {
			was_powered = is_powered();

			set_arm(false);
		}

		if(is_pressed() && (get_con_status() == ARMED)) {
			if(btn_press_time++ == 20)
				this->ignite(-1);
		}
		else
			btn_press_time = 0;

		if(!is_powered()) {}
		else if(!armed) {fireRequest = -1;}
		else if(fireRequest != -1){
			fireCountdown += 100;

			if((fireRequest >= 0) && (fireCountdown >= 0)) {
				raw_ignite(fireRequest);
				fireRequest = -2;
			}
			else if(fireCountdown >= 5000) {
				fireRequest = -1;
				fireCountdown = 0;

				uint16_t dummy = 0;
				esp_mqtt_client_publish(mqtt_client, "Xasin/NYEv3/Countdown", reinterpret_cast<char *>(&dummy), 2, 1, false);
			}
		}

		mqtt_repub_status();
	}
}
