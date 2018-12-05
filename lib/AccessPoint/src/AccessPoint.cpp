/*
 * AccessPoint.cpp
 *
 *  Created on: 25.11.2018
 *      Author: conradwild
 */

#include "AccessPoint.h"

WiFiServer AccessPoint::server;
WebContent AccessPoint::web_content;

String AccessPoint::_time;
bool AccessPoint::update_time;

int AccessPoint::initWifi() {
	const char *ssid = sysdefs::wifi::ssid.c_str();
	const char *password = sysdefs::wifi::passphrase.c_str();

	// setup globals
	ulReqcount = 0;

	// AP mode
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, password);
	server.begin();

	DEBUG_MSG("AP started...");
	return 0;
}

AccessPoint::AccessPoint() {
	// TODO Auto-generated constructor stub
}

AccessPoint::~AccessPoint() {
	// TODO Auto-generated destructor stub
}

String AccessPoint::getTimeString() {
	String _time_loc = _time;
	_time = "";
	return _time_loc;
}

void AccessPoint::handleClient() {
	// Check if a client has connected
	client = server.available();
	if (!client) {
		return;
	}

	// Wait until the client sends some data
	Serial.println("new client");
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

	///////////////////////////////////////////////////////////////////////////////
	// output parameters to serial, you may connect e.g. an Arduino and react on it
	///////////////////////////////////////////////////////////////////////////////
	if (sParam.startsWith("?time")) {
		int iEqu = sParam.indexOf("=");
		if (iEqu >= 0) {
			sCmd = sParam.substring(iEqu + 1, sParam.length());
			_time = sCmd;
			Serial.println(_time);
		}
	}

	///////////////////////////
	// format the html response
	///////////////////////////
	String sResponse, sHeader;

	///////////////////////
	// format the html page
	///////////////////////
	if (sPath == "/") {
		ulReqcount++;
		sResponse = String(WebContent::webpage_main);

		sHeader = "HTTP/1.1 200 OK\r\n";
		sHeader += "Content-Length: ";
		sHeader += sResponse.length();
		sHeader += "\r\n";
		sHeader += "Content-Type: text/html\r\n";
		sHeader += "Connection: close\r\n";
		sHeader += "\r\n";

		client.print(sHeader);
		client.print(sResponse);

	} else if (sPath == "/download") {
		File logFile = SD.open("DATA.TXT");

		sHeader = "HTTP/1.1 200 OK\r\n";
		sHeader += "Content-Length: ";
		sHeader += logFile.size();
		sHeader += "\r\n";
		sHeader += "Content-Type: application/octet-stream\r\n";
		sHeader += "Connection: close\r\n";
		sHeader += "\r\n";

		client.print(sHeader);
		client.write(logFile);
		logFile.close();

	} else if (sPath == "/settings") {
		sResponse = String(WebContent::webpage_settings);

		sHeader = "HTTP/1.1 200 OK\r\n";
		sHeader += "Content-Length: ";
		sHeader += sResponse.length();
		sHeader += "\r\n";
		sHeader += "Content-Type: text/html\r\n";
		sHeader += "Connection: close\r\n";
		sHeader += "\r\n";

		client.print(sHeader);
		client.print(sResponse);

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
