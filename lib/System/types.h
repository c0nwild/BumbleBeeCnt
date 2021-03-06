/*
 * types.h
 *
 *  Created on: 11.12.2018
 *      Author: conradwild
 */

#ifndef LIB_SYSTEM_TYPES_H_
#define LIB_SYSTEM_TYPES_H_

#include <sysconfig.h>
#include <EventData.h>
#include <Arduino.h>

struct BumbleBeeCntData: public EventData {
	String info;
	float humidity = 0;
	float temperature = 0;
	float pressure = 0;
	float weight = 0;
	uint8_t lb0 = 0;
#ifdef LB1
	uint8_t lb1 = 0;
#endif
#ifdef DIR_SENSE
	int8_t dir = 0;
	int ev_cnt_in;
	int ev_cnt_out;
#else
	int ev_cnt0 = 0;
#ifdef LB1
	int ev_cnt1 = 0;
#endif// LB1
#endif//DIR_SENSE
	uint8_t wlan_en = 0;
	uint8_t tare = 0;
	uint16_t mcp_gpioab = 0;
	int v_batt = 0; //in digits (10 bit)
	bool new_data = false;
	bool do_log_entry = false;
};

struct BumbleBeeRamData {
	float humidity = 0;
	float temperature = 0;
	float pressure = 0;
	float weight = 0;
	uint8_t lb0 = 0;
#ifdef LB1
	uint8_t lb1 = 0;
#endif
	uint8_t edge_lb0 = 0;
#ifdef LB1
	uint8_t edge_lb1 = 0;
#endif
	int8_t dir = 0;
	int v_batt = 0;
	uint32_t ts = 0;
	uint16_t ts_tick_count = 0;
	uint8_t wakeup_countdown = 0;
};//size 32 (mit lb1 und dir sense)

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
