/*
 * SReg.cpp
 *
 *  Created on: 13.04.2018
 *      Author: conradwild
 */

#include "SReg.h"
#ifdef UNIT_TEST
#include "SReg_test.h"
using namespace rtc_test;
#else
#include "Arduino.h"
extern "C" {
#include <user_interface.h>
}
#endif

namespace rtc {

SReg::SReg() {
	// TODO Auto-generated constructor stub

}

SReg::~SReg() {
	// TODO Auto-generated destructor stub
}

void SReg::init() {
	rtc_sreg_t s = { .sreg = 0, .sreg_mirror = 0xffffffff };
	system_rtc_mem_write(RTC_SREG, &s, 8);
}

void SReg::set_bit(uint32_t mask) {
	rtc_sreg_t s;
	system_rtc_mem_read(RTC_SREG, &s, 8);
	s.sreg |= mask;
	s.sreg_mirror &= ~mask;
	system_rtc_mem_write(RTC_SREG, &s, 8);
}

void SReg::unset_bit(uint32_t mask) {
	rtc_sreg_t s;
	system_rtc_mem_read(RTC_SREG, &s, 8);
	s.sreg &= ~mask;
	s.sreg_mirror |= mask;
	system_rtc_mem_write(RTC_SREG, &s, 8);
}

int SReg::query_bit(uint32_t mask) {
	rtc_sreg_t s;
	int rv;
	system_rtc_mem_read(RTC_SREG, &s, 8);
	if ((s.sreg ^ s.sreg_mirror) == 0xffffffff)
		rv = (s.sreg & mask) ? 1 : 0;
	else
		rv = -1;
	return rv;
}

rtc_sreg_t SReg::query_sreg() {
	rtc_sreg_t s;
	system_rtc_mem_read(RTC_SREG, &s, 8);
	return s;
}

} /* namespace rtc */

