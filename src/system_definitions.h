/*
 * system_definitions.h
 *
 *  Created on: 06.10.2018
 *      Author: cwild
 */

#ifndef SRC_SYSTEM_DEFINITIONS_H_
#define SRC_SYSTEM_DEFINITIONS_H_

namespace sysdefs {

	namespace res_ctrl {
		const uint8_t	i2c_addr = 0x47;
		const unsigned 	sys_initialized = (1 << 2);
		const unsigned 	allowreset = (1 << 3);
		const unsigned 	int_src_mcp = (1 << 0);
		const unsigned 	int_src_esp = (1 << 1);
	}

	namespace scale {
		const float 	scale_factor = 696.0;
	}

	namespace mcp{
		const uint8_t	wlan_en = 0x10;
		const uint8_t	lb0 = 0x20;
		const uint8_t	lb1 = 0x40;
		const uint8_t	tare = 0x80;
	}

	namespace wifi{
		const String ssid = "ESPap";
		const String passphrase = "thereisnospoon";
	}
}

#endif /* SRC_SYSTEM_DEFINITIONS_H_ */
