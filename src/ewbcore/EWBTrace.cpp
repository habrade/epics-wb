/*
 * EWBTrace.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include <EWBTrace.h>

#include <string>
#include <vector>
#include <cstdarg>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>



std::string EWBTrace::string_format(const std::string &fmt, ...) {
	int size = 512;
	char* buffer = 0;
	buffer = new char[size];
	va_list vl;
	va_start(vl, fmt);
	int nsize = vsnprintf(buffer, size, fmt.c_str(), vl);
	if(size<=nsize){ //fail delete buffer and try again
		delete[] buffer;
		buffer = 0;
		buffer = new char[nsize+1]; //+1 for /0
		nsize = vsnprintf(buffer, size, fmt.c_str(), vl);
	}
	std::string ret(buffer);
	va_end(vl);
	delete[] buffer;
	return ret;
}
