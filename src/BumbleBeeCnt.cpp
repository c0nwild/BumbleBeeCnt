/*
 * BumbleBeeCnt.cpp
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#include <BumbleBeeCnt.h>
#include <time.h>

#include "../test/src/serial_debug.h"

void BumbleBeeCnt::trigger() {
	ExternalEvent(ST_WAKEUP);
//	BEGIN_TRANSITION_MAP
//	TRANSITION_MAP_ENTRY(ST_INIT_PERIPHERALS)//ST_WAKEUP
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_INIT_PERIPHERALS
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_READ_PERIPHERALS
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_PREPARE_SLEEP
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_GOTO_SLEEP
//	TRANSITION_MAP_ENTRY(EVENT_IGNORED)//ST_ERROR
//	END_TRANSITION_MAP(NULL)
}

int BumbleBeeCnt::init_peripheral_system_once() {
	int retval = 0;
	Ds1307::DateTime init_date = { 18, 9, 11, 0, 0, 0, 0 };

	ds1307.setDateTime(&init_date);

	pinMode(chipSelectSD, OUTPUT);
	if (!SD.begin(chipSelectSD)) {
		retval |= -DEBUG_ID_SD;
	} else {
		DEBUG_MSG(DEBUG_ID_SD)
	}
	return retval;
}

int BumbleBeeCnt::init_peripheral_system() {
	int retval = 0;

	scale.powerDown();

	Wire.begin();

	if (!bme.begin())
		retval = -DEBUG_ID_BME280;
	else
		DEBUG_MSG(DEBUG_ID_BME280);

	//Only init once after power off.
	if (!mcp.readRegister(MCP23017_IOCONA)) {
		sreg.init();
		mcp.begin();
		for (int n = 0; n < 16; n++) {
			mcp.pullUp(n, LOW);
			mcp.pinMode(n, OUTPUT);
		}
		mcp.pinMode(7, INPUT);
		mcp.pullUp(7, HIGH);
		mcp.pinMode(6, INPUT);
		mcp.pullUp(6, HIGH);
		mcp.pinMode(5, INPUT);
		mcp.pullUp(5, HIGH);
		mcp.pinMode(0, INPUT);
		mcp.pullUp(0, HIGH);
		for (int n = 0; n < 16; n++)
			mcp.setupInterruptPin(n, CHANGE);
		mcp.setupInterrupts(true, true, LOW);

		DEBUG_MSG(DEBUG_ID_MCP23017)
	}

	return retval;
}

void BumbleBeeCnt::do_tare() {
#ifdef SERIAL_DEBUG
	Serial.println("Tare...");
#endif
	float tare_value = 0;

	tare_value = weight_meas();



	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

float BumbleBeeCnt::weight_meas() {
#ifdef SERIAL_DEBUG
	Serial.println("Weight meas");
#endif
	float rv = 0;
	uint8_t scale_status = 0;
	long timeout = 0;
	uint8_t meas_cnt = 0;

	scale.begin(D3, D4);
//	scale.start(2000);
	scale.setCalFactor(sysdefs::scale::scale_factor);

	timeout = millis();
	while (true) {
		scale_status = scale.update();
		if (scale_status > 0)
			++meas_cnt;
		if (meas_cnt == 10) {
			rv = scale.getData();
			break;
		}

		if ((millis() - timeout) > 2000) {
#ifdef SERIAL_DEBUG
			Serial.println("timeout!");
#endif
			break;
		}
	}
	return rv;
}

void BumbleBeeCnt::eval_peripheral_event(uint8_t mcp_gpioa) {
// data processing here
}

void BumbleBeeCnt::wakeup() {
	DEBUG_MSG(DEBUG_ID_ST_WAKEUP)
	/* TODO: Hier den Attiny 88 über I2C abfragen, solange keine Freigabe vom Wemos erfolgt
	 * darf der Tiny keinen Reset durchführen.
	 */
	int addr = sysdefs::res_ctrl::i2c_addr;

//get sreg from reset controller
	Wire.begin();
	Wire.requestFrom(addr, 1);
	while (Wire.available()) {
		i2c_reg = Wire.read();
	}
	Wire.endTransmission(true);

#ifdef SERIAL_DEBUG
	String src;
	if (i2c_reg & sysdefs::res_ctrl::int_src_esp)
		src = "ESP";
	else if (i2c_reg & sysdefs::res_ctrl::int_src_mcp)
		src = "MCP";
	else
		src = "undef";

	Serial.print("I2CREG: 0x");
	Serial.print(i2c_reg);
	Serial.println();
	Serial.print("SRC: ");
	Serial.print(src);
	Serial.println();
#endif

	InternalEvent(ST_INIT_PERIPHERALS, NULL);
}

