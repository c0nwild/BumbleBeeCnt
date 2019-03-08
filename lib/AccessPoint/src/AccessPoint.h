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

	bool _need_weight = false;
	bool _prepare_cal = false;
	bool _do_cal = false;

	String retrieveParams(String parameter_string, String parameter_name);

public:
	static WebContent web_content;
	static WiFiServer server;
	WiFiClient client;

	static String _time;
	static String _scale_calib;
	static bool update_time;

	AccessPoint();
	virtual ~AccessPoint();

	int initWifi();
	int stopWifi();bool sendHTMLcontent(unsigned http_code, WiFiClient client, String content);
	String getCurrentPath();

	String getTimeString();
	String getScaleCalibString();
	bool setPeripheralData(BumbleBeeCntData p_data);
	bool setDateTime(Ds1307::DateTime d);
	bool setWeight(float w);
	float getWeight();
	bool setPasswd(String pw);
	bool setSSID(String id);

	inline bool get_need_weight(){return _need_weight;}
	inline void unset_need_weight(){_need_weight = false;}

	inline bool get_prepare_cal(){return _prepare_cal;}
	inline void unset_prepare_cal(){_prepare_cal = false;}
	inline bool get_do_cal(){return _do_cal;}
	inline void unset_do_cal(){_do_cal = false;}

	void handleClient();
};

#endif /* LIB_ACCESSPOINT_ACCESSPOINT_H_ */
