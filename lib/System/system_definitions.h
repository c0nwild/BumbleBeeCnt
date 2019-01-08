/*
 * system_definitions.h
 *
 *  Created on: 06.10.2018
 *      Author: cwild
 */

#ifndef SRC_SYSTEM_DEFINITIONS_H_
#define SRC_SYSTEM_DEFINITIONS_H_

#include <Arduino.h>

namespace sysdefs {

namespace res_ctrl {
const uint8_t i2c_addr = 0x47;
const unsigned sys_initialized = (1 << 2);
const unsigned allowreset = (1 << 3);
const unsigned int_src_mcp = (1 << 0);
const unsigned int_src_esp = (1 << 1);
}//res_ctrl

namespace scale {
const float scale_factor = 696.0;
}//scale

namespace mcp {
const uint8_t wlan_en = 0x10;
const uint8_t lb0 = 0x20;
const uint8_t lb1 = 0x40;
const uint8_t tare = 0x80;
}//mcp

namespace wifi {
const String ssid = "ESPap";
const String passphrase = "thereisnospoon";
}//wifi

namespace general {
const String log_filename = "data.txt";
const String error_filename = "error.txt";
}//general

namespace rtc {
const uint8_t rtc_memstart = 65;
const uint8_t rtc_memsize = 127;
const uint8_t rtc_sreg = (rtc_memstart + 0); //2x uint32 needs 8 bytes
const uint8_t rtc_eventcnt = (rtc_memstart + 8); //2x unsinged int (32bit) needs 8 bytes
const uint8_t rtc_errmem = (rtc_eventcnt + 8); //2x unsinged int (32bit) needs 8 bytes
const uint8_t rtc_errcnt = (rtc_errmem + 8);
}//rtc
}//sysdefs


#endif /* SRC_SYSTEM_DEFINITIONS_H_ */
