/*
 * SegMan.cpp
 *
 *  Created on: 19 Dec 2018
 *      Author: xasin
 */

#include "SegMan.h"

const uint8_t sSegCodes[] = {
		0b0111111,
		0b0001100,
		0b1110110,
		0b1011110,
		0b1001101,
		0b1011011,
		0b1111011,
		0b0001111,
		0b1111111,
		0b1011111
};

struct LEDPos {
	int x;
	int y;
};

const LEDPos digitLEDPos[] {
		{x:0, y:2}, {x:0, y:1},
		{x:1, y:0}, {x:2, y:0},
		{x:3, y:1}, {x:3, y:2},
		{x:3, y:4}, {x:3, y:5},
		{x:2, y:6}, {x:1, y:6},
		{x:0, y:5}, {x:0, y:4},
		{x:1, y:3}, {x:2, y:3},
};

SegMan::SegMan(int length, NeoController &controller) :
	currentSegments(length), targetSegments(length),
	transitPosition(-1), formerTransitPosition(0),
	rgbController(controller),
	digits_is(length*14), digits_tgt(length*14),
	brightness_is(length*14), brightness_tgt(length*14),
	transitMode(SEGMENTS_DELAYED_PARALLEL),
	length(length),
	onColors(Color::HSV(0)), offColors(Color::HSV(180)),
	transitMarker(0xBBBBBB), transitSpeed(100)
	{

	digits_tgt.alpha = 60;
	brightness_tgt.alpha = 5;
	brightness_tgt.fill(0x999999);
	brightness_is.fill(0);
}

void SegMan::update_target_layers() {
	for(int n=0; n<length; n++)
		for(int i=0; i<14; i+=2) {
			digits_tgt[n*14 + i] = ((currentSegments[n] >> i/2) & 1) ? onColors[n*14 + i] : offColors[n*14 + i];
			digits_tgt[n*14 + i +1] = ((currentSegments[n] >> i/2) & 1) ? onColors[n*14 + i] : offColors[n*14 + i];
		}
}

void SegMan::update_tick() {
	if(transitPosition != -1) {
		int scaledTransitPosition = (++transitPosition * transitSpeed)/100;

		if(formerTransitPosition != scaledTransitPosition) {
			switch(transitMode) {
			case NUMBERS_DELAYED:
				for(int i=(formerTransitPosition+1); i<=scaledTransitPosition; i++) {
					if(i >= length) {
						transitPosition = -1;
						break;
					}
					currentSegments[i] = targetSegments[i];
				}
			break;

			case SEGMENTS_DELAYED:
				for(int i=(formerTransitPosition+1); i<=scaledTransitPosition; i++) {
					if(i >= (length*14)) {
						transitPosition = -1;
						break;
					}
					currentSegments[i/14] &= ~(1<<((i/2)%7));
					currentSegments[i/14] |= (1<<((i/2)%7)) & targetSegments[i/14];

					brightness_is[i].merge_overlay(transitMarker);
				}
			break;

			case SEGMENTS_DELAYED_PARALLEL:
				for(int i=(formerTransitPosition+1); i<=scaledTransitPosition; i++) {
					if(i >= 14) {
						transitPosition = -1;
						break;
					}
					for(int n=0; n<length; n++) {
						currentSegments[n] &= ~(1<<(i/2));
						currentSegments[n] |= (1<<(i/2)) & targetSegments[n];

						brightness_is[14*n + i].merge_overlay(transitMarker);
					}
				}
			break;

			case SWIPE:
				for(int i=0; i<(length*14); i++) {
					int x = digitLEDPos[i%14].x + 7*(i/14);

					if(x <= scaledTransitPosition && x > formerTransitPosition) {
						currentSegments[i/14] &= ~(1<<((i/2)%7));
						currentSegments[i/14] |= (1<<((i/2)%7)) & targetSegments[i/14];

						brightness_is[i].merge_overlay(transitMarker);
					}

					if(scaledTransitPosition >= (length*7)) {
						transitPosition = -1;
						break;
					}
				}
			break;
			default:
				for(int i=0; i<length; i++)
					currentSegments[i] = targetSegments[i];
				transitPosition = -1;
			break;
			}

			update_target_layers();
		}

		formerTransitPosition = scaledTransitPosition;
	}

	digits_is.merge_overlay(digits_tgt);
	brightness_is.merge_overlay(brightness_tgt);

	rgbController.colors.merge_overlay(digits_is);
	rgbController.colors.merge_multiply(brightness_is);
}

void SegMan::beat() {
	brightness_is.fill(0xBBBBBB);
}

void SegMan::write_number(long int num) {
	for(int i=0; i<length; i++) {
		targetSegments[length-1 - i] = sSegCodes[num % 10];
		num /= 10;
	}

	transitPosition = 0;
}
