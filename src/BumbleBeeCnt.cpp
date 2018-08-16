/*
 * BumbleBeeCnt.cpp
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#include <BumbleBeeCnt.h>
#include <time.h>

#ifdef SERIAL_DEBUG
#include "../test/src/serial_debug.h"
#endif

void BumbleBeeCnt::trigger() {
	ExternalEvent(ST_WAKEUP);
//	BEGIN_TRANSITION_MAP
//	TRANSITION_MAP_ENTRY(ST_INIT_PERIPHERALS)//ST_WAKEUP
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_INIT_PERIPHERALS
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_READ_PERIPHERALS
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_PREPARE_SLEEP
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_GOTO_SLEEP
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_ERROR
//	END_TRANSITION_MAP(NULL)
}

int BumbleBeeCnt::init_peripheral_system() {
	int retval = 0;

	if (!bme.begin())
		retval = -DEBUG_ID_BME280;
	else
		DEBUG_MSG(DEBUG_ID_BME280);
	Wire.begin();

	//Only init once after power off.
	if (!mcp.readRegister(MCP23017_IOCONA)) {
		sreg.init();
		mcp.begin();
		for (int n = 0; n < 16; n++) {
			mcp.pullUp(n, LOW);
		}
		mcp.pullUp(7, HIGH);
		mcp.pullUp(6, HIGH);
		mcp.pullUp(5, HIGH);
		mcp.pullUp(0, HIGH);
		for (int n = 0; n < 16; n++)
			mcp.setupInterruptPin(n, CHANGE);
		mcp.setupInterrupts(true, true, LOW);

		DEBUG_MSG(DEBUG_ID_MCP23017)
	}

	ds1307.set(0,0,0,11,8,18);
	ds1307.start();

	pinMode(chipSelectSD, OUTPUT);
	if (!SD.begin(chipSelectSD)) {
		retval |= -DEBUG_ID_SD;
	} else {
		DEBUG_MSG(DEBUG_ID_SD)
	}
	return retval;
}

void BumbleBeeCnt::eval_peripheral_event(uint8_t mcp_gpioa) {
	// data processing here
}

void BumbleBeeCnt::wakeup() {
	DEBUG_MSG(DEBUG_ID_ST_WAKEUP)
	InternalEvent(ST_INIT_PERIPHERALS, NULL);
}

void BumbleBeeCnt::init_peripherals() {
	DEBUG_MSG(DEBUG_ID_ST_INIT_PERIPHERALS)

	int retval = 0;
	states next_state = ST_INIT_PERIPHERALS;
	BumbleBeeCntData *data;

	retval = init_peripheral_system();
	if (retval < 0) {
		next_state = ST_ERROR;
		data = new BumbleBeeCntData;
		data->info = "init_peripheral_system";
	} else {
		next_state = ST_READ_PERIPHERALS;
		data = NULL;
	}
	InternalEvent(next_state, data);
}

void BumbleBeeCnt::read_peripherals() {
	DEBUG_MSG(DEBUG_ID_ST_READ_PERIPHERALS)

	BumbleBeeCntData* peripheral_data;
	peripheral_data = new BumbleBeeCntData;

	peripheral_data->temperature = bme.temp();
	peripheral_data->humidity = bme.hum();

	peripheral_data->mcp_gpioab = mcp.readGPIOAB();
#ifdef SERIAL_DEBUG
	Serial.print("Hum: ");
	Serial.println(bme.hum());
#endif
	InternalEvent(ST_EVAL_PERIPHERAL_DATA, peripheral_data);
}

void BumbleBeeCnt::eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA)

	char date_buffer[80];
	struct tm t;
	BumbleBeeCntData *d_out;
	unsigned lb0 = (p_data->mcp_gpioab & MCP_LB0) ? 1 : 0;
	unsigned lb1 = (p_data->mcp_gpioab & MCP_LB1) ? 1 : 0;

	d_out = new BumbleBeeCntData;

#ifdef SERIAL_DEBUG
	Serial.print("LB0 ");
	Serial.println(lb0);
	Serial.print("LB1 ");
	Serial.println(lb1);
	Serial.print("GPIOAB: ");
	Serial.println(mcp.readGPIOAB());
#endif

	ds1307.get(&t.tm_sec, &t.tm_min, &t.tm_hour, &t.tm_mday, &t.tm_mon,
			&t.tm_year);
	strftime(date_buffer, 80, "%F_%T", &t);

	String date(date_buffer);
	date += ",";
	date += lb0;
	date += ",";
	date += lb1;
	date += ",";
	date += p_data->humidity;
	date += ",";
	date += p_data->temperature;

	d_out->info = date;

#ifdef SERIAL_DEBUG
	Serial.println(date);
#endif

//	eval_peripheral_event(p_data->mcp_gpioa);

//	InternalEvent(ST_WRITE_TO_SD, d_out); //string, den wir schreiben wollen konstruieren wir hier und übergeben ihn als event data.
	InternalEvent(ST_PREPARE_SLEEP, d_out);
}

void BumbleBeeCnt::write_to_sd(BumbleBeeCntData* d) {
	DEBUG_MSG(DEBUG_ID_ST_WRITE_TO_SD)
	File datafile;
	String logstring;

	datafile = SD.open(data_file_name, FILE_WRITE);

	if (datafile) {
		datafile.println(d->info);
	} else {
		ev_data.info = "SD IO-Error";
		InternalEvent(ST_ERROR, &ev_data);
	}
	datafile.close();
	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

void BumbleBeeCnt::prepare_sleep() {
	InternalEvent(ST_GOTO_SLEEP, NULL);
}

void BumbleBeeCnt::goto_sleep() {
#ifdef SERIAL_DEBUG
	Serial.println("State goto_sleep...");
#endif
	ESP.deepSleep(5E6);
//	delay(1000);
//	ESP.restart();
}

void BumbleBeeCnt::error(BumbleBeeCntData *d) {
#ifdef SERIAL_DEBUG
	Serial.print("ERROR --- ");
	Serial.println(d->info);
#endif
}
