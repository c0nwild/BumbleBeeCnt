/*
 * CRC8.h MAXIM
 *
 *  Created on: 15.11.2019
 *      Author: cwild
 */

#ifndef LIB_CRC8_SRC_CRC8_H_
#define LIB_CRC8_SRC_CRC8_H_

#include <Arduino.h>

namespace protocol {

class CRC8 {
private:
	uint8_t _crc = 0;
	uint8_t crc8(const uint8_t *data,size_t length);
public:
	CRC8(const uint8_t *data,size_t length);
	virtual ~CRC8();
	uint8_t get_crc();
};

} /* namespace protocol */

#endif /* LIB_CRC8_SRC_CRC8_H_ */
