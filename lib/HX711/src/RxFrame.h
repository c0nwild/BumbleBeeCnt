/*
 * RxFrame.h
 *
 *  Created on: 27.11.2019
 *      Author: cwild
 */

#ifndef LIB_HX711_SRC_RXFRAME_H_
#define LIB_HX711_SRC_RXFRAME_H_

#include "Arduino.h"
#include <serial_debug.h>

namespace hx711 {

class RxFrame {
	union {
		float f = 0.0;
		uint8_t b[sizeof(f)];
	} float_value;
	uint8_t header = 0;
public:
	/*
	 * Constructor converts Databytes from input vector to unsigned
	 * integer using a union type.
	 */
	RxFrame(std::vector<uint8_t> data) {
		DEBUG_MSG("RxFrame size: " + String(data.size()))
		for (uint8_t idx = 0; idx < sizeof(float_value.f); idx++) {
			float_value.b[idx] = data.at(idx + sizeof(header));
		}
	}

	float get_value() { return float_value.f; }
};

} /* namespace hx711 */

#endif /* LIB_HX711_SRC_RXFRAME_H_ */
