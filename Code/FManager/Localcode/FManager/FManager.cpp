
#include "FManager.h"
#include "../Selector/Selector.h"
#include "SBuffer.h"

namespace Fireworks {
namespace Manager {

SBuffer toIgnite = SBuffer();

// Current status of the firing mechanism
SMMode mode = standby;

uint8_t standbyOn = 0;
uint8_t oldStandbyOn = 0;

void update() {
	if(standbyOn != oldStandbyOn) {
		Selector::selectRow(0, standbyOn);
		oldStandbyOn = standbyOn;
		FIRE_PIN_ON;
	}

	switch(mode) {
	case standby:
	break;

	case firing:
		FIRE_PIN_OFF;
		if(toIgnite.available()) {
			Selector::select(toIgnite.pop());
		}
		else {
			Selector::selectRow(0, standbyOn);
			mode = standby;
		}
		FIRE_PIN_ON;
	break;
	}
}

void fire(uint8_t nr) {
	while(toIgnite.available() == 8) {}
	toIgnite.enqueue(nr);

	mode = firing;
}

}
}
