/*
 * SegMan.cpp
 *
 *  Created on: 19 Dec 2018
 *      Author: xasin
 */

#include "SegMan.h"

SegMan::SegMan(int length, NeoController &controller) :
	currentSegments(length, 0), targetSegments(length, 0),
	transitPosition(-1), rgbController(controller),
	digits_is(length), digits_tgt(length),
	brightness_is(length), brightness_tgt(length),
	length(length),
	onColors(0xFFFFFF), offColors(0)
	{

}

