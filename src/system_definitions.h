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

}

#endif /* SRC_SYSTEM_DEFINITIONS_H_ */
