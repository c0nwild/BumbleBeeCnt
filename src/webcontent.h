/*
 * webcontent.h
 *
 *  Created on: 16.03.2018
 *      Author: conradwild
 */

#ifndef WEBCONTENT_H_
#define WEBCONTENT_H_

#include <WString.h>

struct WebContent {
	float _t_val;
	float _h_val;

	String output(void);
	void setTVal(float t);
	void setHVal(float h);
private:
	String create_temp_entry();
	String create_humid_entry();
};

#endif /* WEBCONTENT_H_ */