void BumbleBeeCnt::init_peripherals() {
	DEBUG_MSG(DEBUG_ID_ST_INIT_PERIPHERALS)

	int retval = 0;
	states next_state = ST_INIT_PERIPHERALS;
	BumbleBeeCntData *data;

	retval = init_peripheral_system();

	if (!(i2c_reg & sysdefs::res_ctrl::sys_initialized))
		retval |= init_peripheral_system_once();

	if (retval < 0) {
		next_state = ST_ERROR;
		data = new BumbleBeeCntData;
		data->info = "init_peripherals failed";
	} else {
		next_state = ST_READ_PERIPHERALS;
		data = NULL;

		i2c_reg |= sysdefs::res_ctrl::sys_initialized;
		Wire.begin();
		Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
		Wire.write(i2c_reg);
		Wire.endTransmission(true);
	}
	InternalEvent(next_state, data);
}

void BumbleBeeCnt::read_peripherals() {
	DEBUG_MSG(DEBUG_ID_ST_READ_PERIPHERALS)

	BumbleBeeCntData* peripheral_data;
	peripheral_data = new BumbleBeeCntData;

	peripheral_data->temperature = bme.temp();
	peripheral_data->humidity = bme.hum();
	peripheral_data->pressure = bme.pres();

	peripheral_data->mcp_gpioab = mcp.readGPIOAB();

//Weight measurement after fixed time intervals
	if (i2c_reg & sysdefs::res_ctrl::int_src_esp)
		peripheral_data->weight = weight_meas();

	i2c_reg &=
			~(sysdefs::res_ctrl::int_src_esp | sysdefs::res_ctrl::int_src_mcp);
	Wire.begin();
	Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
	Wire.write(i2c_reg);
	Wire.endTransmission(true);

	InternalEvent(ST_EVAL_PERIPHERAL_DATA, peripheral_data);
}

void BumbleBeeCnt::eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA)

	char date_buffer[80];
	struct tm t;
	BumbleBeeCntData *d_out;
	p_data->lb0 = (p_data->mcp_gpioab & MCP_LB0) ? 1 : 0;
	p_data->lb1 = (p_data->mcp_gpioab & MCP_LB1) ? 1 : 0;
	p_data->wlan_en = (p_data->mcp_gpioab & MCP_WLAN_EN) ? 1 : 0;
	p_data->tare = (p_data->mcp_gpioab & MCP_TARE) ? 1 : 0;

	d_out = new BumbleBeeCntData;

	Ds1307::DateTime dt;

#ifdef SERIAL_DEBUG
	Serial.print("GPIOAB: 0b");
	Serial.println(p_data->mcp_gpioab, BIN);
#endif

	ds1307.getDateTime(&dt);

	t.tm_hour = dt.hour;
	t.tm_min = dt.minute;
	t.tm_sec = dt.second;
	t.tm_year = dt.year + 100;
	t.tm_mon = dt.month;
	t.tm_mday = dt.day;

	strftime(date_buffer, 80, "%F_%T", &t);

	String date(date_buffer);
	date += ",";
	date += p_data->lb0;
	date += ",";
	date += p_data->lb1;
	date += ",";
	date += p_data->humidity;
	date += ",";
	date += p_data->temperature;
	date += ",";
	date += p_data->pressure;
	date += ",";
	date += p_data->weight;

	*d_out = *p_data;
	d_out->info = date;

#ifdef SERIAL_DEBUG
	Serial.println(date);
#endif
//  TODO: Wird für Schreibvorgang auf SD benötigt. delete d_out muss dann wieder weg.
	delete d_out;
//	eval_peripheral_event(p_data->mcp_gpioa);

//	InternalEvent(ST_WRITE_TO_SD, d_out); //string, den wir schreiben wollen konstruieren wir hier und übergeben ihn als event data.
	if (p_data->tare)
		InternalEvent(ST_TARE, NULL);
	else
		InternalEvent(ST_PREPARE_SLEEP, NULL);
}

void BumbleBeeCnt::write_to_sd(BumbleBeeCntData* d) {
	DEBUG_MSG(DEBUG_ID_ST_WRITE_TO_SD)
	File datafile;
	String logstring;

	datafile = SD.open(data_file_name, FILE_WRITE);

	if (datafile) {
		datafile.println(d->info);
	} else {
		ev_data.info = "SD IO-Error";
		InternalEvent(ST_ERROR, &ev_data);
	}
	datafile.close();
	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

void BumbleBeeCnt::prepare_sleep() {
	InternalEvent(ST_GOTO_SLEEP, NULL);
//TODO: Hier dem ATTiny 88 über i2c bescheid geben, dass er wieder resetten darf.
	i2c_reg |= sysdefs::res_ctrl::allowreset;
	Wire.begin();
	Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
	Wire.write(i2c_reg);
	Wire.endTransmission(true);
}

void BumbleBeeCnt::goto_sleep() {
#ifdef SERIAL_DEBUG
	Serial.println("State goto_sleep...");
#endif
	ESP.deepSleep(10E6);
//	delay(1000);
//	ESP.restart();
//	InternalEvent(ST_WAKEUP, NULL);
}

void BumbleBeeCnt::error(BumbleBeeCntData *d) {
#ifdef SERIAL_DEBUG
	Serial.print("ERROR --- ");
	Serial.println(d->info);
#endif
	while (1)
		;
}
