/*
 * BumbleBeeCnt.h
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#ifndef SRC_BUMBLEBEECNT_H_
#define SRC_BUMBLEBEECNT_H_

#define SERIAL_DEBUG

#define MCP_GPIOA 0u
#define MCP_GPIOB 1u

#define MCP_LB0 0x20
#define MCP_LB1 0x40

#include <StateMachine.h>
#include "../lib/RTC/src/SReg.h"
#include "../lib/DataStore.h"
#include <Arduino.h>
#include <Adafruit_MCP23017.h>
#include <DS1307.h>
#include <DHT.h>
#include <SD.h>
#include <WiFiClient.h>

struct BumbleBeeCntData: public EventData {
	String info;
	float dht_humidity = 0;
	float dht_temperature = 0;
	uint8_t lb0 = 0;
	uint8_t lb1 = 0;
	uint16_t mcp_gpioab = 0;
};

class BumbleBeeCnt: public StateMachine {
public:
	BumbleBeeCnt() :
			StateMachine(ST_MAX_STATES) {
	}
	void trigger();
private:

	void wakeup();
	void init_peripherals();
	void read_peripherals();
	void eval_peripheral_data(BumbleBeeCntData *p_data);
	void write_to_sd(BumbleBeeCntData *d);
	void prepare_sleep();
	void goto_sleep();
	void error(BumbleBeeCntData *d);

	int init_peripheral_system();
	void eval_peripheral_event(uint8_t mcp_gpioa);

	BumbleBeeCntData ev_data;

	String data_file_name = "data.txt";

	const unsigned chipSelectSD = D8; //D8

BEGIN_STATE_MAP
STATE_MAP_ENTRY		(&BumbleBeeCnt::wakeup)
		STATE_MAP_ENTRY(&BumbleBeeCnt::init_peripherals)
		STATE_MAP_ENTRY(&BumbleBeeCnt::read_peripherals)
		STATE_MAP_ENTRY(&BumbleBeeCnt::eval_peripheral_data)
		STATE_MAP_ENTRY(&BumbleBeeCnt::write_to_sd)
		STATE_MAP_ENTRY(&BumbleBeeCnt::prepare_sleep)
		STATE_MAP_ENTRY(&BumbleBeeCnt::goto_sleep)
		STATE_MAP_ENTRY(&BumbleBeeCnt::error)
		END_STATE_MAP

		enum states {
			ST_WAKEUP = 0,
			ST_INIT_PERIPHERALS,
			ST_READ_PERIPHERALS,
			ST_EVAL_PERIPHERAL_DATA,
			ST_WRITE_TO_SD,
			ST_PREPARE_SLEEP,
			ST_GOTO_SLEEP,
			ST_ERROR,
			ST_MAX_STATES
		};

		rtc::SReg sreg;

		// DHT Sensor
		const unsigned DHTPin = D3;

		DHT dht22;

		//  Portexpander
		Adafruit_MCP23017 mcp;

		// Real Time Clock
		DS1307 ds1307;

	}; /* class BUmbleBeeCnt */

#endif /* SRC_BUMBLEBEECNT_H_ */
