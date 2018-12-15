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
String _temp = "Temperature: ";
String _humidity = "Humidity: ";
String _weight = "Weight: ";

char WebContent::webpage_body_main[] =
		R"=====( 	
	Willkommen auf <b>BumbleBee Logger</b>.
	<div>
		<form action="/download">
			<button>Get data</button>
		</form>
	</div>
)=====";

char WebContent::webpage_settings[] =
		R"=====( 	
<!DOCTYPE html>				
<html>
	<head>
		<meta charset='utf-8'>
		<title>Biozentrum Zoologie</title>
		<link rel="stylesheet" href="css/style.css" type="text/css" />
	</head>
	<body>
		<div class='page'>
			<div class='header'>
				<a href='/' id='logo'><img src='images/logo.jpg' alt=''/></a>
			</div>
			<div class='body'>
				<p>
					Settings
				</p>
				<div>
					<form action=''>
						<input name='rtcset' placeholder='HHMMDDMMYY'>
					</form>
				</div>
			</div>
			<div class='footer'>
				<ul>
					<li><a href='/'>Home</a></li>
					<li><a href='/settings'>Settings</a></li>
					<li><a href='/sensors'>Sensors</a></li>
					<li><a href='/scale_calib'>Scale calibration</a></li>
				</ul>
			</div>
		</div>
	</body>
</html>
)=====";

char WebContent::webpage_head[] =
		R"=====( 	
<!DOCTYPE html>				
<html>
	<head>
		<meta charset='utf-8'>
		<title>Biozentrum Zoologie</title>
		<link rel="stylesheet" href="css/style.css" type="text/css" />
	</head>
	<body>
		<div class='page'>
			<div class='header'>
				<a href='/' id='logo'><img src='images/logo.jpg' alt=''/></a>
			</div>
			<div class='body'>
)=====";

char WebContent::webpage_tail[] =
		R"=====(
			</div>
			<div class='footer'>
				<ul>
					<li><a href='/'>Home</a></li>
					<li><a href='/settings'>Settings</a></li>
					<li><a href='/sensors'>Sensors</a></li>
					<li><a href='/scale_calib'>Scale calibration</a></li>
				</ul>
			</div>
		</div>
	</body>
</html>
)=====";

String WebContent::create_heading(String h) {
	return _tagH1begin + h + _tagH1end;
}

String WebContent::create_entry(String s) {
	return _tagH2begin + s + _tagH2end;
}

String WebContent::create_temp_entry(float t) {
	String _t_val_s(t);
	return _temp + _t_val_s + " deg C";
}

String WebContent::create_humid_entry(float h) {
	String _h_val_s(h);
	return _humidity + _h_val_s + "%";
}

String WebContent::create_weight_entry(float w) {
	String _w_val_s(w);
	return _weight + _w_val_s + " g";
}

String WebContent::output(void) {
	String rv;
//	rv = create_heading() + create_temp_entry() + create_humid_entry();
	rv = webpage_head;
	for (auto n : content) {
		rv += "<div>";
		rv += n;
		rv += "</div>";
	}
	rv += webpage_tail;
	return rv;
}

void WebContent::append(String s) {
	content.push_back(s);
}

void WebContent::clear(void) {
	content.clear();
}

String WebContent::create_input_form(String name = "",
		String placeholder = "") {
	return "<form action=''><input type='text' name='" + name
			+ "' placeholder='" + placeholder + "'></form>";
}

