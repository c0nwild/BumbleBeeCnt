/*
 * RTCDataBuffer.cpp
 *
 *  Created on: 27.01.2019
 *      Author: conradwild
 */

#include <RTCDataBuffer.h>
extern "C" {
#include <user_interface.h>
}

namespace rtc {

RTCDataBuffer::RTCDataBuffer(uint8_t mem_loc) :
		mem_location(mem_loc) {
}

RTCDataBuffer::~RTCDataBuffer() {
}

void RTCDataBuffer::setBuffer(BumbleBeeRamData* data) {
	if (sizeof(*data) > sysdefs::rtc::rtc_memsize) {
		return;
	}
	system_rtc_mem_write(mem_location, data, sizeof(*data));
}

void RTCDataBuffer::init() {
	BumbleBeeRamData data;
	system_rtc_mem_write(mem_location, &data, sizeof(data));
}

BumbleBeeRamData RTCDataBuffer::getBuffer() {
	BumbleBeeRamData rv;
	if (sizeof(rv) > sysdefs::rtc::rtc_memsize) {
		return rv;
	}
	system_rtc_mem_read(mem_location, &rv, sizeof(rv));
	return rv;
}

} /* namespace rtc */
