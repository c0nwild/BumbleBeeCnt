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
#include <EEPROM.h>
#include <Ds1307.h>
#include <system_definitions.h>
#include <types.h>

class AccessPoint {
private:
	BumbleBeeCntData peripheral_data;
	String current_path;

	Ds1307::DateTime dt;
	String _ssid;
	String _password;

	String retrieveParams(String parameter_string, String parameter_name);

public:
	static WebContent web_content;
	static WiFiServer server;
	WiFiClient client;

	static String _time;
	static String _scale_calib;
	static bool update_time;

	AccessPoint(String ssid, String passwd);
	virtual ~AccessPoint();

	int initWifi();
	int stopWifi();bool sendHTMLcontent(WiFiClient client, String content);
	String getCurrentPath();

	String getTimeString();
	String getScaleCalibString();
	bool setPeripheralData(BumbleBeeCntData p_data);
	bool setDateTime(Ds1307::DateTime d);
	bool setWeight(float w);
	float getWeight();

	void handleClient();
};

#endif /* LIB_ACCESSPOINT_ACCESSPOINT_H_ */
