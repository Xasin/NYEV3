/*
 * FManager.h
 *
 *  Created on: 17.12.2016
 *      Author: xasin
 */

#ifndef LOCALCODE_FMANAGER_FMANAGER_H_
#define LOCALCODE_FMANAGER_FMANAGER_H_

#include <avr/io.h>

#define FIRE_PIN_ON	PORTD |= (1<< PD2);
#define FIRE_PIN_OFF PORTD &= ~(1<< PD2);

namespace Fireworks {
namespace Manager {

enum SMMode : uint8_t {standby, scanning, firing};

extern uint8_t standbyOn;

void update();
void fire(uint8_t nr);

}
}


#endif /* LOCALCODE_FMANAGER_FMANAGER_H_ */
