/*
 * AccessPoint.cpp
 *
 *  Created on: 25.11.2018
 *      Author: conradwild
 */

#include "AccessPoint.h"
#include <cstring>

WiFiServer AccessPoint::server(80);
WebContent AccessPoint::web_content;

String AccessPoint::_time;
String AccessPoint::_scale_calib;
bool AccessPoint::update_time;

int AccessPoint::initWifi() {
	// AP mode
	WiFi.mode(WIFI_AP);
	WiFi.softAP(_ssid.c_str(), _password.c_str());
	server.begin();

	DEBUG_MSG("AP started...");
	return 0;
}

AccessPoint::AccessPoint() {
}

AccessPoint::~AccessPoint() {
}

String AccessPoint::getTimeString() {
	String _time_loc = _time;
	_time = "";
	return _time_loc;
}

String AccessPoint::getScaleCalibString() {
	String _scale_calib_loc = _scale_calib;
	_scale_calib = "";
	return _scale_calib_loc;
}

bool AccessPoint::sendHTMLcontent(unsigned http_code, WiFiClient client, String content) {
	String sHeader = "HTTP/1.1 ";
	sHeader += String(http_code);
	sHeader += " OK\r\n";
	sHeader += "Content-Length: ";
	sHeader += content.length();
	sHeader += "\r\n";
	sHeader += "Content-Type: text/html\r\n";
	sHeader += "Connection: close\r\n";
	sHeader += "\r\n";

	client.print(sHeader);
	client.print(content);
	return true;
}

bool AccessPoint::setPeripheralData(BumbleBeeCntData p_data) {
	peripheral_data = p_data;
	return true;
}

String AccessPoint::retrieveParams(String parameter_string,
		String parameter_name) {
	String pvalue = "";
	if (parameter_string.startsWith("?" + parameter_name)) {
		int iEqu = parameter_string.indexOf("=");
		if (iEqu >= 0) {
			pvalue = parameter_string.substring(iEqu + 1,
					parameter_string.length());
		}
	}
	return pvalue;
}

String AccessPoint::getCurrentPath() {
	DEBUG_MSG(current_path);
	return current_path;
}

bool AccessPoint::setWeight(float w) {
	peripheral_data.weight = w;
	return true;
}

float AccessPoint::getWeight() {
	return peripheral_data.weight;
}

bool AccessPoint::setDateTime(Ds1307::DateTime d) {
	dt = d;
	return true;
}

bool AccessPoint::setPasswd(String pw) {
	_password = pw;
	return true;
}

bool AccessPoint::setSSID(String id) {
	_ssid = id;
	return true;
}

bool AccessPoint::set_need_weight() {
	return _need_weight;
}

void AccessPoint::unset_need_weight() {
	_need_weight = false;
}

