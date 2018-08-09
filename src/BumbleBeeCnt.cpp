/*
 * BumbleBeeCnt.cpp
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#include <BumbleBeeCnt.h>

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


	dht->begin();
	DEBUG_MSG(DEBUG_ID_DHT22)

	Wire.begin();
#ifdef SERIAL_DEBUG
	Serial.print("IOCONA ");
	Serial.println(mcp.readRegister(MCP23017_IOCONA));
#endif
	//Only init once after power off.
	if (!mcp.readRegister(MCP23017_IOCONA)) {
		sreg.init();
		mcp.begin();
		for (int n = 0; n < 16; n++){
			mcp.pullUp(n, LOW);
		}
		mcp.pullUp(7, HIGH);
		mcp.pullUp(6, HIGH);
		mcp.pullUp(5, HIGH);
		mcp.pullUp(0, HIGH);
		for (int n = 0; n < 16; n++)
			mcp.setupInterruptPin(n, CHANGE);
		mcp.setupInterrupts(false, true, LOW);

		DEBUG_MSG(DEBUG_ID_MCP23017)
	}

	pinMode(chipSelectSD, OUTPUT);
	if (!SD.begin(chipSelectSD)) {
		retval |= -DEBUG_ID_SD;
	} else {
		DEBUG_MSG(DEBUG_ID_SD)
	}
	return retval;
}

void BumbleBeeCnt::eval_peripheral_event(uint8_t mcp_gpioa) {
	//Check GPIOA
	if (sreg.query_bit(RTC_SREG_BIT_LS0) == 0)
		sreg.set_bit(RTC_SREG_BIT_LS0);
	else
		sreg.unset_bit(RTC_SREG_BIT_LS0);

#ifdef SERIAL_DEBUG
	Serial.print("sreg ");
	Serial.println(sreg.query_bit(RTC_SREG_BIT_LS0));
#endif
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

	peripheral_data->dht_humidity = dht->readHumidity();
	peripheral_data->dht_temperature = dht->readTemperature();
	peripheral_data->mcp_gpioa = mcp.readGPIO(MCP_GPIOA);
#ifdef SERIAL_DEBUG
	Serial.print("GPIOA: ");
	Serial.println(mcp.readGPIO(MCP_GPIOA), HEX);
#endif
	InternalEvent(ST_EVAL_PERIPHERAL_DATA, peripheral_data);
}

void BumbleBeeCnt::eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA)
	unsigned eval_out = 0;
	eval_peripheral_event(p_data->mcp_gpioa);
#ifdef SERIAL_DEBUG
	Serial.print("eval_out: ");
	Serial.println(eval_out, HEX);
#endif
	InternalEvent(ST_WRITE_TO_SD); //string, den wir schreiben wollen konstruieren wir hier und übergeben ihn als event data.
}

void BumbleBeeCnt::write_to_sd(BumbleBeeCntData* d) {
	DEBUG_MSG(DEBUG_ID_ST_WRITE_TO_SD)
	File datafile;
	String logstring;

	datafile = SD.open(data_file_name, FILE_WRITE);

	if (datafile) {
		datafile.println("test");
	} else {
		ev_data.info = "SD IO-Error";
		InternalEvent(ST_ERROR, &ev_data);
	}
	datafile.close();
	InternalEvent(ST_GOTO_SLEEP, NULL);
}

void BumbleBeeCnt::prepare_sleep() {
}

void BumbleBeeCnt::goto_sleep() {
#ifdef SERIAL_DEBUG
	Serial.println("State goto_sleep...");
#endif
	ESP.deepSleep(30E6);
//	delay(1000);
//	ESP.restart();
}

void BumbleBeeCnt::error(BumbleBeeCntData *d) {
#ifdef SERIAL_DEBUG
	Serial.print("ERROR --- ");
	Serial.println(d->info);
#endif
}
