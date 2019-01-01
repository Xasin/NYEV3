/*
 * SegMan.cpp
 *
 *  Created on: 19 Dec 2018
 *      Author: xasin
 */


#include <cstring>

#include "SegMan.h"

const uint16_t sSegCodes[] = {
		0b00111111111111,
		0b00000011110000,
		0b11111100111100,
		0b10001111111100,
		0b11000011110011,
		0b11001111001111,
		0b11111111001111,
		0b00000011111100,
		0b11111111111111,
		0b11000111111111
};

const SegMan::LEDPos digitLEDPos[] {
		{x:0, y:2}, {x:0, y:1},
		{x:1, y:0}, {x:2, y:0},
		{x:3, y:1}, {x:3, y:2},
		{x:3, y:4}, {x:3, y:5},
		{x:2, y:6}, {x:1, y:6},
		{x:0, y:5}, {x:0, y:4},
		{x:1, y:3}, {x:2, y:3},
};

SegMan::LEDPos SegMan::get_led_pos(int i) {
	int extraX = 7*(i/14);

	i %= 14;
	if(i < 0)
		i+= 14;

	LEDPos oPos = digitLEDPos[i];
	oPos.x += extraX;
	return oPos;
}

SegMan::SegMan(int length, NeoController &controller) :
	currentSegments(length), targetSegments(length),
	transitPosition(-1), formerTransitPosition(0),
	rgbController(controller),
	digits_is(length*14), digits_tgt(length*14),
	brightness_is(length*14), brightness_tgt(length*14),
	transitMode(SEGMENTS_DELAYED_PARALLEL),
	length(length),
	onColors(Color::HSV(180, 255, 150)), offColors(Color::HSV(0, 100, 20)),
	transitMarker(0x888888), transitSpeed(10)
	{

	digits_tgt.alpha = 25;
	brightness_tgt.alpha = 10;
	brightness_tgt.fill(0x555555);
	brightness_is.fill(0);
}

void SegMan::update_target_layers() {
	for(int n=0; n<length; n++)
		for(int i=0; i<14; i++) {
			digits_tgt[n*14 + i] = ((currentSegments[n] >> i) & 1) ? onColors[n*14 + i] : offColors[n*14 + i];
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
					currentSegments[length-1 -i] = targetSegments[length-1 - i];
				}
			break;

			case SEGMENTS_DELAYED:
				for(int i=(formerTransitPosition+1); i<=scaledTransitPosition; i++) {
					if(i >= (length*14)) {
						transitPosition = -1;
						break;
					}
					else if((i%14) == 0) {
						void *cD = currentSegments.data();
						void *tD = targetSegments.data();

						if(memcmp(cD, tD, sizeof(uint16_t)*(length-i/14)) == 0) {
							transitPosition = -1;
							break;
						}
					}

					int iT = length*14-1 -i;

					currentSegments[iT/14] &= ~(1<<(iT%14));
					currentSegments[iT/14] |= (1<<(iT%14)) & targetSegments[iT/14];

					if((targetSegments[iT/14] >> (iT%14)) & 1)
						digits_is[iT].merge_overlay(onColors[iT]);

					brightness_is[iT].merge_overlay(transitMarker);
				}
			break;

			case SEGMENTS_DELAYED_PARALLEL:
				for(int i=(formerTransitPosition+1); i<=scaledTransitPosition; i++) {
					if(i >= 14) {
						transitPosition = -1;
						break;
					}

					for(int n=0; n<length; n++) {
						if(currentSegments[n] == targetSegments[n])
							continue;

						currentSegments[n] &= ~(1<<i);
						currentSegments[n] |= (1<<i) & targetSegments[n];

						if((targetSegments[n] >> i) & 1)
							digits_is[i + 14*n].merge_overlay(onColors[i + 14*n]);

						brightness_is[14*n + i].merge_overlay(transitMarker);
					}
				}
			break;

			case SWIPE:
				for(int i=0; i<(length*14); i++) {
					int x = length*6 - digitLEDPos[i%14].x - 6*(i/14);

					if((x <= scaledTransitPosition) && (x >= formerTransitPosition)) {
						currentSegments[i/14] &= ~(1<<(i%14));
						currentSegments[i/14] |= (1<<(i%14)) & targetSegments[i/14];

						if((targetSegments[i/14] >> (i%14)) & 1)
							digits_is[i].merge_overlay(onColors[i]);

						brightness_is[i].merge_overlay(transitMarker);
					}
				}

				if((scaledTransitPosition%6) == 0) {
					void *cD = currentSegments.data();
					void *tD = targetSegments.data();

					if(memcmp(cD, tD, sizeof(uint16_t)*(length-scaledTransitPosition/7)) == 0)
						transitPosition = -1;
				}

				if(scaledTransitPosition >= (length*7))
					transitPosition = -1;
			break;
			default:
				for(int i=0; i<length; i++)
					currentSegments[i] = targetSegments[i];
				transitPosition = -1;
			break;
			}
			formerTransitPosition = (transitPosition == -1) ? -1 : scaledTransitPosition;
		}
	}
	else if(memcmp(targetSegments.data(), currentSegments.data(), length*sizeof(uint16_t)) != 0)
		transitPosition = 0;

	update_target_layers();

	digits_is.merge_overlay(digits_tgt);
	brightness_is.merge_overlay(brightness_tgt);

	rgbController.colors.merge_overlay(digits_is);
	rgbController.colors.merge_multiply(brightness_is);
}

void SegMan::beat() {
	brightness_is.fill(0x888888);
}

void SegMan::write_number(long int num) {
	for(int i=0; i<length; i++) {
		targetSegments[length-1 - i] = sSegCodes[num % 10];
		num /= 10;
	}
}

void SegMan::write_countdown_ms(long int num) {
	uint8_t segShift = 0;

	if(num < 0)
		num *= -1;

	// Check for s:ms (largest 9:99)
	if(num < 10000)
		segShift = 0;
	// Check for ss:ms (largest 59:9)
	else if(num < 60*1000)
		segShift = 1;
	// Check for m:ss (largest 9:59)
	else if(num < 10 * 60 * 1000)
		segShift = 2;
	// Check for mm:s (largest 59:5)
	else if(num < 60*60*1000)
		segShift = 3;
	// h:mm (largest 9:59)
	else if(num < 10*60*60*1000)
		segShift = 4;
	else
		segShift = 5;

	long int cN = 0;
	cN += (num/10)%100;
	cN += (num/1000)%60 * 100;
	cN += (num/(60*1000)%60) * 10000;
	cN += (num/(60*60*1000)) * 1000000;

	for(uint8_t i=segShift; i!=0; i--)
		cN /= 10;

	write_number(cN);

	const Color segColors[] = {
			Material::DEEP_ORANGE,
			Material::AMBER,
			Material::GREEN,
			Material::BLUE
	};

	for(uint8_t i=0; i<length; i++)
		for(uint8_t j=0; j<14; j++)
			onColors[j + 14*i] = segColors[(length-1-i+segShift)/2];
}
