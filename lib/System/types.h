/*
 * types.h
 *
 *  Created on: 11.12.2018
 *      Author: conradwild
 */

#ifndef LIB_SYSTEM_TYPES_H_
#define LIB_SYSTEM_TYPES_H_

#include <EventData.h>
#include <Arduino.h>

struct BumbleBeeCntData: public EventData {
	String info;
	float humidity = 0;
	float temperature = 0;
	float pressure = 0;
	float weight = 0;
	uint8_t lb0 = 0;
	uint8_t lb1 = 0;
	uint8_t wlan_en = 0;
	uint8_t tare = 0;
	uint16_t mcp_gpioab = 0;
	int v_batt = 0;
	bool new_data = false;
};

struct BumbleBeeRamData {
	float humidity = 0;
	float temperature = 0;
	float pressure = 0;
	float weight = 0;
	uint8_t lb0 = 0;
	uint8_t lb1 = 0;
	uint8_t wlan_en = 0;
	uint8_t tare = 0;
	uint16_t mcp_gpioab = 0;
	int v_batt = 0;
};

#endif /* LIB_SYSTEM_TYPES_H_ */
