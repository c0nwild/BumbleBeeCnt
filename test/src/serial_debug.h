/*
 * serial_debug.h
 *
 *  Created on: 04.08.2018
 *      Author: conradwild
 */

#ifndef TEST_SRC_SERIAL_DEBUG_H_
#define TEST_SRC_SERIAL_DEBUG_H_

#ifdef SERIAL_DEBUG
#define DEBUG_MSG(x) \
			Serial.println(x);
#define DEBUG_MSG_PASS(x) \
			Serial.println(x + "... OK");
#define DEBUG_MSG_FAIL(x) \
			Serial.println(x + "... FAIL");
	#ifdef SERIAL_DEBUG_STATES
	#define DEBUG_MSG_ARG(x,t) \
			Serial.println(x, t);
	#else //SERIAL_DEBUG_STATES
	#define DEBUG_MSG_ARG(x,t)
	#endif //SERIAL_DEBUG_STATES
#else //SERIAL_DEBUG
#define DEBUG_MSG(x)
#define DEBUG_MSG_ARG(x,t)
#define DEBUG_MSG_PASS(x)
#define DEBUB_MSG_FAIL(x)
#endif //SERIAL_DEBUG

//Debug ID´s for peripheral hardware
#define DEBUG_ID_BME280 (0x1)
#define DEBUG_ID_MCP23017 (0x2)
#define DEBUG_ID_SD (0x4)

//Debug ID´s for St.Mach. States
#define DEBUG_ID_ST_WAKEUP (0x101)
#define DEBUG_ID_ST_INIT_PERIPHERALS (0x102)
#define DEBUG_ID_ST_READ_PERIPHERALS (0x104)
#define DEBUG_ID_ST_EVAL_PERIPHERAL_DATA (0x108)
#define DEBUG_ID_ST_WRITE_TO_SD (0x110)
#define DEBUG_ID_ST_PREPARE_SLEEP (0x120)
#define DEBUG_ID_ST_GOTO_SLEEP (0x140)
#define DEBUG_ID_ST_ERROR (0x180)
#endif /* TEST_SRC_SERIAL_DEBUG_H_ */
