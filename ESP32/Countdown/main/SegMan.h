/*
 * SegMan.h
 *
 *  Created on: 19 Dec 2018
 *      Author: xasin
 */

#ifndef MAIN_SEGMAN_H_
#define MAIN_SEGMAN_H_

#include <vector>

#include <xnm/neocontroller.h>

using namespace XNM::Neo;

class SegMan {
public:
	std::vector<uint16_t> currentSegments;
	std::vector<uint16_t> targetSegments;

	int transitPosition;
	int formerTransitPosition;

	NeoController &rgbController;

	Layer digits_is;
	Layer digits_tgt;

	Layer brightness_is;
	Layer brightness_tgt;

	void update_target_layers();

public:
	struct LEDPos {
		int x;
		int y;
	};

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

	static LEDPos get_led_pos(int i);

	SegMan(int length, NeoController &controller);

	void update_tick();

	void beat();

	void write_number(long num);
	void write_countdown_ms(long num);
};

#endif /* MAIN_SEGMAN_H_ */
