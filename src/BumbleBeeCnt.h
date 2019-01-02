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

#define MCP_WLAN_EN 0x1
#define MCP_LB0 0x20
#define MCP_LB1 0x40
#define MCP_TARE 0x80

#include "../test/src/serial_debug.h"
#include <StateMachine.h>
#include "../lib/I2C/I2CCom.h"
#include <Arduino.h>
#include <Adafruit_MCP23017.h>
#include "../lib/Ds1307/Ds1307.h"
#include <SD.h>
#include <BME280I2C.h>
#include <HX711_ADC.h>
#include <AccessPoint.h>
#include <EEPROM.h>
#include <types.h>
#include "system_definitions.h"
#include <EventCounter.h>

class BumbleBeeCnt: public StateMachine {
public:
	BumbleBeeCnt() :
			StateMachine(ST_MAX_STATES) {
	}
	void trigger();
private:

	void st_wakeup();
	void st_wifi_init();
	void st_wifi(BumbleBeeCntData *d);
	void st_wifi_end();
	void st_init_peripherals();
	void st_read_peripherals();
	void st_eval_peripheral_data(BumbleBeeCntData *p_data);
	void st_tare();
	void st_write_to_sd(BumbleBeeCntData *d);
	void st_prepare_sleep();
	void st_goto_sleep();
	void st_error(BumbleBeeCntData *d);

	float weight_meas();
	void do_tare();

	int init_peripheral_system();
	int init_peripheral_system_once();
	void read_sensors(BumbleBeeCntData *s_data);
	void read_port_expander(BumbleBeeCntData *p_data);
	void eval_peripheral_event(uint8_t mcp_gpioa);

	BumbleBeeCntData ev_data;

	String data_file_name = sysdefs::general::log_filename;

	uint8_t i2c_reg;

	const unsigned chipSelectSD = D8; //D8

	BEGIN_STATE_MAP
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_wakeup)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_wifi_init)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_wifi)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_wifi_end)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_init_peripherals)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_read_peripherals)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_eval_peripheral_data)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_tare)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_write_to_sd)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_prepare_sleep)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_goto_sleep)
	STATE_MAP_ENTRY(&BumbleBeeCnt::st_error)
	END_STATE_MAP

		enum states {
			ST_WAKEUP = 0,
			ST_WIFI_INIT,
			ST_WIFI,
			ST_WIFI_END,
			ST_INIT_PERIPHERALS,
			ST_READ_PERIPHERALS,
			ST_EVAL_PERIPHERAL_DATA,
			ST_TARE,
			ST_WRITE_TO_SD,
			ST_PREPARE_SLEEP,
			ST_GOTO_SLEEP,
			ST_ERROR,
			ST_MAX_STATES
		};

		//BME280
		BME280I2C bme;

		//  Portexpander
		Adafruit_MCP23017 mcp;

		// Real Time Clock
		Ds1307 ds1307;

		//Scale
//		HX711_ADC scale;
		HX711_ADC scale;

		//Interrupt controller
		i2c::I2CCom attiny88;

		//WebServer
		AccessPoint ap;

		//RTC ram based event counter
		rtc::EventCounter ec;

	}; /* class BUmbleBeeCnt */

#endif /* SRC_BUMBLEBEECNT_H_ */
