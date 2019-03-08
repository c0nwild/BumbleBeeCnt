/*
 * webcontent.h
 *
 *  Created on: 16.03.2018
 *      Author: conradwild
 */

#ifndef WEBCONTENT_H_
#define WEBCONTENT_H_

#include <WString.h>
#include <vector>

struct WebContent {

	void append(String s);
	void clear(void);

	String create_heading(String h);
	String create_entry(String s);

	String create_temp_entry(float t);
	String create_humid_entry(float h);
	String create_weight_entry(float w);

	String create_input_form(String name, String placeholder);
	String create_button(String name, String action);

	String output(void);

	static char webpage_body_main[];
	static char webpage_settings[];
	static char webpage_head[];
	static char webpage_tail[];

private:
	std::vector<String> content;
};

#endif /* WEBCONTENT_H_ */
