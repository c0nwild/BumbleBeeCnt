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
	Ds1307::DateTime init_date = {
			18,
			9,
			11,
			0,
			0,
			0,
			0
	};

	Wire.begin();

	if (!bme.begin())
		retval = -DEBUG_ID_BME280;
	else
		DEBUG_MSG(DEBUG_ID_BME280);

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

	ds1307.setDateTime(&init_date);

	scale.begin();
	scale.start(2000);
	scale.setCalFactor(900.0);

	pinMode(chipSelectSD, OUTPUT);
	if (!SD.begin(chipSelectSD)) {
		retval |= -DEBUG_ID_SD;
	} else {
		DEBUG_MSG(DEBUG_ID_SD)
	}
	return retval;
}

void BumbleBeeCnt::do_tare() {
#ifdef SERIAL_DEBUG
	Serial.println("Tare...");
#endif
	scale.tare();
	InternalEvent(ST_PREPARE_SLEEP, NULL);
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
	peripheral_data->pressure = bme.pres();

	peripheral_data->mcp_gpioab = mcp.readGPIOAB();

	scale.update();
	peripheral_data->weight = scale.getData();

	InternalEvent(ST_EVAL_PERIPHERAL_DATA, peripheral_data);
}

void BumbleBeeCnt::eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA)

	char date_buffer[80];
	struct tm t;
	BumbleBeeCntData *d_out;
	p_data->lb0 = (p_data->mcp_gpioab & MCP_LB0) ? 1 : 0;
	p_data->lb1 = (p_data->mcp_gpioab & MCP_LB1) ? 1 : 0;
	p_data->wlan_en = (p_data->mcp_gpioab & MCP_WLAN_EN) ? 1 : 0;
	p_data->tare = (p_data->mcp_gpioab & MCP_TARE) ? 1 : 0;

	d_out = new BumbleBeeCntData;

	Ds1307::DateTime dt;

#ifdef SERIAL_DEBUG
	Serial.print("LB0 ");
	Serial.println(p_data->lb0);
	Serial.print("LB1 ");
	Serial.println(p_data->lb0);
	Serial.print("GPIOAB: ");
	Serial.println(p_data->mcp_gpioab, HEX);
#endif

	ds1307.getDateTime(&dt);

	t.tm_hour = dt.hour;
	t.tm_min = dt.minute;
	t.tm_sec = dt.second;
	t.tm_year = dt.year + 100;
	t.tm_mon = dt.month;
	t.tm_mday = dt.day;

	strftime(date_buffer, 80, "%F_%T", &t);

	String date(date_buffer);
	date += ",";
	date += p_data->lb0;
	date += ",";
	date += p_data->lb1;
	date += ",";
	date += p_data->humidity;
	date += ",";
	date += p_data->temperature;
	date += ",";
	date += p_data->pressure;
	date += ",";
	date += p_data->weight;

	*d_out = *p_data;
	d_out->info = date;

#ifdef SERIAL_DEBUG
	Serial.println(date);
#endif
//  Wird für Schreibvorgang auf SD benötigt. delete d_out muss dann wieder weg.
	delete d_out;
//	eval_peripheral_event(p_data->mcp_gpioa);

//	InternalEvent(ST_WRITE_TO_SD, d_out); //string, den wir schreiben wollen konstruieren wir hier und übergeben ihn als event data.
	if(p_data->tare)
		InternalEvent(ST_TARE, NULL);
	else
		InternalEvent(ST_PREPARE_SLEEP, NULL);
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
//	ESP.deepSleep(5E6);
	delay(1000);
//	ESP.restart();
	InternalEvent(ST_READ_PERIPHERALS, NULL);
}

void BumbleBeeCnt::error(BumbleBeeCntData *d) {
#ifdef SERIAL_DEBUG
	Serial.print("ERROR --- ");
	Serial.println(d->info);
#endif
}
