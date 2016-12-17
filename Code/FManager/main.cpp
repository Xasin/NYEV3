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


#include "AVR/TIMER/Timer2.h"
#include "AVR/TIMER/Timer1.h"

using namespace Interfacing;
using namespace Fireworks;

ShiftReg testreg = ShiftReg(&PORTD, PD5);

ISR(TIMER1_COMPA_vect) {
	Manager::update();
}

int main() {

	DDRB |= (1 << PB5 | 1 << PB3);
	DDRD |= (1 << PD2);

	Timer2::set_mode(TIMER2_MODE_TOP_OCR2A);
	Timer2::set_OC2A_mode(TIMER2_OC2A_TOGGLE);

	Timer2::set_OC2A_frequency(0);

	Timer1::enable_CTC(100);

	Selector::select(255);

	Manager::standbyOn = 0b1;

	while(true) {
		_delay_ms(2000);
		Manager::fire(3);
		Manager::fire(1);
		Manager::fire(5);
	}
}
