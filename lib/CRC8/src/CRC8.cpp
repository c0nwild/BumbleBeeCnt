/*
 * CRC8.cpp
 *
 *  Created on: 15.11.2019
 *      Author: cwild
 */

#include "CRC8.h"

namespace protocol {

CRC8::CRC8(const uint8_t *data,size_t length) {
	// TODO Auto-generated constructor stub
	_crc = crc8(data,length);
}

CRC8::~CRC8() {
	// TODO Auto-generated destructor stub
}

uint8_t CRC8::crc8(const uint8_t *data,size_t length)
{
   uint8_t crc = 0x00;
   uint8_t extract;
   uint8_t sum;
   for(int i=0;i<length;i++)
   {
      extract = *data;
      for (uint8_t tempI = 8; tempI; tempI--)
      {
         sum = (crc ^ extract) & 0x01;
         crc >>= 1;
         if (sum)
            crc ^= 0x8C;
         extract >>= 1;
      }
      data++;
   }
   return crc;
}

uint8_t CRC8::get_crc() {
	return _crc;
}

} /* namespace protocol */
