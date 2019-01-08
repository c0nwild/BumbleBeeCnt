/*
 * RTCCounter.cpp
 *
 *  Created on: 02.01.2019
 *      Author: cwild
 */

#include <RTCCounter.h>

#include "Arduino.h"
extern "C" {
#include <user_interface.h>
}

namespace rtc {

RTCCounter::RTCCounter(uint8_t mem_loc) : mem_location(mem_loc) {}

RTCCounter::~RTCCounter() {}

void RTCCounter::init() {
	rtc_cnt_t e;
	e.cnt = 0;
	e.cnt_mirror = 0xffffffff;
	system_rtc_mem_write(mem_location, &e, sizeof(e));
}

int RTCCounter::get_cnt() {
	rtc_cnt_t e;
	int rv;
	system_rtc_mem_read(mem_location, &e, sizeof(e));
	if ((e.cnt ^ e.cnt_mirror) == -1)
		rv = e.cnt;
	else
		rv = -1;
	return rv;
}

void RTCCounter::set_cnt(int val) {
	rtc_cnt_t e;
	e.cnt = val;
	e.cnt_mirror = ~val;
	system_rtc_mem_write(mem_location, &e, sizeof(e));
}

void RTCCounter::inc() {
	int c_val;
	c_val = this->get_cnt();
	++c_val;
	this->set_cnt(c_val);
}

void RTCCounter::dec() {
	int c_val;
	c_val = this->get_cnt();
	--c_val;
	this->set_cnt(c_val);
}

} /* namespace rtc */