void AccessPoint::handleClient() {
	// Check if a client has connected
	client = server.available();
	if (!client) {
		return;
	}

	// Wait until the client sends some data
	unsigned long ultimeout = millis() + 250;
	while (!client.available() && (millis() < ultimeout)) {
		delay(1);
	}
	if (millis() > ultimeout) {
		Serial.println("client connection time-out!");
		return;
	}

	// Read the first line of the request
	String sRequest = client.readStringUntil('\r');
	//Serial.println(sRequest);
	client.flush();

	// stop client, if request is empty
	if (sRequest == "") {
		Serial.println("empty request! - stopping client");
		client.stop();
		return;
	}

	// get path; end of path is either space or ?
	// Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
	String sPath = "", sParam = "", sCmd = "";
	String sGetstart = "GET ";
	int iStart, iEndSpace, iEndQuest;
	iStart = sRequest.indexOf(sGetstart);
	if (iStart >= 0) {
		iStart += +sGetstart.length();
		iEndSpace = sRequest.indexOf(" ", iStart);
		iEndQuest = sRequest.indexOf("?", iStart);

		// are there parameters?
		if (iEndSpace > 0) {
			if (iEndQuest > 0) {
				// there are parameters
				sPath = sRequest.substring(iStart, iEndQuest);
				sParam = sRequest.substring(iEndQuest, iEndSpace);
			} else {
				// NO parameters
				sPath = sRequest.substring(iStart, iEndSpace);
			}
		}
	}

	current_path = sPath;

	///////////////////////////////////////////////////////////////////////////////
	// output parameters to serial, you may connect e.g. an Arduino and react on it
	///////////////////////////////////////////////////////////////////////////////
	_time = retrieveParams(sParam, "rtcset");
	Serial.println(_time);
	_scale_calib = retrieveParams(sParam, "scale_calib");
	Serial.println(_scale_calib);

	///////////////////////////
	// format the html response
	///////////////////////////
	String sResponse, sHeader;

	///////////////////////
	// format the html page
	///////////////////////
	if (sPath == "/") {
		String content_head = WebContent::webpage_head;
		String content_body = WebContent::webpage_body_main;
		String content_ssid = "\n<div>SSID: " + String(_ssid) + "</div>\n";
		String content_tail = WebContent::webpage_tail;

		sendHTMLcontent(200, client,
				content_head + content_body + content_ssid + content_tail);

	} else if (sPath == "/download") {
		File logFile = SD.open("DATA.TXT");
		String filename = _ssid + "_" + String((uint16_t) dt.year + 2000) + "-"
				+ String(dt.month) + "-" + String(dt.day) + ".txt";

		sHeader = "HTTP/1.1 200 OK\r\n";
		sHeader += "Content-Length: ";
		sHeader += logFile.size();
		sHeader += "\r\n";
		sHeader += "Content-Type: application/text\r\n";
		sHeader += "Content-Disposition: attachment; name='DATA'; filename=" + filename + "\r\n";
		sHeader += "Connection: close\r\n";
		sHeader += "\r\n";

		client.print(sHeader);
		client.write(logFile);
		logFile.close();

	} else if (sPath == "/settings") {
		sendHTMLcontent(200, client, WebContent::webpage_settings);

	} else if (sPath == "/sensors") {
		String content = "";
		_need_weight = true;
		web_content.clear();
		content = "Time: " + String((uint16_t) dt.year + 2000) + "-"
				+ String(dt.month) + "-" + String(dt.day) + "_"
				+ String(dt.hour) + ":" + String(dt.minute) + ":"
				+ String(dt.second);
		web_content.append(content);
		content = web_content.create_weight_entry(peripheral_data.weight);
		web_content.append(content);
		content = web_content.create_temp_entry(peripheral_data.temperature);
		web_content.append(content);
		content = web_content.create_humid_entry(peripheral_data.humidity);
		web_content.append(content);

		sendHTMLcontent(206, client, web_content.output());

	} else if (sPath == "/scale_calib") {
		String content = "";
		float calib;
		EEPROM.begin(128);
		EEPROM.get(0, calib);
		EEPROM.end();

		web_content.clear();
		content = web_content.create_weight_entry(peripheral_data.weight);
		web_content.append(content);
		content = "Current scale cal. factor: " + String(calib);
		web_content.append(content);
		content = web_content.create_input_form("scale_calib",
				"Scale cal value");
		web_content.append(content);

		sendHTMLcontent(205, client, web_content.output());

	} else if (sPath == "/wipe_log_file") {
		String ret_str;
		boolean rv;

		rv = SD.remove(sysdefs::general::log_filename);

		ret_str = (rv) ? "Done" : "Error";

		sendHTMLcontent(200, client, ret_str);

	} else if (sPath != "/") {
		File logFile = SD.open(sPath);

		if (!logFile) {
			sResponse =
					"<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";

			sHeader = "HTTP/1.1 404 Not found\r\n";
			sHeader += "Content-Length: ";
			sHeader += sResponse.length();
			sHeader += "\r\n";
			sHeader += "Content-Type: text/html\r\n";
			sHeader += "Connection: close\r\n";
			sHeader += "\r\n";

			client.print(sHeader);
			client.print(sResponse);
			return;
		}

		String fileName = logFile.name();
		String content_type = "application/octet-stream";

		if (fileName.endsWith(".css")) {
			content_type = "text/css";
		}

		sHeader = "HTTP/1.1 200 OK\r\n";
		sHeader += "Content-Length: ";
		sHeader += logFile.size();
		sHeader += "\r\n";
		sHeader += "Content-Type: ";
		sHeader += content_type;
		sHeader += "\r\n";
		sHeader += "Connection: close\r\n";
		sHeader += "\r\n";

		Serial.println(logFile.name());

		client.print(sHeader);
		client.write(logFile);
		logFile.close();
	}
}

int AccessPoint::stopWifi() {

// and stop the client
	client.stop();
	Serial.println("Client disonnected");
	return 0;
}
