
#include "FManager.h"
#include "../Selector/Selector.h"
#include "SBuffer.h"

namespace Fireworks {
namespace Manager {

SBuffer toIgnite = SBuffer();

// Current status of the firing mechanism
SMMode mode = firing;

uint8_t standbyOn = 0;
uint8_t refrDelay = 0;

uint8_t firedSlots[8] = {0};

bool isAvailable(uint8_t n) {
	return (firedSlots[n/8] & (1<< n%8)) == 0;
}
void markFired(uint8_t n) {
	firedSlots[n/8] |= (1<< n%8);
}

uint8_t getNextReady() {
	for(uint8_t i=0; i<64; i++) {
		if(isAvailable(i))
			return i;
	}

	return 255;
}

void update() {
	switch(mode) {
	case standby:
		if(++refrDelay == 5) {
			refrDelay = 0;

			if(standbyOn != 0) {
				Selector::selectRow(0, standbyOn);
				FIRE_PIN_ON;
			}
			else {
				FIRE_PIN_OFF;
				Selector::select(255);
			}
		}
	break;

	case firing:
		FIRE_PIN_OFF;
		if(toIgnite.available()) {
			Selector::select(toIgnite.pop());
			FIRE_PIN_ON;
		}
		else {
			if(standbyOn) {
				Selector::selectRow(0, standbyOn);
				FIRE_PIN_ON;
			}
			else
				Selector::select(255);
			mode = standby;
		}
	break;
	}
}

void fire(uint8_t nr) {
	if(!isAvailable(nr))
		return;

	markFired(nr);
	while(toIgnite.available() == 7) {}
	toIgnite.enqueue(nr);

	mode = firing;
}

}
}
