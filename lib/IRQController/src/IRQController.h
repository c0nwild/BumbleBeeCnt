/*
 * IRQController.h
 *
 *  Created on: 08.10.2018
 *      Author: cwild
 */

#ifndef LIB_IRQCONTROLLER_IRQCONTROLLER_H_
#define LIB_IRQCONTROLLER_IRQCONTROLLER_H_

#include <Wire.h>
#include <Arduino.h>

class IRQController {
public:
	IRQController();
	virtual ~IRQController();
	bool sendData(uint8_t data);
	uint8_t getData();
	void setSlaveAddr(uint8_t addr);
	uint16_t dumpBuffer();

private:
	uint8_t slave_addr = 0;

	uint8_t buf[2];

	bool verifyData(uint8_t *data);

};

#endif /* LIB_IRQCONTROLLER_IRQCONTROLLER_H_ */
