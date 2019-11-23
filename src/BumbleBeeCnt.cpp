/*
 * BumbleBeeCnt.cpp
 *
 *  Created on: 19.07.2018
 *      Author: conradwild
 */

#include <BumbleBeeCnt.h>
#include <time.h>
#include <stdlib.h>

void BumbleBeeCnt::trigger() {
	ExternalEvent(ST_WAKEUP);
#ifdef SM_CYCLETIME_MEAS
	this->setCycleTime(millis());
#endif
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

	DEBUG_MSG_ARG(DEBUG_ID_MCP23017, HEX);
	DEBUG_MSG_PASS(sysdefs::debug::mcp);

	evc0.init();
#ifdef LB1
	evc1.init();
#endif
	rtc_buf.init();

	return retval;
}

int BumbleBeeCnt::init_peripheral_system() {
	int retval = 0;
	long scale_offset = 0;

	Wire.begin();

	if (bme.begin()) {
		DEBUG_MSG_ARG(DEBUG_ID_BME280, HEX);
		DEBUG_MSG_PASS(sysdefs::debug::bme280);
	} else {

		retval += -DEBUG_ID_BME280;
	}

	EEPROM.begin(128);
	EEPROM.get(8, scale_offset);
	EEPROM.end();

	DEBUG_MSG("Offset: " + String(scale_offset))
	scale.set_offset(scale_offset);

	pinMode(chipSelectSD, OUTPUT);
	if (SD.begin(chipSelectSD)) {
		DEBUG_MSG_PASS(sysdefs::debug::sd)
	} else {
		DEBUG_MSG_FAIL(sysdefs::debug::sd)
		retval += -DEBUG_ID_SD;
	}

	return retval;
}

void BumbleBeeCnt::do_tare() {
	long offset = 0;
	DEBUG_MSG("Tare...")

	//Just trigger weight measurement
	scale.begin(D3,D4);
	scale.power_up();
	scale.tare(5);
	scale.power_down();

	offset = scale.get_offset();

	EEPROM.begin(128);
	EEPROM.put(8, offset);
	EEPROM.commit();
	EEPROM.end();

	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

float BumbleBeeCnt::weight_meas() {
	DEBUG_MSG("Weight meas...")
	float rv = 0.0;
	float calib = 0.0;
	long offset = 0;

	EEPROM.begin(128);
	EEPROM.get(0, calib);
	EEPROM.end();
	if (calib == 0.0) {
		calib = 1.0;
	}

	EEPROM.begin(128);
	EEPROM.get(8, offset);
	EEPROM.end();

	DEBUG_MSG("Calib factor: " + String(calib));

	scale.begin(D3,D4);
	scale.power_up();
	scale.set_offset(offset);
	scale.set_scale(calib);

	rv = scale.get_units();

	scale.power_down();

	if (isnan(rv)) {
		DEBUG_MSG_FAIL(sysdefs::debug::hx711);
	} else {
		DEBUG_MSG("Weight is: " + String(rv));
	}

	return rv;
}

void BumbleBeeCnt::read_sensors(BumbleBeeCntData *s_data) {
	s_data->temperature = bme.temp();
	s_data->humidity = bme.hum();
	s_data->pressure = bme.pres();
	s_data->v_batt = analogRead(A0);
}

void BumbleBeeCnt::read_port_expander(BumbleBeeCntData *p_data) {
	p_data->mcp_gpioab = mcp.readGPIOAB();
}

void BumbleBeeCnt::eval_peripheral_event(uint8_t mcp_gpioa) {
// data processing here

}

void BumbleBeeCnt::prepare_cal() {
	do_cal(1.0);
	do_tare();
	ap.unset_prepare_cal();
}

void BumbleBeeCnt::do_cal(float calib) {
	Serial.print("web_cal_factor: ");
	Serial.println(calib);
	EEPROM.begin(128);
	EEPROM.put(0, calib);
	EEPROM.commit();
	EEPROM.end();
	ap.unset_do_cal();
}

String BumbleBeeCnt::prepare_log_str(Ds1307::DateTime dt, BumbleBeeCntData* d) {
	String date_str = "";
	String log_str = "";

	date_str = String((uint16_t) dt.year + 2000) + "-" + String(dt.month) + "-"
			+ String(dt.day) + "_" + String(dt.hour) + ":" + String(dt.minute)
			+ ":" + String(dt.second);

	log_str = date_str;
	log_str += ",";
#ifdef DIR_SENSE
	log_str += d->ev_cnt_in;
	log_str += ",";
	log_str += d->ev_cnt_out;
	log_str += ",";
#else
	log_str += d->ev_cnt0;
	log_str += ",";
#ifdef LB1
	log_str += d->ev_cnt1;
	log_str += ",";
#endif
#endif//DIR_SENSE
	log_str += d->humidity;
	log_str += ",";
	log_str += d->temperature;
	log_str += ",";
	log_str += d->pressure;
	log_str += ",";
	log_str += d->weight;
	log_str += ",";
	log_str += (float)d->v_batt / 1024 * 5;
	log_str += ",";
	log_str += ts;

	return log_str;
}

//State function
void BumbleBeeCnt::st_wakeup() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_WAKEUP, HEX);

	states next_state = ST_INIT_PERIPHERALS;
	BumbleBeeCntData *d = NULL;

	irqctl.setSlaveAddr(sysdefs::res_ctrl::i2c_addr);

