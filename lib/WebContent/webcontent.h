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

	String output(void);

private:
	std::vector<String> content;
};

#endif /* WEBCONTENT_H_ */
