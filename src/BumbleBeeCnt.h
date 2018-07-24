/*
 * BumbleBeeCnt.h
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#ifndef SRC_BUMBLEBEECNT_H_
#define SRC_BUMBLEBEECNT_H_

#define SERIAL_DEBUG

#include <StateMachine.h>
#include <RTC.h>
#include <Arduino.h>

struct BumbleBeeCntData: public EventData {

};

class BumbleBeeCnt: public StateMachine {
public:
	BumbleBeeCnt() : StateMachine(ST_MAX_STATES){}
	void trigger();
private:

	void wakeup();
	void init_peripherals();
	void read_peripherals();
	void prepare_sleep();
	void goto_sleep();
	void error();

	void init_peripheral_system();

	const unsigned chipSelectSD = D8; //D8

	BEGIN_STATE_MAP
	STATE_MAP_ENTRY(&BumbleBeeCnt::wakeup)
	STATE_MAP_ENTRY(&BumbleBeeCnt::init_peripherals)
	STATE_MAP_ENTRY(&BumbleBeeCnt::read_peripherals)
	STATE_MAP_ENTRY(&BumbleBeeCnt::prepare_sleep)
	STATE_MAP_ENTRY(&BumbleBeeCnt::goto_sleep)
	STATE_MAP_ENTRY(&BumbleBeeCnt::error)
	END_STATE_MAP

	enum states {
		ST_WAKEUP = 0,
		ST_INIT_PERIPHERALS,
		ST_READ_PERIPHERALS,
		ST_PREPARE_SLEEP,
		ST_GOTO_SLEEP,
		ST_ERROR,
		ST_MAX_STATES
	};

	rtc::SReg sreg;

}; /* class BUmbleBeeCnt */

#endif /* SRC_BUMBLEBEECNT_H_ */
