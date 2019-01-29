/*
 * RTCDataBuffer.h
 *
 *  Created on: 27.01.2019
 *      Author: conradwild
 */

#ifndef LIB_RTC_SRC_RTCDATABUFFER_H_
#define LIB_RTC_SRC_RTCDATABUFFER_H_

#include <system_definitions.h>
#include <types.h>

namespace rtc {

class RTCDataBuffer {
public:
	RTCDataBuffer(uint8_t mem_loc);
	virtual ~RTCDataBuffer();

	void init();
	void setBuffer (BumbleBeeRamData *data);
	BumbleBeeRamData getBuffer();
private:
	uint8_t mem_location;
};

} /* namespace rtc */

#endif /* LIB_RTC_SRC_RTCDATABUFFER_H_ */
