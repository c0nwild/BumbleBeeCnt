/*
 * I2CCom.h
 *
 *  Created on: 08.10.2018
 *      Author: cwild
 */

#ifndef LIB_I2C_I2CCOM_H_
#define LIB_I2C_I2CCOM_H_

#include <Wire.h>
#include <Arduino.h>

namespace i2c {

class I2CCom {
public:
	I2CCom();
	virtual ~I2CCom();
	bool sendData(uint8_t data);
	uint8_t getData();
	void setSlaveAddr(uint8_t addr);
	uint16_t dumpBuffer();

private:
	uint8_t slave_addr = 0;

	uint8_t buf[2];

	bool verifyData(uint8_t *data);

};

} /* namespace i2c */

#endif /* LIB_I2C_I2CCOM_H_ */
