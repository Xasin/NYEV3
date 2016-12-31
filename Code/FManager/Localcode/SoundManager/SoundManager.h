/*
 * SoundManager.h
 *
 *  Created on: 31.12.2016
 *      Author: xasin
 */

#ifndef LOCALCODE_SOUNDMANAGER_SOUNDMANAGER_H_
#define LOCALCODE_SOUNDMANAGER_SOUNDMANAGER_H_

#include <avr/io.h>
#include "../../AVR/TIMER/Timer2.h"

namespace Sound {

	enum soundType : uint8_t {off, wail, custom};

	void init();

	void set_frequency(uint16_t frequency, uint16_t duration);
	void set_frequency(uint16_t frequency);

	void start_wailing(uint16_t duration);
	void start_wailing();

	void sound_off();

	bool is_playing();

	void update();
}

#endif /* LOCALCODE_SOUNDMANAGER_SOUNDMANAGER_H_ */
