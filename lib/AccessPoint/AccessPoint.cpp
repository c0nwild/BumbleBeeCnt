/*
 * AccessPoint.cpp
 *
 *  Created on: 25.11.2018
 *      Author: conradwild
 */

#include "AccessPoint.h"

ESP8266WebServer AccessPoint::server;
WebContent AccessPoint::web_content;

static void handle_root() {
	WebContent *wc;
	String heading;
	String line1;
	String line2;
	String line3;

	wc = &AccessPoint::web_content;

	wc->clear();

	heading = wc->create_heading("HummelCounter");
	line1 = wc->create_entry("Line1");

	wc->append(heading);
	wc->append(line1);

	AccessPoint::server.send(200, "text/html",
			AccessPoint::web_content.output());
}

int AccessPoint::initWifi() {

#ifdef SERIAL_DEBUG
	Serial.print("Configuring access point...");
#endif
	const char *ssid = sysdefs::wifi::ssid.c_str();
	const char *password = sysdefs::wifi::passphrase.c_str();
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);
	IPAddress myIP = WiFi.softAPIP();
#ifdef SERIAL_DEBUG
	Serial.print("AP IP address: ");
	Serial.println(myIP);
#endif
	server.on("/", handle_root);
	server.begin();
#ifdef SERIAL_DEBUG
	Serial.println("HTTP server started");
#endif
	return 0;
}

AccessPoint::AccessPoint() {
	// TODO Auto-generated constructor stub

}

AccessPoint::~AccessPoint() {
	// TODO Auto-generated destructor stub
}

void AccessPoint::handleClient() {
	server.handleClient();
}


int AccessPoint::stopWifi(){
	server.stop();
	return 0;
}

