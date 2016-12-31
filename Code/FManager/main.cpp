/*
 * main.cpp
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#include <avr/io.h>
#include <util/delay.h>

#include "AVR/Interfacing/Shift/ShiftReg.h"
#include "Localcode/Selector/Selector.h"
#include "Localcode/FManager/FManager.h"
#include "Localcode/SoundManager/SoundManager.h"

#include "AVR/TIMER/Timer1.h"

using namespace Interfacing;
using namespace Fireworks;

#define BTN_ON 		PORTB |= (1<< PB1);
#define BTN_OFF		PORTB &= ~(1<<PB1);
#define BTN_STATE 	(PINB & (1<< PB0))

#define POWER_STATE	(PIND & (1<< PD4))

ShiftReg testreg = ShiftReg(&PORTD, PD5);

ISR(TIMER1_COMPA_vect) {
	Manager::update();
	Sound::update();
}

void wait_seconds(uint16_t seconds) {
	while(--seconds != 0)
		_delay_ms(1000);
}

void init() {
	DDRB |= (1 << PB5 | 1 << PB3 | 1 << PB1);
	DDRD |= (1 << PD2);
	PORTD |= (1<< PD4);

	Manager::update();
	Sound::init();

	Timer1::enable_CTC(100);
}

enum FWMode : uint8_t {arming, button_wait, firing};
FWMode fireworksMode = arming;


void state_arming() {
	Sound::start_wailing(10);

	while(POWER_STATE != 0) {
		Sound::set_frequency(1000, 2);
		BTN_ON;
		_delay_ms(100);
		BTN_OFF;
		_delay_ms(1900);
	}

	Sound::start_wailing(500);
	Manager::standbyOn |= 1;
	wait_seconds(6);

	fireworksMode = button_wait;
}


#define PRE_FIRE_WAIL 4
#define POST_FIRE_WAIL 4
#define POST_WAIL_WAIT 10


void state_button_wait() {
	BTN_ON;
	if(BTN_STATE != 0) {
		BTN_OFF;

		Manager::standbyOn |= 1;
		Sound::start_wailing(PRE_FIRE_WAIL * 100);

		wait_seconds(PRE_FIRE_WAIL);

		fireworksMode = firing;
		return;
	}
}

uint8_t nextFired = 1;
void state_firing() {
	Manager::fire(nextFired++);

	wait_seconds(POST_FIRE_WAIL);

	Sound::sound_off();
	Manager::standbyOn &= ~(1);

	wait_seconds(POST_WAIL_WAIT);

	fireworksMode = button_wait;
}

int main() {
	init();

	while(true) {
		if(POWER_STATE != 0)
			fireworksMode = arming;

		switch(fireworksMode) {
		case arming:
			state_arming();
		break;

		case button_wait:
			state_button_wait();
		break;

		case firing:
			state_firing();
		break;
		}
	}
}
