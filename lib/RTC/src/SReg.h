/*
 * SReg.h
 *
 *  Created on: 13.04.2018
 *      Author: conradwild
 */

#ifndef SRC_RTC_H_
#define SRC_RTC_H_

#include <stdint.h>

namespace rtc {

// Defines für Zugriff auf RTC-RAM
#define RTCMEMSTART 65
#define RTCMEMSIZE 127
#define RTC_SREG (RTCMEMSTART + 0)
#define RTC_INIT_TS (RTCMEMSTART + 4)

#define SET 1
#define UNSET 0

#define RTC_SREG_BIT_POWERON 0x1
#define RTC_SREG_BIT_SYSTEM_INIT 0x2
#define RTC_SREG_BIT_MCP_GPIOA_EVENT 0x4
#define RTC_SREG_BIT_MCP_GPIOB_EVENT 0x8

typedef struct {
	uint32_t sreg;
	uint32_t sreg_mirror;
} rtc_sreg_t;

class SReg {

public:
	SReg();
	virtual ~SReg();
	void init();
	void set_bit(uint32_t mask);
	void unset_bit(uint32_t mask);
	int query_bit(uint32_t mask);
	rtc_sreg_t query_sreg();
	void set_init_timestamp(uint32_t ts);
	uint32_t get_init_timestamp();
};

} /* namespace rtc */

#endif /* SRC_RTC_H_ */

