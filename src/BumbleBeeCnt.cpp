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

	mcp.begin();
	mcp.pinMode(0, INPUT);
	for (int n = 0; n < 16; n++)
		mcp.pullUp(n, HIGH);
	//mcp.pullUp(0, HIGH);
	mcp.setupInterrupts(true, true, LOW);
	mcp.setupInterruptPin(0, CHANGE);
	DEBUG_MSG(DEBUG_ID_MCP23017)

	pinMode(chipSelectSD, OUTPUT);
	if (!SD.begin(chipSelectSD)) {
		retval |= -DEBUG_ID_SD;
	} else {
		DEBUG_MSG(DEBUG_ID_SD)
	}
	return retval;
}

unsigned BumbleBeeCnt::eval_peripheral_event(uint8_t mcp_gpioa,
		uint8_t mcp_gpiob) {
	uint8_t mcp_gpioa_prev = data_store.getMcpGpioa();
	uint8_t mcp_gpiob_prev = data_store.getMcpGpiob();

	unsigned retval = 0;
	//Check GPIOA
	if ((mcp_gpioa_prev ^ mcp_gpioa) == 1)
		sreg.set_bit(RTC_SREG_BIT_MCP_GPIOA_EVENT);
	else
		sreg.unset_bit(RTC_SREG_BIT_MCP_GPIOA_EVENT);
	//Check GPIOB
	if ((mcp_gpiob_prev ^ mcp_gpiob) == 1)
		sreg.set_bit(RTC_SREG_BIT_MCP_GPIOB_EVENT);
	else
		sreg.unset_bit(RTC_SREG_BIT_MCP_GPIOB_EVENT);
}

void BumbleBeeCnt::wakeup() {
	DEBUG_MSG(DEBUG_ID_ST_WAKEUP)
	InternalEvent(ST_INIT_PERIPHERALS, NULL);
}

void BumbleBeeCnt::init_peripherals() {
	DEBUG_MSG(DEBUG_ID_ST_INIT_PERIPHERALS)
	init_peripheral_system();
	InternalEvent(ST_READ_PERIPHERALS, NULL);
}

void BumbleBeeCnt::read_peripherals() {
	DEBUG_MSG(DEBUG_ID_ST_READ_PERIPHERALS)

	BumbleBeeCntData* peripheral_data;
	peripheral_data = new BumbleBeeCntData;

	peripheral_data->dht_humidity = dht->readHumidity();
	peripheral_data->dht_temperature = dht->readTemperature();
	peripheral_data->mcp_gpioa = mcp.readGPIO(MCP_GPIOA);
	peripheral_data->mcp_gpiob = mcp.readGPIO(MCP_GPIOB);

	InternalEvent(ST_EVAL_PERIPHERAL_DATA, peripheral_data);
}

void BumbleBeeCnt::eval_peripheral_data(BumbleBeeCntData* p_data) {
	eval_peripheral_event(p_data->mcp_gpioa, p_data->mcp_gpiob);

	InternalEvent(ST_WRITE_TO_SD); //string, den wir schreiben wollen konstruieren wir hier und übergeben ihn als event data.
}

void BumbleBeeCnt::write_to_sd() {
	DEBUG_MSG(DEBUG_ID_ST_WRITE_TO_SD)
	File datafile;

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
	delay(1000);
	ESP.restart();
}

void BumbleBeeCnt::error(BumbleBeeCntData *d) {
#ifdef SERIAL_DEBUG
	Serial.println(d->info);
#endif
}
