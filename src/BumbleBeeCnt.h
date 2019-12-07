/** @file BumbleBeeCnt.h
 * @brief The infamous Bumble Bee Counter
 *
 */

#ifndef SRC_BUMBLEBEECNT_H_
#define SRC_BUMBLEBEECNT_H_

#include <sysconfig.h>
#include "../test/src/serial_debug.h"
#include <Wire.h>
#include <StateMachine.h>
#include <Arduino.h>
#include <Adafruit_MCP23017.h>
#include "../lib/Ds1307/Ds1307.h"
#include <SD.h>
#include <BME280I2C.h>
#include <AccessPoint.h>
#include <EEPROM.h>
#include <types.h>
#include <system_definitions.h>
#include <EventCounter.h>
#include <RTCDataBuffer.h>
#include <ArduinoJson.h>
#include <HX711.h>
#include <IRQController.h>

/** @brief BumbleBeeCnt class implements a state machine that coordinates
 * logging and power management
 *
 * This is the main class for a esp8266 based datalogger. The main features comprise:
 * - weight log
 * - barometer / temperature log
 * - activity log
 * - power management for low battery consumption
*/
class BumbleBeeCnt: public StateMachine {
public:
	BumbleBeeCnt() :
			StateMachine(ST_MAX_STATES),
			evc0(sysdefs::rtc::rtc_eventcnt0),
#ifdef LB1
			evc1(sysdefs::rtc::rtc_eventcnt1),
#endif
			rtc_buf(sysdefs::rtc::rtc_bmbcnt_data)
			{
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
	/** @brief Power management method
	 *
	 * This method triggers the external usb power bank, if the internal
	 * buffer battery goes below a predefined voltage threshold. Check is
	 * performed along with writing to the log file all 10 minutes.
	 */
	void st_power_management(BumbleBeeCntData *p_data);
	void st_tare();
	void st_write_to_sd(BumbleBeeCntData *d);
	void st_prepare_sleep();
	void st_goto_sleep();
	void st_error(BumbleBeeCntData *d);
	void st_fatal_error(BumbleBeeCntData *d);

	void prepare_cal();
	void do_cal();
	float weight_meas();
	void do_tare();
	void do_calibration();

	int init_peripheral_system();
	int init_peripheral_system_once();
	void read_sensors(BumbleBeeCntData *s_data);
	void read_port_expander(BumbleBeeCntData *p_data);
	void eval_peripheral_event(uint8_t mcp_gpioa);
	String prepare_log_str(Ds1307::DateTime dt,
			BumbleBeeCntData *d);

	String data_file_name = sysdefs::general::log_filename;
	String error_file_name = sysdefs::general::error_filename;
	String config_file_name = sysdefs::general::config_filename;

	uint8_t i2c_reg = 0;

	BEGIN_STATE_MAP
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_wakeup)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_wifi_init)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_wifi)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_wifi_end)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_init_peripherals)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_read_peripherals)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_eval_peripheral_data)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_power_management)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_tare)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_write_to_sd)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_prepare_sleep)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_goto_sleep)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_error)
		STATE_MAP_ENTRY(&BumbleBeeCnt::st_fatal_error)
	END_STATE_MAP

	enum states {
		ST_WAKEUP = 0,
		ST_WIFI_INIT,
		ST_WIFI,
		ST_WIFI_END,
		ST_INIT_PERIPHERALS,
		ST_READ_PERIPHERALS,
		ST_EVAL_PERIPHERAL_DATA,
		ST_POWER_MANAGEMENT,
		ST_TARE,
		ST_WRITE_TO_SD,
		ST_PREPARE_SLEEP,
		ST_GOTO_SLEEP,
		ST_ERROR,
		ST_FATAL_ERROR,
		ST_MAX_STATES
	};

	Ds1307::DateTime dt;
	long ts = 0;
	uint64_t reset_cntdown = 0; //Time until esp has to wake itself.

	//BME280
	BME280I2C bme;

	//  Portexpander
	Adafruit_MCP23017 mcp;

	// Real Time Clock
	Ds1307 ds1307;

	//Scale
	//		HX711_ADC scale;
	HX711 scale;

	//Interrupt controller
	IRQController irqctl;

	//WebServer
	AccessPoint ap;

	//RTC ram based event counter, one for each channel
	rtc::EventCounter evc0;
#ifdef LB1
	rtc::EventCounter evc1;
#endif

	//RTC ram based data buffer
	rtc::RTCDataBuffer rtc_buf;

}; /* class BUmbleBeeCnt */

#endif /* SRC_BUMBLEBEECNT_H_ */
