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
	int8_t dir = 0;
	uint8_t wlan_en = 0;
	uint8_t tare = 0;
	uint16_t mcp_gpioab = 0;
	int v_batt = 0;
	bool new_data = false;
	bool do_log_entry = false;
	int ev_cnt_in;
	int ev_cnt_out;
};

struct BumbleBeeRamData {
	float humidity = 0;
	float temperature = 0;
	float pressure = 0;
	float weight = 0;
	uint8_t lb0 = 0;
	uint8_t lb1 = 0;
	uint8_t edge_lb0 = 0;
	uint8_t edge_lb1 = 0;
	int8_t dir = 0;
	int v_batt = 0;
	long ts = 0;
	uint16_t ts_diff = 0;
};//size 31 (mit lb1 und dir sense)

enum event_eval {
	new_edge_lb0 = 0,
	new_edge_lb1,
	count_event_in,
	count_event_out,
	cleanup,
	idle
};

struct WiFiConfig {
  char ssid[64] = {0};
  char passwd[128] = {0};
};

#endif /* LIB_SYSTEM_TYPES_H_ */
