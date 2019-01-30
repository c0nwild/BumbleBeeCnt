extern "C" {
#include "user_interface.h"
}

#include <BumbleBeeCnt.h>

BumbleBeeCnt st_machine_hw_test;

void trigger_st_machine() {
	st_machine_hw_test.trigger();
}

void setup() {
#ifdef SERIAL_DEBUG
	Serial.begin(115200);
#endif
}

void loop() {
	trigger_st_machine();
}
