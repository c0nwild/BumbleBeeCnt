/** Ds1307.cpp
 *
 * Ds1307 class.
 *
 * @version 1.0.1
 * @author Rafa Couto <caligari@treboada.net>
 * @license GNU Affero General Public License v3.0
 * @see https://github.com/Treboada/Ds1307
 *
 */

#include "../Ds1307/Ds1307.h"

#ifndef UNIT_TEST
#include <Arduino.h>
#include <Wire.h>
#define _PROGMEM_ PROGMEM
#else
#define _PROGMEM_
#endif

const uint8_t Ds1307::daysInMonth[] _PROGMEM_ = { 31, 28, 31, 30, 31, 30, 31, 31,
		30, 31, 30, 31 };

Ds1307::Ds1307(uint8_t i2c_address) {
	_i2c_address = i2c_address;
}

void Ds1307::init() {
	Wire.begin();
}

// number of days since 2000/01/01, valid for 2001..2099
uint16_t Ds1307::date2days(uint16_t y, uint8_t m, uint8_t d) {
	if (y >= 2000)
		y -= 2000;
	uint16_t days = d;
	for (uint8_t i = 1; i < m; ++i)
		days += pgm_read_byte(daysInMonth + i - 1);
	if (m > 2 && y % 4 == 0)
		++days;
	return days + 365 * y + (y + 3) / 4 - 1;
}

uint32_t Ds1307::time2int32(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
	return ((days * 24UL + h) * 60 + m) * 60 + s;
}

bool Ds1307::isHalted() {
	uint8_t buffer[1];
	buffer[0] = 0x00;
	_writeBytes(buffer, 1);
	_readBytes(buffer, 1);
	return (buffer[0] & 0b10000000);
}

void Ds1307::getDateTime(DateTime* dt) {
	uint8_t buffer[7];
	buffer[0] = 0x00;
	_writeBytes(buffer, 1);
	_readBytes(buffer, 7);
	dt->second = _bcd2dec(buffer[0] & 0b01111111);
	dt->minute = _bcd2dec(buffer[1] & 0b01111111);
	dt->hour = _bcd2dec(buffer[2] & 0b00111111);
	dt->dow = _bcd2dec(buffer[3] & 0b00000111);
	dt->day = _bcd2dec(buffer[4] & 0b00111111);
	dt->month = _bcd2dec(buffer[5] & 0b00011111);
	dt->year = _bcd2dec(buffer[6] & 0b01111111);
}

void Ds1307::setDateTime(DateTime* dt) {
	uint8_t buffer[8];
	buffer[0] = 0x00;
	buffer[1] = _dec2bcd(dt->second % 60);
	buffer[2] = _dec2bcd(dt->minute % 60);
	buffer[3] = _dec2bcd(dt->hour % 24);
	buffer[4] = _dec2bcd(dt->dow % 8);
	buffer[5] = _dec2bcd(dt->day % 32);
	buffer[6] = _dec2bcd(dt->month % 13);
	buffer[7] = _dec2bcd(dt->year % 100);
	_writeBytes(buffer, 8);
}

void Ds1307::halt() {
	uint8_t buffer[2];
	buffer[0] = 0x00;
	buffer[1] = 0b10000000;
	_writeBytes(buffer, 2);
}

void Ds1307::_readBytes(uint8_t* buffer, uint8_t count) {
	Wire.requestFrom(_i2c_address, count);
	while (count-- > 0) {
		*buffer++ = Wire.read();
	}
	delay(1);
}

void Ds1307::_writeBytes(uint8_t* buffer, uint8_t count) {
	Wire.beginTransmission(_i2c_address);
	while (count-- > 0) {
		Wire.write(*buffer++);
	}
	Wire.endTransmission();
	delay(1);
}

uint8_t Ds1307::_dec2bcd(uint8_t dec) {
	return ((dec / 10 * 16) + (dec % 10));
}

uint32_t Ds1307::getTimestamp() {
	DateTime dt;
	uint16_t days;
	uint32_t ts;

	getDateTime(&dt);
	days = date2days(dt.year, dt.month, dt.day);
	ts = time2int32(days, dt.hour, dt.minute, dt.second);
	return ts;
}

uint8_t Ds1307::_bcd2dec(uint8_t bcd) {
	return ((bcd / 16 * 10) + (bcd % 16));
}

