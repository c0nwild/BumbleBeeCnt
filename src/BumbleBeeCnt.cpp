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
}

void BumbleBeeCnt::init_peripherals() {
}

void BumbleBeeCnt::read_peripherals() {
}

void BumbleBeeCnt::prepare_sleep() {
}

void BumbleBeeCnt::goto_sleep() {
}
