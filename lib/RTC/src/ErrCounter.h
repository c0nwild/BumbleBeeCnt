/*
 * ErrCounter.h
 *
 *  Created on: 08.01.2019
 *      Author: cwild
 */

#ifndef LIB_RTC_SRC_ERRCOUNTER_H_
#define LIB_RTC_SRC_ERRCOUNTER_H_

#include <RTCCounter.h>

namespace rtc {

class ErrCounter: public RTCCounter {
public:
	ErrCounter(uint8_t mem_location);
	virtual ~ErrCounter();
};

} /* namespace rtc */

#endif /* LIB_RTC_SRC_ERRCOUNTER_H_ */
