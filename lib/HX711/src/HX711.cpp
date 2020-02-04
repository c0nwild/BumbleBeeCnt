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
	Wire.setClockStretchLimit(500U);
	Wire.beginTransmission(sysdefs::hx711::i2c_address);
	Wire.write(cmd);
	Wire.endTransmission();
}

float HX711::read_from_hx711() {
	Serial.println("Read from HX711");
	std::vector<uint8_t> i2c_rx_buffer;
	float rv = -1000;

	Wire.begin();
	Wire.setClockStretchLimit(500U);
	Wire.requestFrom(sysdefs::hx711::i2c_address,
			sysdefs::hx711::data_frame_size);
	byte rx_byte = 0;
	while (Wire.available()) {
		rx_byte = Wire.read();
		i2c_rx_buffer.push_back(rx_byte);
	}

	if (i2c_rx_buffer.size() == sysdefs::hx711::data_frame_size){
		hx711::RxFrame rx(i2c_rx_buffer);
		rv = rx.get_value();
	}

#ifdef SERIAL_DEBUG
	Serial.print("Buffer: ");
	for (auto n: i2c_rx_buffer){
		Serial.print("0x" + String(n) + " ");
	}
	Serial.println();
#endif

	return rv;
}

void HX711::tare() {
	send_command(do_tare);
}

void HX711::calibration() {
	send_command(do_calibration);
}
