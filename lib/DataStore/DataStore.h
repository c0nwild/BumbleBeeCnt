/*
 * DataStore.h
 *
 *  Created on: 05.08.2018
 *      Author: conradwild
 */

#ifndef LIB_DATASTORE_H_
#define LIB_DATASTORE_H_

#include <Arduino.h>

struct DataStore {
public:
	DataStore(){};
	virtual ~DataStore(){};

	uint8_t getMcpGpioa() const {
		return mcp_gpioa;
	}

	void setMcpGpioa(uint8_t mcpGpioa = 0) {
		mcp_gpioa = mcpGpioa;
	}

	uint8_t getMcpGpiob() const {
		return mcp_gpiob;
	}

	void setMcpGpiob(uint8_t mcpGpiob = 0) {
		mcp_gpiob = mcpGpiob;
	}

private:
	uint8_t mcp_gpioa = 0;
	uint8_t mcp_gpiob = 0;


};

#endif /* LIB_DATASTORE_H_ */
