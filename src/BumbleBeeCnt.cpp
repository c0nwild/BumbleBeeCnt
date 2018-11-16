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

	//Only init once after power off.
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

	DEBUG_MSG_ARG(DEBUG_ID_MCP23017, HEX)

	pinMode(chipSelectSD, OUTPUT);
	if (!SD.begin(chipSelectSD)) {
		retval |= -DEBUG_ID_SD;
	} else {
		DEBUG_MSG_ARG(DEBUG_ID_SD, HEX)
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
		DEBUG_MSG_ARG(DEBUG_ID_BME280, HEX);

	return retval;
}

void BumbleBeeCnt::st_do_tare() {
	DEBUG_MSG("Tare...")
	float tare_value = 0;

	tare_value = st_weight_meas();

	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

float BumbleBeeCnt::st_weight_meas() {
	DEBUG_MSG("Weight meas...")
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
			DEBUG_MSG("Timeout!")
			break;
		}
	}
	scale.powerDown();
	return rv;
}

void BumbleBeeCnt::eval_peripheral_event(uint8_t mcp_gpioa) {
// data processing here
}

//State function
void BumbleBeeCnt::st_wakeup() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_WAKEUP, HEX)
	/* TODO: Hier den Attiny 88 über I2C abfragen, solange keine Freigabe vomconversion Wemos erfolgt
	 * darf der Tiny keinen Reset durchführen.
	 */
	states next_state = ST_INIT_PERIPHERALS;
	BumbleBeeCntData *d = NULL;

	attiny88.setSlaveAddr(sysdefs::res_ctrl::i2c_addr);

//get sreg from reset controller
//	Wire.begin();
//	Wire.requestFrom(addr, 1);
//	while (Wire.available()) {
//		i2c_reg = Wire.read();
//	}
//	Wire.endTransmission(true);
	i2c_reg = attiny88.getData();

	if (i2c_reg == 0xff) {
		next_state = ST_ERROR;
		d = new BumbleBeeCntData;
		d->info = "invalid data from int ctrl";
	}
#ifdef SERIAL_DEBUG
	String src;
	if (i2c_reg & sysdefs::res_ctrl::int_src_esp)
		src = "ESP";
	else if (i2c_reg & sysdefs::res_ctrl::int_src_mcp)
		src = "MCP";
	else
		src = "undef";

	Serial.print("i2cbuf: ");
	Serial.println(attiny88.dumpBuffer(), BIN);
	Serial.print("I2CREG: 0x");
	Serial.print(i2c_reg, BIN);
	Serial.println();
	Serial.print("SRC: ");
	Serial.print(src);
	Serial.println();
#endif

	InternalEvent(next_state, d);
}

//State function
void BumbleBeeCnt::st_init_peripherals() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_INIT_PERIPHERALS, HEX)

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

//		Wire.begin();
//		Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
//		Wire.write(i2c_reg);
//		Wire.endTransmission(true);
		attiny88.sendData(i2c_reg);
	}
	InternalEvent(next_state, data);
}

//State function
void BumbleBeeCnt::st_read_peripherals() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_READ_PERIPHERALS, HEX)

	BumbleBeeCntData* peripheral_data;
	peripheral_data = new BumbleBeeCntData;

	//Weight measurement after fixed time intervals
	if (i2c_reg & sysdefs::res_ctrl::int_src_esp)
		peripheral_data->weight = st_weight_meas();

	peripheral_data->temperature = bme.temp();
	peripheral_data->humidity = bme.hum();
	peripheral_data->pressure = bme.pres();

	peripheral_data->mcp_gpioab = mcp.readGPIOAB();

	i2c_reg &=
			~(sysdefs::res_ctrl::int_src_esp | sysdefs::res_ctrl::int_src_mcp);
//	Wire.begin();
//	Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
//	Wire.write(i2c_reg);
//	Wire.endTransmission(true);
	attiny88.sendData(i2c_reg);

	InternalEvent(ST_EVAL_PERIPHERAL_DATA, peripheral_data);
}

void BumbleBeeCnt::st_eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG_ARG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA, HEX)

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

void BumbleBeeCnt::st_write_to_sd(BumbleBeeCntData* d) {
	DEBUG_MSG_ARG(DEBUG_ID_ST_WRITE_TO_SD, HEX)
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

void BumbleBeeCnt::st_prepare_sleep() {
	InternalEvent(ST_GOTO_SLEEP, NULL);
//TODO: Hier dem ATTiny 88 über i2c bescheid geben, dass er wieder resetten darf.
	i2c_reg |= sysdefs::res_ctrl::allowreset;
//	Wire.begin();
//	Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
//	Wire.write(i2c_reg);
//	Wire.endTransmission(true);
#ifdef SERIAL_DEBUG
	Serial.print("I2CREG: 0x");
	Serial.print(i2c_reg, BIN);
	Serial.println();
#endif
	attiny88.sendData(i2c_reg);
}

void BumbleBeeCnt::st_goto_sleep() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_GOTO_SLEEP, HEX)
	ESP.deepSleep(10E6);
//	delay(1000);
//	ESP.restart();
//	InternalEvent(ST_WAKEUP, NULL);
}

void BumbleBeeCnt::st_error(BumbleBeeCntData *d) {
#ifdef SERIAL_DEBUG
	Serial.print("ERROR --- ");
	Serial.println(d->info);
#endif
	while (1)
		;
}
