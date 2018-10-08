/*
 * I2CCom.cpp
 *
 *  Created on: 08.10.2018
 *      Author: cwild
 */

#include "../I2C/I2CCom.h"

namespace i2c {

I2CCom::I2CCom() {
	// TODO Auto-generated constructor stub

}

I2CCom::~I2CCom() {
	// TODO Auto-generated destructor stub
}

bool I2CCom::sendData(uint8_t data) {
	uint8_t rv;
	uint8_t to_send[2];

	//Check if addr is set
	if (!slave_addr)
		return false;

	to_send[0] = data;
	to_send[1] = (uint8_t)~data;

	Wire.begin();
	Wire.beginTransmission(slave_addr);
	Wire.write(to_send, 2);
	rv = Wire.endTransmission();
	return (rv == 0);
}

uint8_t I2CCom::getData() {
	size_t cnt = 0;
	uint8_t rv = 0xff;
	bool verified = false;

	//Check if addr is set
	if (!slave_addr)
		return 0;

	Wire.begin();
	Wire.requestFrom(slave_addr, 2u);
	while (Wire.available()) {
		if (cnt > 2)
			break;
		buf[cnt] = Wire.read();
		++cnt;
	}
	Wire.endTransmission();

	verified = verifyData(buf);

	if (verified)
		rv = buf[0];

	return rv;
}

void i2c::I2CCom::setSlaveAddr(uint8_t addr) {
	slave_addr = addr;
}

uint16_t i2c::I2CCom::dumpBuffer() {
	uint16_t rv;
	rv = (buf[1] << 8)| buf[0];
	return rv;
}

bool I2CCom::verifyData(uint8_t *data) {
	return ((data[0] ^ data[1]) == 0xff);
}

} /* namespace i2c */
