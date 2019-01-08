/*
 * EventCounter.cpp
 *
 *  Created on: 02.01.2019
 *      Author: cwild
 */

#include <EventCounter.h>

#include "Arduino.h"
extern "C" {
#include <user_interface.h>
}

namespace rtc {

EventCounter::EventCounter() {}

EventCounter::~EventCounter() {}

void EventCounter::init() {
	rtc_evcnt_t e;
	e.evcnt = 0;
	e.evcnt_mirror = 0xffffffff;
	system_rtc_mem_write(sysdefs::rtc::rtc_eventcnt, &e, sizeof(e));
}

int EventCounter::get_evcnt() {
	rtc_evcnt_t e;
	int rv;
	system_rtc_mem_read(sysdefs::rtc::rtc_eventcnt, &e, sizeof(e));
	if ((e.evcnt ^ e.evcnt_mirror) == 0xffffffff)
		rv = e.evcnt;
	else
		rv = -1;
	return rv;
}

void EventCounter::set_evcnt(int val) {
	rtc_evcnt_t e;
	e.evcnt = val;
	e.evcnt_mirror = ~val;
	system_rtc_mem_write(sysdefs::rtc::rtc_eventcnt, &e, sizeof(e));
}

} /* namespace rtc */
