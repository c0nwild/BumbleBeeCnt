/*
 * system_definitions.h
 *
 *  Created on: 06.10.2018
 *      Author: cwild
 */

#ifndef SRC_SYSTEM_DEFINITIONS_H_
#define SRC_SYSTEM_DEFINITIONS_H_

#include <Arduino.h>
#include <IPAddress.h>

namespace sysdefs {

namespace pin_mapping {
const unsigned chip_select_sd = D8;
const unsigned pwr_mgmnt_trigger = D3;
} //pin_mapping

namespace res_ctrl {
const uint8_t i2c_addr = 0x47;
const unsigned sys_initialized = (1 << 2);
const unsigned allowreset = (1 << 3);
const unsigned int_src_mcp = (1 << 0);
const unsigned int_src_esp = (1 << 1);
} //res_ctrl

namespace mcp {
const uint8_t wlan_en = 0x10;
const uint8_t lb0 = 0x20;
const uint8_t lb1 = 0x40;
const uint8_t tare = 0x80;
const uint8_t wlan_en_pin = 4;
const uint8_t lb0_pin = 5;
const uint8_t lb1_pin = 6;
const uint8_t tare_pin = 7;

const uint8_t pwr_mgmnt_trigger = 0;
} //mcp

namespace wifi {
const String ssid = "ESPap";
const String passphrase = "thereisnospoon";
const IPAddress ap_ip(192,168,4,1);
} //wifi

namespace general {
const String log_filename = "data.txt";
const String error_filename = "error.txt";
const String config_filename = "CONFIG";
// TODO: Change log_sensor_interval back to 600 sec
const unsigned log_sensor_interval = 5; // in sec.
const uint16_t event_sum_interval = 60; // in sec.
const unsigned event_timeout = 5; // in sec.
const int dir_in = 1;
const int dir_out = -1;
} //general

namespace rtc {
const uint8_t rtc_memstart = 65;
const uint8_t rtc_memsize = 127;
const uint8_t rtc_sreg = (rtc_memstart + 0); //2x uint32 needs 8 bytes
const uint8_t rtc_errmem = (rtc_memstart + 8); //2x unsinged int (32bit) needs 8 bytes
const uint8_t rtc_eventcnt0 = (rtc_memstart + 16); //2x unsinged int (32bit) needs 8 bytes
const uint8_t rtc_eventcnt1 = (rtc_memstart + 24); //2x unsinged int (32bit) needs 8 bytes
const uint8_t rtc_bmbcnt_data = (rtc_memstart + 32); // BumbleBeeRamData 31 bytes
} //rtc

namespace hx711 {
const float hx711_cal_weight = 1000.0;
const uint8_t i2c_address = 0x4;
const uint8_t data_frame_size = 6; //1 byte header + 4 byte data + 1 byte crc
} //hx711

namespace debug {
const String bme280 = "bme280";
const String sd = "sd";
const String mcp = "mcp 23017";
const String hx711 = "hx711";
} //debug

namespace pwr_mgmnt {
//TODO: Change batt_thres_volt back to 512
const int batt_thresh_volt = 800; //in digits == 2,5V
} //pwr_mgmnt

} //sysdefs

#endif /* SRC_SYSTEM_DEFINITIONS_H_ */
