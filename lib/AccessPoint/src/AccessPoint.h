/*
 * AccessPoint.h
 *
 *  Created on: 25.11.2018
 *      Author: conradwild
 */

#ifndef LIB_ACCESSPOINT_ACCESSPOINT_H_
#define LIB_ACCESSPOINT_ACCESSPOINT_H_

#define SERIAL_DEBUG

#include "../../test/src/serial_debug.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <webcontent.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <SD.h>
#include <system_definitions.h>

class AccessPoint {
private:
	unsigned long ulReqcount;

public:
	static WebContent web_content;
	static WiFiServer server;
	WiFiClient client;

	static String _time;
	static bool update_time;

	AccessPoint();
	virtual ~AccessPoint();

	int initWifi();
	int stopWifi();

	String getTimeString();

	void handleClient();
};

#endif /* LIB_ACCESSPOINT_ACCESSPOINT_H_ */
