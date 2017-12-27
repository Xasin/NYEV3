/*
 * main.cpp
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#include <avr/io.h>
#include <util/delay.h>

#include "AVR/Interfacing/Shift/ShiftReg.h"
#include "AVR/Communication/NEW_TWI/TWI.h"

#include "Localcode/Selector/Selector.h"
#include "Localcode/FManager/FManager.h"
#include "Localcode/SoundManager/SoundManager.h"

#include "AVR/TIMER/Timer1.h"

using namespace Interfacing;
using namespace Fireworks;

#define PRE_FIRE_WAIL 4
#define POST_FIRE_WAIL 4
#define POST_WAIL_WAIT 10

#define REQUIRED_BUTTON_HOLD 10
#define INVALID_BUTTON_HOLD 30


#define BTN_ON 		PORTB |= (1<< PB1);
#define BTN_OFF		PORTB &= ~(1<<PB1);
#define BTN_STATE 	(PINB & (1<< PB0))

#define POWER_STATE	(PIND & (1<< PD4))

uint8_t buttonPressDuration = 0;

uint8_t nextToFire = 255;

ISR(TIMER1_COMPA_vect) {
	Manager::update();
	Sound::update();
}

ISR(TWI_vect) {
	TWI::updateTWI();
}

class FireworksTWI: public TWI::Job {
public:
	FireworksTWI() {}

	bool slavePrepare() {
		switch(TWI::targetReg) {
		case 0x66:
			TWI::dataPacket = (uint8_t *)&nextToFire;
			TWI::dataLength = 1;
		return true;

		case 0x10:
			Manager::standbyOn &= ~1;
		return true;

		case 0x11:
			Manager::standbyOn |= 1;
		return true;

		default:
			return false;
		}
	}
};
FireworksTWI fireworksTwi = FireworksTWI();

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

	TWI::init();
	TWI::setAddr(0x66);
}

enum FWMode : uint8_t {arming, button_wait, pre_fire, firing};
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

	for(uint8_t i=6; i > 0; i--) {
		if(POWER_STATE != 0)
			return;

		_delay_ms(1000);
	}

	fireworksMode = button_wait;
	buttonPressDuration = 0;
}

void state_button_wait() {
	if(BTN_STATE != 0) {
		if(buttonPressDuration <= INVALID_BUTTON_HOLD)
			buttonPressDuration++;

		if(buttonPressDuration < INVALID_BUTTON_HOLD) {
			Manager::standbyOn |= 1;

			if((buttonPressDuration) & 1) {
				BTN_ON;
				Sound::set_frequency(2000, 20);
			}
			else if(buttonPressDuration >= REQUIRED_BUTTON_HOLD) {
				BTN_ON;
				Sound::set_frequency(2400, 20);
			}
			else
				BTN_OFF;
		}
		else if(buttonPressDuration == INVALID_BUTTON_HOLD) {
			Manager::standbyOn &= ~1;
			Sound::set_frequency(400, 100);
			BTN_OFF;
		}

		_delay_ms(200);
	}
	else {
		bool completed = buttonPressDuration >= REQUIRED_BUTTON_HOLD;
		if(buttonPressDuration >= INVALID_BUTTON_HOLD)
			completed = false;

		Manager::standbyOn &= ~1;
		buttonPressDuration = 0;

		if(completed) {
			nextToFire = Manager::getNextReady();
		}
		else {
			BTN_OFF;
			Sound::set_frequency(2200, 10);
			_delay_ms(100);
			BTN_ON;
			_delay_ms(900);
		}
	}
}

int main() {
	init();

	Manager::markFired(0);

	while(true) {
		if(POWER_STATE != 0)
			fireworksMode = arming;

		switch(fireworksMode) {
		case arming:
			state_arming();
		break;

		case button_wait:
			if(nextToFire != 255)
				fireworksMode = pre_fire;
			else
				state_button_wait();
		break;

		case pre_fire:
			Manager::standbyOn |= 1;
			Sound::start_wailing(PRE_FIRE_WAIL * 100);
			wait_seconds(PRE_FIRE_WAIL);

			fireworksMode = firing;
		break;

		case firing:
			Manager::fire(nextToFire);
			nextToFire = 255;

			Sound::start_wailing(POST_FIRE_WAIL * 90);
			wait_seconds(POST_FIRE_WAIL);

			Sound::sound_off();
			Manager::standbyOn &= ~(1);

			for(uint8_t i=POST_WAIL_WAIT; i!=0; i--) {
				_delay_ms(900);
				BTN_ON;
				_delay_ms(100);
				BTN_OFF;
			}

			fireworksMode = button_wait;
		break;
		}
	}
}
