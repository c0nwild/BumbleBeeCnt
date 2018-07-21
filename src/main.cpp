/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <Adafruit_MCP23017.h>
#include <DS1307.h>
#include <DHT.h>
#include <SPI.h>
#include <SD.h>
#include <SReg.h>
#include "webcontent.h"

extern "C" {
#include "user_interface.h"
}

#include <DateTime.h>

#define SERIAL_DEBUG

/* Set these to your desired credentials. */
const char *ssid = "ESPap";
const char *password = "thereisnospoon";

ESP8266WebServer server(80);

WebContent web_content;


float temperature, humidity;
bool is_initialized = false;
volatile bool getMCP = false;
volatile uint16_t pinval = 0;

//RTC Module
uint16_t year;
uint8_t month, day, dow, hours, minutes, seconds;
DS1307 rtc_shield;

//Access to status register in RTC RAM
rtc::SReg sreg;

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
	server.send(200, "text/html", web_content.output());
}

void getMCPValue() {
	getMCP = true;
}

void init_wifi() {
#ifdef SERIAL_DEBUG
	Serial.print("Configuring access point...");
#endif
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);
	IPAddress myIP = WiFi.softAPIP();
#ifdef SERIAL_DEBUG
	Serial.print("AP IP address: ");
#endif
	Serial.println(myIP);
	server.on("/", handleRoot);
	server.begin();
#ifdef SERIAL_DEBUG
	Serial.println("HTTP server started");
#endif
}

void setup() {
	Serial.begin(115200);
	rtc::rtc_sreg_t sr;
	int query_sreg_bit = 0;
	uint32_t rtc_time;

	rtc_shield.start();

	rtc_shield.get(&seconds, &minutes, &hours, &day, &month, &year);

	time_t t;

	t = DateTime.makeTime(seconds, minutes, hours, day, month, year);

//	temperature = dht.readTemperature();
//	web_content.setTVal(temperature);
//	humidity = dht.readHumidity();
//	web_content.setHVal(humidity);
//	server.handleClient();

	//pinval = mcp.readGPIOAB();

	yield();
	delay(100);
	ESP.deepSleep(10e6);
//
//	query_sreg_bit = sreg.query_bit(RTC_SREG_BIT_SYSTEM_INIT);
//	Serial.print("Query sysinit bit: ");
//	Serial.println(query_sreg_bit);
//
//	if (query_sreg_bit != 1) {
//		init_peripheral_system();
//		sreg.set_bit(RTC_SREG_BIT_SYSTEM_INIT);
//	}
//attachInterrupt(digitalPinToInterrupt(D4), getMCPValue, FALLING);
}

void loop() {

}
