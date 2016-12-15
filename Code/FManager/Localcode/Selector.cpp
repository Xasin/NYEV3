/*
 * Selector.cpp
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#include "Selector.h"

namespace Fireworks {

Interfacing::ShiftReg muplex = Interfacing::ShiftReg(&PORTD, PD5);

void select(uint8_t nr) {
	if(nr > 63)
		muplex.write16(0);
	else
		muplex.write16((1 << nr/8) | (1 << (nr%8 + 8)));

}

void selectRow(uint8_t r, uint8_t data) {
	muplex.write16((1 << r) | (data << 8));
}

}