//get sreg from reset controller
	i2c_reg = irqctl.getData();

	if (i2c_reg == 0xff) {
		next_state = ST_ERROR;
		d = new BumbleBeeCntData;
		d->info = "invalid data from int ctrl";
	}

#ifdef SERIAL_DEBUG_INT_CNTR
	String src;
	if (i2c_reg & sysdefs::res_ctrl::int_src_esp)
	src = "ESP";
	else if (i2c_reg & sysdefs::res_ctrl::int_src_mcp)
	src = "MCP";
	else
	src = "undef";

	Serial.print("i2cbuf: ");
	Serial.println(irqctl.dumpBuffer(), BIN);
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
	BumbleBeeCntData *data = NULL;

	if (!(i2c_reg & sysdefs::res_ctrl::sys_initialized)) {
		(void) init_peripheral_system_once();
	}

	retval = init_peripheral_system();

	if (retval < 0) {
		next_state = ST_ERROR;
		data = new BumbleBeeCntData;
		data->info = "init_peripherals failed with code " + String(retval);
	} else {
		next_state = ST_READ_PERIPHERALS;
		data = NULL;

		i2c_reg |= sysdefs::res_ctrl::sys_initialized;

		irqctl.sendData(i2c_reg);
	}
	InternalEvent(next_state, data);
}

//State function
void BumbleBeeCnt::st_wifi_init() {
	float weight;
	states next_state = ST_WIFI;

	String ssid = "";
	String passwd = "";
	String config_str = "{ \"ssid\",\"error\"}";
	File config_file;

	WiFiConfig cfg;

	const size_t capacity = 128;
	DynamicJsonBuffer jsonBuffer(capacity);

	config_file = SD.open(config_file_name, FILE_READ);

	if (config_file) {
		config_str = config_file.readString();
		config_str.trim();
	} else {
		DEBUG_MSG("Error opening " + config_file_name);
	}

	JsonObject &configRoot = jsonBuffer.parseObject(config_str);

	if (!configRoot.success()) {
		DEBUG_MSG("Error reading CONFIG!");
		next_state = ST_WIFI_END;
	} else {
		DEBUG_MSG("Success reading CONFIG!");
		String ssid = configRoot["ssid"];
		String pw = configRoot["passwd"];
		ap.setSSID(ssid);
		ap.setPasswd(pw);
		ap.initWifi();

		// Wir wollen eine initiale Gewichtsmessung für die Sensoranzeige.
		weight = weight_meas();
		ap.setWeight(weight);
	}

	config_file.close();

	InternalEvent(next_state, NULL);
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
	if (ap.get_need_weight()) {
		weight = weight_meas();
		ap.unset_need_weight();
	}

	if (ap.get_prepare_cal()){
		prepare_cal();
	}

	if(ap.get_do_cal()){
		float weight;
		weight = weight_meas();
		do_cal(weight / sysdefs::hx711::hx711_cal_weight);
	}

	str_time = ap.getTimeString();
	str_scale_calib = ap.getScaleCalibString();
	str_scale_calib.toLowerCase();

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
		str_scale_calib = str_scale_calib.substring(3,
				str_scale_calib.length());
		calib = str_scale_calib.toFloat();
		do_cal(calib);
		weight = weight_meas();
	} else if (str_scale_calib == "tare") {
		do_tare();
	}

	ds1307.getDateTime(&dt);
	ap.setDateTime(dt);

	read_sensors(&p_data);
	read_port_expander(&p_data);

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
	DEBUG_MSG("Stopping wifi...");
	ap.stopWifi();

	InternalEvent(ST_READ_PERIPHERALS, NULL);
}

