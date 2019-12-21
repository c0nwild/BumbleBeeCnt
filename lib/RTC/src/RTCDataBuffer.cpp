/*
 * RTCDataBuffer.cpp
 *
 *  Created on: 27.01.2019
 *      Author: conradwild
 */

#include <RTCDataBuffer.h>
#include <serial_debug.h>
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
		DEBUG_MSG("RAM Buf Ovf!")
		return;
	}
	if (!system_rtc_mem_write(mem_location, data, sizeof(*data))) {
		DEBUG_MSG("RAM write failed...");
	}
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
	if (!system_rtc_mem_read(mem_location, &rv, sizeof(rv))) {
		DEBUG_MSG("RAM read failed...");
	}
	return rv;
}

} /* namespace rtc */
