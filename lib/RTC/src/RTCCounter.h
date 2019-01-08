/*
 * EventCounter.h
 *
 *  Created on: 02.01.2019
 *      Author: cwild
 */

#ifndef LIB_RTC_SRC_EVENTCOUNTER_H_
#define LIB_RTC_SRC_EVENTCOUNTER_H_

#include <system_definitions.h>

namespace rtc {

typedef struct {
	int evcnt;
	int evcnt_mirror;
} rtc_evcnt_t;

class EventCounter {
public:
	EventCounter();
	virtual ~EventCounter();

	void init();
	int get_evcnt();
	void set_evcnt(int val);
};

} /* namespace rtc */

#endif /* LIB_RTC_SRC_EVENTCOUNTER_H_ */
