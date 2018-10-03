#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <BumbleBeeCnt.h>
#include "webcontent.h"

#include "../lib/RTC/src/SReg.h"

extern "C" {
#include "user_interface.h"
}

#include <DateTime.h>

#define SERIAL_DEBUG

BumbleBeeCnt st_machine_hw_test;

void trigger_st_machine() {
	st_machine_hw_test.trigger();
}

void setup() {
	Serial.begin(115200);
}

void loop() {
	trigger_st_machine();
}
