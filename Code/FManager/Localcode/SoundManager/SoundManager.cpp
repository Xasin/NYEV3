/*
 * SoundManager.cpp
 *
 *  Created on: 31.12.2016
 *      Author: xasin
 */

#include "SoundManager.h"

namespace Sound {

	soundType currentMode = off;

	uint16_t duration = 0;
	uint16_t timing = 0;

#define LOW_WAIL_FREQUENCY 2400
#define HIGH_WAIL_FREQUENCY 3000

	void init() {
		Timer2::set_mode(TIMER2_MODE_TOP_OCR2A);
		Timer2::set_OC2A_mode(TIMER2_OC2A_TOGGLE);

		Timer2::set_OC2A_frequency(0);
	}


	void set_frequency(uint16_t frequency, uint16_t duration) {
		currentMode = custom;
		Timer2::set_OC2A_frequency(frequency);
		Sound::duration = duration;
	}
	void set_frequency(uint16_t frequency) {
		Sound::set_frequency(frequency, 0);
	}

	void start_wailing(uint16_t duration) {
		timing = 0;
		Sound::duration = duration;
		currentMode = wail;
	}
	void start_wailing() {
		Sound::start_wailing(0);
	}

	void sound_off() {
		Timer2::set_OC2A_frequency(0);
		currentMode = off;
	}

	bool is_playing() {
		return currentMode == off;
	}

	void update() {
		switch(currentMode) {
		case custom:
			if(duration != 0)
				if(--duration == 0)
					sound_off();
		break;

		case wail:
			timing++;
			if(timing == 50)
				Timer2::set_OC2A_frequency(LOW_WAIL_FREQUENCY);
			if(timing == 100) {
				Timer2::set_OC2A_frequency(HIGH_WAIL_FREQUENCY);
				timing = 0;
			}

			if(duration != 0)
				if(--duration == 0)
					sound_off();
		break;

		case off:
			break;
		}

	}
}
