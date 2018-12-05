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

char WebContent::webpage_main[] = R"=====( 	
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
					Willkommen auf <b>BumbleBee Logger</b>.
				</p>
				<div>
					<form action='download'>
						<button>Get data</button>
					</form>
				</div>
			</div>
			<div class='footer'>
				<ul>
					<li><a href='/'>Home</a></li>
					<li><a href='/settings'>Settings</a></li>
				</ul>
			</div>
		</div>
	</body>
</html>
)=====";

char WebContent::webpage_settings[] = R"=====( 	
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
					<form action="" method="get">
						<input type="text" name="time" placeholder="HHMMDDMMYYYY"/>
					</form>
				</div>
			</div>
			<div class='footer'>
				<ul>
					<li><a href='/'>Home</a></li>
					<li><a href='/settings'>Settings</a></li>
				</ul>
			</div>
		</div>
	</body>
</html>
)=====";

String WebContent::create_heading(String h) {
	return _tagH1begin + h + _tagH1end;
}

String WebContent::create_entry(String s){
	return _tagH2begin + s + _tagH2end;
}

String WebContent::create_temp_entry(float t){
	String _t_val_s(t);
	return _tagH2begin + _temp + _t_val_s + "deg C" + _tagH2end;
}

String WebContent::create_humid_entry(float h){
	String _h_val_s(h);
	return _tagH2begin + _humidity + _h_val_s + _tagH2end;
}

String WebContent::output(void) {
	String rv;
//	rv = create_heading() + create_temp_entry() + create_humid_entry();
	for(auto n:content){
		rv += n;
	}
	return rv;
}

void WebContent::append(String s) {
	content.push_back(s);
}

void WebContent::clear(void){
	content.clear();
}