//State function reads sensor data and stores timestamp in rtc ram
void BumbleBeeCnt::st_read_peripherals() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_READ_PERIPHERALS, HEX)
	states next_state = ST_EVAL_PERIPHERAL_DATA;

	long last_ts = 0;
	uint16_t ts_diff = 0;
	uint16_t last_ts_diff = 0;

	BumbleBeeCntData* data;
	data = new BumbleBeeCntData;

	BumbleBeeRamData ram_data;

	ram_data = rtc_buf.getBuffer();
	last_ts = ram_data.ts;
	last_ts_diff = ram_data.ts_diff;

	ds1307.getDateTime(&dt);
	ts = ds1307.getTimestamp();

	ts_diff = (uint16_t) (ts - last_ts);

	DEBUG_MSG("ts diff: " + String(ts_diff));
	DEBUG_MSG("delta ts_diff: " + String(ts_diff - last_ts_diff));

	if ((ts_diff - last_ts_diff) >= sysdefs::general::event_sum_interval) {
		data->do_log_entry = true;
		ram_data.ts_diff = ts_diff;
		rtc_buf.setBuffer(&ram_data);
	}

	if (ts_diff >= sysdefs::general::log_sensor_interval) {
		data->weight = weight_meas();
		read_sensors(data);
		data->new_data = true;
		data->do_log_entry = true;
		ram_data.ts = ts;
		ram_data.ts_diff = 0;
		ts_diff = 0;
		rtc_buf.setBuffer(&ram_data);
	}

	read_port_expander(data);

	reset_cntdown = (sysdefs::general::log_sensor_interval - ts_diff);

	if (data->mcp_gpioab & sysdefs::mcp::tare) {
		next_state = ST_TARE;
		data = NULL;
	}

	InternalEvent(next_state, data);
}

/*State function analyzes sensor data, stores data to ram and
 * prepares string for log entry.
 * Two different implementations depending on presence of second light barrier
 * and directory sensing.
 */
#ifndef DIR_SENSE
void BumbleBeeCnt::st_eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG_ARG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA, HEX);

	states next_state = ST_PREPARE_SLEEP;

	String log_str = "";

	BumbleBeeCntData *d_out = NULL;

	BumbleBeeRamData ram_data;

	bool lb0_rising_edge = false;
#ifdef LB1
	bool lb1_rising_edge = false;
#endif

	ram_data = rtc_buf.getBuffer();

	p_data->lb0 = (p_data->mcp_gpioab & sysdefs::mcp::lb0) ? 1 : 0;
#ifdef LB1
	p_data->lb1 = (p_data->mcp_gpioab & sysdefs::mcp::lb1) ? 1 : 0;
