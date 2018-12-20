/*
 * SegMan.h
 *
 *  Created on: 19 Dec 2018
 *      Author: xasin
 */

#ifndef MAIN_SEGMAN_H_
#define MAIN_SEGMAN_H_

#include <vector>

#include "NeoController.h"

using namespace Peripheral;

class SegMan {
private:
	std::vector<uint8_t> currentSegments;
	std::vector<uint8_t> targetSegments;

	int transitPosition;
	int formerTransitPosition;

	NeoController &rgbController;

	Layer digits_is;
	Layer digits_tgt;

	Layer brightness_is;
	Layer brightness_tgt;

	void update_target_layers();

public:
	enum TransitMode {
		DIRECT,
		NUMBERS_DELAYED,
		SEGMENTS_DELAYED,
		SEGMENTS_DELAYED_PARALLEL,
		SWIPE,
	} transitMode;

	const int length;

	Layer onColors;
	Layer offColors;

	Color transitMarker;
	int   transitSpeed;

	SegMan(int length, NeoController &controller);

	void update_tick();

	void beat();

	void write_number(long num);
};

#endif /* MAIN_SEGMAN_H_ */
