/*
 * EventCounter.h
 *
 *  Created on: 08.01.2019
 *      Author: cwild
 */

#ifndef LIB_RTC_SRC_EVENTCOUNTER_H_
#define LIB_RTC_SRC_EVENTCOUNTER_H_

#include <RTCCounter.h>

namespace rtc {

class EventCounter: public RTCCounter {
public:
	EventCounter(uint8_t mem_location);
	virtual ~EventCounter();
};

} /* namespace rtc */

#endif /* LIB_RTC_SRC_EVENTCOUNTER_H_ */
