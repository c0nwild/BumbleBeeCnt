#include <Arduino.h>
#include <HX711.h>

HX711::HX711() {
}

HX711::~HX711() {
}

void HX711::begin() {
	Wire.begin();
}

void HX711::set_gain(byte gain) {
}

float HX711::get_weight() {
	send_command(do_measurement);
	delay(500);
	float weight = read_from_hx711();
	return weight;
}

void HX711::send_command(uint8_t cmd) {
	Serial.println("Sending command " + String(cmd));
	Wire.begin();
	Wire.beginTransmission(0x4);
	Wire.write(cmd);
	Wire.endTransmission();
}

float HX711::read_from_hx711() {
	std::vector<uint8_t> i2c_rx_buffer;
	i2c_rx_buffer.reserve(6);

	Wire.begin();
	Wire.setClockStretchLimit(500U);
	Wire.requestFrom(0x4, 6);
	byte rx_byte = 0;
	while (Wire.available()) {
		rx_byte = Wire.read();
		i2c_rx_buffer.push_back(rx_byte);
	}

	hx711::RxFrame rx(i2c_rx_buffer);
	return rx.get_weight();
}

void HX711::tare(byte times) {
	send_command(do_tare);
}

void HX711::calibration() {
	send_command(do_calibration);
}

