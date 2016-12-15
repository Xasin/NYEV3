/*
 * main.cpp
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#include <avr/io.h>
#include <util/delay.h>

#include "AVR/Interfacing/Shift/ShiftReg.h"
#include "Localcode/Selector.h"

#include "AVR/TIMER/Timer2.h"

using namespace Interfacing;
using namespace Fireworks;

ShiftReg testreg = ShiftReg(&PORTD, PD5);

int main() {
	uint16_t i=0;

	DDRB |= (1 << PB5 | 1 << PB3);


	Timer2::set_prescaler(TIMER2_PRESC_64);
	Timer2::set_mode(TIMER2_MODE_TOP_OCR2A);
	Timer2::set_OC2A_mode(TIMER2_OC2A_TOGGLE);
	Timer2::set_OCR2A(20);

	while(true) {
		_delay_ms(100);
		Fireworks::select(i++);
		if(i > 63) i = 0;
	}
}
