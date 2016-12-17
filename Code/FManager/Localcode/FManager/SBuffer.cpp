/*
 * SBuffer.cpp
 *
 *  Created on: 17.12.2016
 *      Author: xasin
 */

#include "SBuffer.h"

namespace Fireworks {
namespace Manager {

SBuffer::SBuffer() : start(0), end(0) {
}

uint8_t SBuffer::available() {
	if(start <= end)
		return end - start;
	else
		return 8 - start + end;
}

uint8_t SBuffer::pop() {
	if(this->available()) {
		uint8_t data = content[start];
		if(++start == 8)
			start = 0;

		return data;
	}
	return 0;
}

bool SBuffer::enqueue(uint8_t data) {
	if(this->available() == 7)
		return false;

	content[end] = data;
	if(++end == 8)
		end = 0;

	return true;
}

} /* namespace Manager */
} /* namespace Fireworks */
