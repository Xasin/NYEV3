/*
 * Selector.h
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#ifndef LOCALCODE_SELECTOR_H_
#define LOCALCODE_SELECTOR_H_

#include "../../AVR/Interfacing/Shift/ShiftReg.h"
#include <avr/io.h>

namespace Fireworks {
namespace Selector {

void select(uint8_t nr);
void selectRow(uint8_t r, uint8_t data);

}
}



#endif /* LOCALCODE_SELECTOR_H_ */
