/*
 * webcontent.cpp
 *
 *  Created on: 16.03.2018
 *      Author: conradwild
 */
#include "webcontent.h"

String _tagH1begin = "<h1>";
String _tagH1end = "</h1>";
String _tagH2begin = "<h2>";
String _tagH2end = "</h2>";
String _heading = "Bumblebee Counter";
String _temp = "Temperature: ";
String _humidity = "Humidity: ";

String create_heading() {
	return _tagH1begin + _heading + _tagH1end;
}

String WebContent::create_temp_entry(){
	String _t_val_s(_t_val);
	return _tagH2begin + _temp + _t_val_s + "deg C" + _tagH2end;
}

String WebContent::create_humid_entry(){
	String _h_val_s(_h_val);
	return _tagH2begin + _humidity + _h_val_s + _tagH2end;
}

String WebContent::output(void) {
	String rv;
	String _t_val_s(_t_val);
	rv = create_heading() + create_temp_entry() + create_humid_entry();
	return rv;
}

void WebContent::setTVal(float t) {
	_t_val = t;
}

void WebContent::setHVal(float h){
	_h_val = h;
}