#endif
	p_data->wlan_en = (p_data->mcp_gpioab & sysdefs::mcp::wlan_en) ? 1 : 0;
	p_data->tare = (p_data->mcp_gpioab & sysdefs::mcp::tare) ? 1 : 0;

	/*If sensor data are not new, use those values stored in ram.
	 * Otherwise update ram values.
	 */
	if (!p_data->new_data) {
		p_data->humidity = ram_data.humidity;
		p_data->pressure = ram_data.pressure;
		p_data->temperature = ram_data.temperature;
		p_data->v_batt = ram_data.v_batt;
		p_data->weight = ram_data.weight;
	} else {
		ram_data.humidity = p_data->humidity;
		ram_data.pressure = p_data->pressure;
		ram_data.temperature = p_data->temperature;
		ram_data.v_batt = p_data->v_batt;
		ram_data.weight = p_data->weight;
	}

	//Edge detection on lightbarriers and counter
	lb0_rising_edge = ((ram_data.lb0 == 0) && (p_data->lb0 == 1));
#ifdef LB1
	lb1_rising_edge = ((ram_data.lb1 == 0) && (p_data->lb1 == 1));
#endif

	ram_data.lb0 = p_data->lb0;
#ifdef LB1
	ram_data.lb1 = p_data->lb1;
#endif

	p_data->ev_cnt0 = evc0.get_cnt();
	if (p_data->ev_cnt0 < 0) {
		evc0.init();
		DEBUG_MSG("ev_cnt0 access error");
	}
	if (lb0_rising_edge) {
		evc0.inc();
		++p_data->ev_cnt0;
		DEBUG_MSG("ev_cnt0: " + String(p_data->ev_cnt0));
	}
#ifdef LB1
	p_data->ev_cnt1 = evc1.get_cnt();
	if (p_data->ev_cnt1 < 0) {
		evc1.init();
		DEBUG_MSG("ev_cnt1 access error");
	}
	if (lb1_rising_edge) {
		evc1.inc();
		++p_data->ev_cnt1;
		DEBUG_MSG("ev_cnt1: " + String(p_data->ev_cnt1));
	}
