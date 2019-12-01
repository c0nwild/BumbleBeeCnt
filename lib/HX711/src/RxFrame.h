/*
 * RxFrame.h
 *
 *  Created on: 27.11.2019
 *      Author: cwild
 */

#ifndef LIB_HX711_SRC_RXFRAME_H_
#define LIB_HX711_SRC_RXFRAME_H_

#include "Arduino.h"

namespace hx711 {

class RxFrame {
	union {
		float f = 0;
		uint8_t b[sizeof(f)];
	} weight;
	uint8_t header = 0;
public:
	/* Der Konstruktor konvertiert die Datenbytes aus dem Input-Vektor über
	 * eine union in ein unsigned integer.
	 * Constructor converts Databytes from input vector to unsigned integer using
	 * a union type.
	 */
	RxFrame(std::vector<uint8_t> data);

	float get_weight() {
		return weight.f;
	}
};

} /* namespace hx711 */

#endif /* LIB_HX711_SRC_RXFRAME_H_ */
