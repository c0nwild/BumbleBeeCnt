/*
 * BumbleBeeCnt.cpp
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#include <BumbleBeeCnt.h>
#include <Adafruit_MCP23017.h>
#include <DS1307.h>
#include <DHT.h>
#include <SD.h>
#include <WiFiClient.h>

#define DHTTYPE DHT22

// DHT Sensor
const int DHTPin = 0;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Initialize Portexpander
Adafruit_MCP23017 mcp;

void BumbleBeeCnt::trigger() {
	BEGIN_TRANSITION_MAP
	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//st_wakeup
	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_INIT_PERIPHERALS
	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_READ_PERIPHERALS
	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_PREPARE_SLEEP
	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_GOTO_SLEEP
	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_ERROR
	END_TRANSITION_MAP(NULL)
}

void BumbleBeeCnt::init_peripheral_system() {
#ifdef SERIAL_DEBUG
	Serial.print("Configuring dht22...");
#endif
	dht.begin();
#ifdef SERIAL_DEBUG
	Serial.println("Done...");
#endif
#ifdef SERIAL_DEBUG
	Serial.print("Configuring mcp23017...");
#endif
	mcp.begin();
	mcp.pinMode(0, INPUT);
	for (int n = 0; n < 16; n++)
		mcp.pullUp(n, HIGH);
	//mcp.pullUp(0, HIGH);
	mcp.setupInterrupts(true, true, LOW);
	mcp.setupInterruptPin(0, CHANGE);
#ifdef SERIAL_DEBUG
	Serial.println("Done...");
#endif
	pinMode(D4, INPUT);
	digitalWrite(D4, INPUT_PULLUP);
	SD.begin(chipSelectSD);
}

void BumbleBeeCnt::wakeup() {
#ifdef SERIAL_DEBUG
	Serial.println("State wakeup...");
#endif
	int sr_poweron = this->sreg.query_bit(RTC_SREG_BIT_POWERON);
	this->sreg.unset_bit(RTC_SREG_BIT_POWERON);
	unsigned char newState = 0;
	switch (sr_poweron){
	case 0:
		newState = ST_INIT_PERIPHERALS;
		break;
	case 1:
		newState = ST_READ_PERIPHERALS;
		break;
	default:
		newState = ST_ERROR;
		break;
	}
	InternalEvent(newState, NULL);
}

void BumbleBeeCnt::init_peripherals() {
#ifdef SERIAL_DEBUG
	Serial.println("State init_peripherals...");
#endif
	init_peripheral_system();
	InternalEvent(ST_READ_PERIPHERALS, NULL);
}

void BumbleBeeCnt::read_peripherals() {
#ifdef SERIAL_DEBUG
	Serial.println("State read_peripherals...");
#endif
}

void BumbleBeeCnt::prepare_sleep() {
}

void BumbleBeeCnt::goto_sleep() {
	sreg.set_bit(RTC_SREG_BIT_POWERON);
	ESP.deepSleep(10e6);
}

void BumbleBeeCnt::error() {
#ifdef SERIAL_DEBUG
	Serial.println("State error...");
#endif
}
