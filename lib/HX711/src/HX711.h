#ifndef HX711_h
#define HX711_h

#include <Arduino.h>
#include <RxFrame.h>
#include <Wire.h>
#include <memory>
#include <vector>

class HX711 {
private:
	volatile bool is_event = false;

	enum i2c_cmd : uint8_t {
		do_measurement = 1, do_tare, do_calibration
	};

	enum header_codes {
		data_fresh = 0, data_stale
	};

	void send_command(uint8_t cmd);
	float read_from_hx711();

public:
	HX711();

	virtual ~HX711();

	// Allows to set the pins and gain later than in the constructor
	void begin();

	// set the gain factor; takes effect only after a call to read()
	// channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
	// depending on the parameter, the channel is also set to either A or B
	void set_gain(byte gain = 128);

	// returns (read_average() - OFFSET), that is the current value without the tare weight; times = how many readings to do
	float get_weight();

	// set the OFFSET value for tare weight; times = how many times to read the tare value
	void tare();

	void calibration();
};

#endif /* HX711_h */
