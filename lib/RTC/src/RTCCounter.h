/*
 * EventCounter.h
 *
 *  Created on: 02.01.2019
 *      Author: cwild
 *
 *      This is the baseclass for a counter that persists in RTC ram. Needs 8 Byte per counter.
 */

#ifndef LIB_RTC_SRC_RTCCOUNTER_H_
#define LIB_RTC_SRC_RTCCOUNTER_H_

#include <system_definitions.h>

namespace rtc {

typedef struct {
	int cnt;
	int cnt_mirror;
} rtc_cnt_t;

class RTCCounter {
public:
	RTCCounter(uint8_t mem_loc);
	virtual ~RTCCounter();

	void init();
	virtual int get_cnt();
	virtual void set_cnt(int val);
	virtual void inc();
	virtual void dec();

private:
	uint8_t mem_location;
};

} /* namespace rtc */

#endif /* LIB_RTC_SRC_RTCCOUNTER_H_ */