#endif
	rtc_buf.setBuffer(&ram_data);

	if (p_data->wlan_en) {
		next_state = ST_WIFI_INIT;
	} else {
		if (p_data->do_log_entry) {
			d_out = new BumbleBeeCntData;

			*d_out = *p_data;
			log_str = prepare_log_str(dt, p_data);
#ifdef SERIAL_DEBUG
			Serial.println(log_str);
#endif
			d_out->info = log_str;

			evc0.init();
#ifdef LB1
			evc1.init();
#endif

			next_state = ST_WRITE_TO_SD;
		}
	}

	delay(1);

	InternalEvent(next_state, d_out);
}
#else
void BumbleBeeCnt::st_eval_peripheral_data(BumbleBeeCntData* p_data) {
	DEBUG_MSG_ARG(DEBUG_ID_ST_EVAL_PERIPHERAL_DATA, HEX);

	states next_state = ST_PREPARE_SLEEP;
	String log_str = "";
	BumbleBeeCntData *d_out = NULL;
	BumbleBeeRamData ram_data;
	uint8_t cycle_counter = 1;
	event_eval st_eval = cleanup;
	bool lb0_rising_edge = false;
	bool lb1_rising_edge = false;

	ram_data = rtc_buf.getBuffer();

	p_data->lb0 = (p_data->mcp_gpioab & sysdefs::mcp::lb0) ? 1 : 0;
	p_data->lb1 = (p_data->mcp_gpioab & sysdefs::mcp::lb1) ? 1 : 0;
	p_data->wlan_en = (p_data->mcp_gpioab & sysdefs::mcp::wlan_en) ? 1 : 0;
	p_data->tare = (p_data->mcp_gpioab & sysdefs::mcp::tare) ? 1 : 0;

	p_data->ev_cnt_in = evc0.get_cnt();
	p_data->ev_cnt_out = evc1.get_cnt();

	/*If sensor data are not new, use those values stored in ram.
	 * Otherwise update ram values.
	 */
	if (!p_data->new_data) {
		p_data->humidity = ram_data.humidity;
		p_data->pressure = ram_data.pressure;
		p_data->temperature = ram_data.temperature;
		p_data->v_batt = ram_data.v_batt;
		p_data->weight = ram_data.weight;
	} else {
		ram_data.humidity = p_data->humidity;
		ram_data.pressure = p_data->pressure;
		ram_data.temperature = p_data->temperature;
		ram_data.v_batt = p_data->v_batt;
		ram_data.weight = p_data->weight;
	}

	//Edge detection on lightbarriers and counter
	lb0_rising_edge = ((ram_data.lb0 == 0) && (p_data->lb0 == 1));
	DEBUG_MSG("ram_data.lb0: " + String(ram_data.lb0));
	DEBUG_MSG("p_data->lb0: " + String(p_data->lb0));
	ram_data.lb0 = p_data->lb0;
	if (lb0_rising_edge) {
		st_eval = new_edge_lb0;
	}

	lb1_rising_edge = ((ram_data.lb1 == 0) && (p_data->lb1 == 1));
	ram_data.lb1 = p_data->lb1;
	if (lb1_rising_edge) {
		st_eval = new_edge_lb1;
	}
	if (i2c_reg & sysdefs::res_ctrl::int_src_esp) {
		st_eval = cleanup;
		DEBUG_MSG("Event timeout");
	}

	//The main sub-statemachine for event evaluation.
	while (cycle_counter--) {
		switch (st_eval) {
			case new_edge_lb0:
			DEBUG_MSG("new edge lb0")
			;
			//Second edge on the same LB -> discard event
			if (ram_data.edge_lb0 == true) {
				++cycle_counter;
				st_eval = cleanup;
				break;
			}
			if (ram_data.dir == 0) {
				ram_data.dir = sysdefs::general::dir_out;
				ram_data.edge_lb0 = true;
				reset_cntdown = sysdefs::general::event_timeout;
			} else {
				++cycle_counter;
				st_eval = count_event_out;
			}
			break;
			case new_edge_lb1:
			DEBUG_MSG("new edge lb1")
			;
			//Second edge on the same LB -> discard event
			if (ram_data.edge_lb1 == true) {
				++cycle_counter;
				st_eval = cleanup;
				break;
			}
			if (ram_data.dir == 0) {
				ram_data.dir = sysdefs::general::dir_in;
				ram_data.edge_lb1 = true;
				reset_cntdown = sysdefs::general::event_timeout;
			} else {
				++cycle_counter;
				st_eval = count_event_in;
			}
			break;
			case count_event_in:
			DEBUG_MSG("count event in")
			;
			if (p_data->ev_cnt_in < 0) {
				evc0.init();
			}
			evc0.inc();
			++p_data->ev_cnt_in;
			DEBUG_MSG("ev_cnt_in: " + String(p_data->ev_cnt_in))
			;
			st_eval = cleanup;
			++cycle_counter;
			break;
			case count_event_out:
			DEBUG_MSG("count event out")
			;
			if (p_data->ev_cnt_out < 0) {
				evc1.init();
			}
			evc1.inc();
			++p_data->ev_cnt_out;
			DEBUG_MSG("ev_cnt_out: " + String(p_data->ev_cnt_out))
			;
			st_eval = cleanup;
			++cycle_counter;
			break;
			case cleanup:
			DEBUG_MSG("cleanup")
			;
			ram_data.edge_lb0 = false;
			ram_data.edge_lb1 = false;
			ram_data.dir = 0;
			cycle_counter = 0;
			break;
			case idle:
			DEBUG_MSG("idle")
			;
			if (ram_data.edge_lb0 || ram_data.edge_lb1) {
				reset_cntdown = sysdefs::general::event_timeout;
			}
			break;
			default:
			DEBUG_MSG("Eval SM invalid state.")
			;
		}
	}

	p_data->dir = ram_data.dir;

	rtc_buf.setBuffer(&ram_data);

	// Reset the interrupt src information.
	i2c_reg &=
	~(sysdefs::res_ctrl::int_src_esp | sysdefs::res_ctrl::int_src_mcp);

	irqctl.sendData(i2c_reg);

	if (p_data->wlan_en) {
		next_state = ST_WIFI_INIT;
	} else {
		if (p_data->do_log_entry) {
			d_out = new BumbleBeeCntData;

			*d_out = *p_data;
			Serial.println(p_data->ev_cnt_in, BIN);
			log_str = prepare_log_str(dt, p_data);
#ifdef SERIAL_DEBUG
			Serial.println(log_str);
#endif
			d_out->info = log_str;

			evc0.init();
			evc1.init();

			next_state = ST_WRITE_TO_SD;
		}
	}

	delay(1);

	InternalEvent(next_state, d_out);
}
#endif//DIR_SENSE

