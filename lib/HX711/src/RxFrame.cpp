/*
 * RxFrame.cpp
 *
 *  Created on: 27.11.2019
 *      Author: cwild
 */

#include <RxFrame.h>

namespace hx711 {

RxFrame::RxFrame(std::vector<uint8_t> data) {
	for (uint8_t idx = 0; idx < sizeof(weight.f); idx++) {
		weight.b[idx] = data.at(idx + sizeof(header));
	}
}

} /* namespace hx711 */
