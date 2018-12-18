/*
 * BumbleBeeCnt.cpp
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#include <BumbleBeeCnt.h>
#include <locale>
#include <sstream>

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
	mcp.pinMode(4, INPUT);
	mcp.pullUp(4, HIGH);
	for (int n = 0; n < 16; n++)
		mcp.setupInterruptPin(n, CHANGE);
	mcp.setupInterrupts(true, true, LOW);

	DEBUG_MSG_ARG(DEBUG_ID_MCP23017, HEX)

	return retval;
}

int BumbleBeeCnt::init_peripheral_system() {
	int retval = 0;
	double scale_offset = 0;

	Wire.begin();

	if (bme.begin()) {
		DEBUG_MSG_ARG(DEBUG_ID_BME280, HEX)
	} else {
		retval = -DEBUG_ID_BME280;
	}

	EEPROM.begin(128);
	EEPROM.get(4, scale_offset);
	EEPROM.end();

	scale.set_offset(scale_offset);

	pinMode(chipSelectSD, OUTPUT);
	if (SD.begin(chipSelectSD)) {
		DEBUG_MSG_ARG(DEBUG_ID_SD, HEX)
	} else {
		retval = -DEBUG_ID_SD;
	}

	return retval;
}

void BumbleBeeCnt::do_tare() {
	double offset = 0;
	DEBUG_MSG("Tare...")

	scale.tare();
	offset=scale.get_offset();

	EEPROM.begin(128);
	EEPROM.write(4, offset);
	EEPROM.commit();
	EEPROM.end();

	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

float BumbleBeeCnt::weight_meas() {
	DEBUG_MSG("Weight meas...")
	float rv = 0.0;
	float calib = 0.0;

	scale.begin(D3, D4);
//	scale.start(2000);
	EEPROM.begin(128);
	EEPROM.get(0, calib);
	EEPROM.end();
	if (calib == 0.0) {
		calib = 1.0;
	}
	DEBUG_MSG("Calib factor: " + String(calib));

	scale.set_scale(calib);

	rv = scale.get_units();

	scale.power_down();
	return rv;
}

void BumbleBeeCnt::read_peripheral_data(BumbleBeeCntData *p_data) {
	p_data->temperature = bme.temp();
	p_data->humidity = bme.hum();
	p_data->pressure = bme.pres();

	p_data->mcp_gpioab = mcp.readGPIOAB();
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
	Serial.print("Cal factor: ");
	float calib = 0.0;
	EEPROM.get(0, calib);
	Serial.println(calib);
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

		attiny88.sendData(i2c_reg);
	}
	InternalEvent(next_state, data);
}

//State function
void BumbleBeeCnt::st_wifi_init() {
	float weight;
	weight = weight_meas();
	DEBUG_MSG("Weight: " + String(weight));

	ap.initWifi();

	// Wir wollen eine initiale Gewichtsmessung für die Sensoranzeige.
	ap.setWeight(weight);
	InternalEvent(ST_WIFI, NULL);
}

//State function
void BumbleBeeCnt::st_wifi(BumbleBeeCntData *d) {
	BumbleBeeCntData p_data;

	states next_state = ST_WIFI;
	String str_time = "";
	String str_hour = "";
	String str_min = "";
	String str_day = "";
	String str_mon = "";
	String str_year = "";
	Ds1307::DateTime dt = { 0 };

	String str_scale_calib = "";
	String webc_temp = "";
	String current_page = "";

	float weight;
	weight = ap.getWeight();

	str_time = ap.getTimeString();
	str_scale_calib = ap.getScaleCalibString();

	if (str_time != "") {
		Ds1307::DateTime init_date;
		ds1307.getDateTime(&init_date);

		str_hour = str_time.substring(0, 2);
		str_min = str_time.substring(2, 4);
		str_day = str_time.substring(4, 6);
		str_mon = str_time.substring(6, 8);
		str_year = str_time.substring(8, 10);

		dt.hour = (uint8_t) str_hour.toInt();
		dt.minute = (uint8_t) str_min.toInt();
		dt.day = (uint8_t) str_day.toInt();
		dt.month = (uint8_t) str_mon.toInt();
		dt.year = (uint8_t) str_year.toInt();

		DEBUG_MSG("hour: " + str_hour);
		DEBUG_MSG("min: " + str_min);
		DEBUG_MSG("day: " + str_day);
		DEBUG_MSG("mon: " + str_mon);
		DEBUG_MSG("year: " + str_year);

		ds1307.setDateTime(&dt);
	}
	if (str_scale_calib.startsWith("cal")) {
		float calib;
		calib = str_scale_calib.toFloat();
		Serial.print("web_cal_factor: ");
		Serial.println(calib);
		EEPROM.begin(128);
		EEPROM.put(0, calib);
		EEPROM.commit();
		EEPROM.end();
		weight = weight_meas();
	} else if (str_scale_calib == "tare") {
		do_tare();
	}

	read_peripheral_data(&p_data);

	ap.setPeripheralData(p_data);
	ap.setWeight(weight);

	ap.handleClient();

	if (!(p_data.mcp_gpioab & sysdefs::mcp::wlan_en)) {
		next_state = ST_WIFI_END;
	}

	InternalEvent(next_state, NULL);
}

//State function
void BumbleBeeCnt::st_wifi_end() {
	ap.stopWifi();
	InternalEvent(ST_READ_PERIPHERALS, NULL);
}

//State function
void BumbleBeeCnt::st_read_peripherals() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_READ_PERIPHERALS, HEX)
	states next_state = ST_EVAL_PERIPHERAL_DATA;

	BumbleBeeCntData* peripheral_data;
	peripheral_data = new BumbleBeeCntData;

	read_peripheral_data(peripheral_data);

	if (i2c_reg & sysdefs::res_ctrl::int_src_esp) {
		peripheral_data->weight = weight_meas();
	}

	i2c_reg &=
			~(sysdefs::res_ctrl::int_src_esp | sysdefs::res_ctrl::int_src_mcp);

	attiny88.sendData(i2c_reg);

	if (peripheral_data->mcp_gpioab & sysdefs::mcp::tare) {
		next_state = ST_TARE;
		peripheral_data = NULL;
	}

	InternalEvent(next_state, peripheral_data);
}

//State function
void BumbleBeeCnt::st_eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG_ARG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA, HEX);

	String date;
	BumbleBeeCntData *d_out;

	p_data->lb0 = (p_data->mcp_gpioab & sysdefs::mcp::lb0) ? 1 : 0;
	p_data->lb1 = (p_data->mcp_gpioab & sysdefs::mcp::lb1) ? 1 : 0;
	p_data->wlan_en = (p_data->mcp_gpioab & sysdefs::mcp::wlan_en) ? 1 : 0;
	p_data->tare = (p_data->mcp_gpioab & sysdefs::mcp::tare) ? 1 : 0;

	d_out = new BumbleBeeCntData;

	Ds1307::DateTime dt;

#ifdef SERIAL_DEBUG
	Serial.print("GPIOAB: 0b");
	Serial.println(p_data->mcp_gpioab, BIN);
#endif

	ds1307.getDateTime(&dt);

	date = String((uint16_t) dt.year + 2000) + "-" + String(dt.month) + "-"
			+ String(dt.day) + "_" + String(dt.hour) + ":" + String(dt.minute)
			+ ":" + String(dt.second);
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
//	delete d_out;
//	eval_peripheral_event(p_data->mcp_gpioa);

//	InternalEvent(ST_WRITE_TO_SD, d_out); //string, den wir schreiben wollen konstruieren wir hier und übergeben ihn als event data.
	if (p_data->wlan_en) {
		InternalEvent(ST_WIFI_INIT, NULL);
	} else {
		InternalEvent(ST_WRITE_TO_SD, d_out);
	}

}

void BumbleBeeCnt::st_tare(){
	do_tare();
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
	//Hier den MCP zurücksetzen, falls während St.M. durchlaufs ein INT angefallen ist.
	mcp.readGPIOAB();
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