void BumbleBeeCnt::st_tare() {
	BumbleBeeCntData data;

	//Todo: Implement timeout counter. This is not elegant but works for the moment.
	delay(1000);

	read_port_expander(&data);

	if (data.mcp_gpioab & sysdefs::mcp::tare) {
		do_tare();
	}

	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

void BumbleBeeCnt::st_write_to_sd(BumbleBeeCntData* d) {
	DEBUG_MSG_ARG(DEBUG_ID_ST_WRITE_TO_SD, HEX);
	File datafile;
	irqctl.sendData(i2c_reg);
	String logstring;
	BumbleBeeCntData *ev_data;

	datafile = SD.open(data_file_name, FILE_WRITE);

	if (datafile) {
		datafile.println(d->info);
	} else {
		ev_data = new BumbleBeeCntData;
		ev_data->info = "SD IO-Error";
		InternalEvent(ST_ERROR, ev_data);
	}
	datafile.close();
	InternalEvent(ST_PREPARE_SLEEP, NULL);
}

void BumbleBeeCnt::st_prepare_sleep() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_PREPARE_SLEEP, HEX)
	InternalEvent(ST_GOTO_SLEEP, NULL);
	i2c_reg |= sysdefs::res_ctrl::allowreset;
//	Wire.begin();
//	Wire.beginTransmission(sysdefs::res_ctrl::i2c_addr);
//	Wire.write(i2c_reg);
//	Wire.endTransmission(true);
#ifdef SERIAL_DEBUG_INT_CNTR
	Serial.print("I2CREG: 0x");
	Serial.print(i2c_reg, BIN);
	Serial.println();
#endif
	//Hier den MCP zurücksetzen, falls während St.M. durchlaufs ein INT angefallen ist.
	mcp.readGPIOAB();
	irqctl.sendData(i2c_reg);
}

void BumbleBeeCnt::st_goto_sleep() {
	DEBUG_MSG_ARG(DEBUG_ID_ST_GOTO_SLEEP, HEX)
#ifdef SM_CYCLETIME_MEAS
	unsigned long cycleTime;
	cycleTime = millis() - this->getCycleTime();
	DEBUG_MSG("cycle: " + String(cycleTime));
#endif
	DEBUG_MSG("reset_cntdown: " + String((uint32_t )reset_cntdown));
	ESP.deepSleep(reset_cntdown * 1e6);
}

void BumbleBeeCnt::st_error(BumbleBeeCntData *d) {
	static int err_cnt = 0;
	states next_state = ST_ERROR;
	BumbleBeeCntData *d_st;

#ifdef SERIAL_DEBUG
	Serial.print("ERROR --- ");
	Serial.println(d->info);
#endif

	++err_cnt;
	DEBUG_MSG("err_cnt: " + String(err_cnt));
	if (err_cnt > 5) {
		d_st = new BumbleBeeCntData;
		d_st->info = d->info;
		next_state = ST_FATAL_ERROR;
	} else {
		delay(5000);
		next_state = ST_WAKEUP;
		d_st = NULL;
	}
	InternalEvent(next_state, d_st);
}

void BumbleBeeCnt::st_fatal_error(BumbleBeeCntData *d) {
	File errorfile;
	long ts;
	ts = ds1307.getTimestamp();

	errorfile = SD.open(error_file_name, FILE_WRITE);
	errorfile.println("FATAL ERROR - " + d->info + "," + String(ts));
	errorfile.close();

	while (1) {
		yield();
	}
}
