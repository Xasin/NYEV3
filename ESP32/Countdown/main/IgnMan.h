/*
 * IgnMan.h
 *
 *  Created on: 26 Dec 2018
 *      Author: xasin
 */

#ifndef MAIN_IGNMAN_H_
#define MAIN_IGNMAN_H_

#include "driver/spi_master.h"

#include "mqtt_client.h"

class IgnMan {
private:
	enum CONN_STATUS : uint8_t {
		UNPOWERED = 2,
		UNARMED = 3,
		ARMED	= 4,
		FIRING	= 5,
	};

	spi_device_handle_t fireworksDevice;

	const gpio_num_t pin_pwr_detect;
	const gpio_num_t pin_btn;
	const gpio_num_t pin_fire;

	uint8_t stdbyOn;

	int8_t lastFired;
	volatile int8_t fireRequest;

	int fireCountdown;

	int btn_press_time;

	CONN_STATUS lastConStatus;

	void send_fire_pulse();

	bool is_connected();

	void raw_write(uint8_t neByte, uint8_t peByte);
	void raw_target(uint8_t id);

	void raw_ignite(uint8_t id);

	void return_stdby();

	void mqtt_repub_status();
public:
	esp_mqtt_client_handle_t mqtt_client;

	bool 	armed;
	int8_t  armPercent;

	IgnMan(gpio_num_t fire, gpio_num_t pwrDetect, gpio_num_t btnPin);

	void init();

	void mqtt_reconnect();

	bool is_pressed();
	bool is_powered();
	uint8_t get_con_status();

	void set_arm(bool armStatus);

	void set_stdby(uint8_t stdbyBits);

	void ignite(int8_t id);

	bool is_firing();
	long int  get_remaining_ms();

	void ignition_thread();
};

#endif /* MAIN_IGNMAN_H_ */
