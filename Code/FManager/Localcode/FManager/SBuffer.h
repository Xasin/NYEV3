/*
 * SBuffer.h
 *
 *  Created on: 17.12.2016
 *      Author: xasin
 */

#ifndef LOCALCODE_FMANAGER_SBUFFER_H_
#define LOCALCODE_FMANAGER_SBUFFER_H_

#include <avr/io.h>

namespace Fireworks {
namespace Manager {

class SBuffer {
private:
volatile uint8_t start;
uint8_t end;
uint8_t content[8];

public:
	SBuffer();

	// Return available bytes
	uint8_t available();
	// Pop next byte
	uint8_t pop();

	bool enqueue(uint8_t data);
};

} /* namespace Manager */
} /* namespace Fireworks */

#endif /* LOCALCODE_FMANAGER_SBUFFER_H_ */
