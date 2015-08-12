/*
 * EWBParam.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBPARAM_H_
#define EWBPARAM_H_

#include "EWBSync.h"

#include <string>


/**
 * Generic class that represent a parameter that need to
 * synchronized with the upper layer.
 *
 */
class EWBParam: public EWBSync {
public:
	EWBParam(std::string name, uint8_t type, uint8_t mode, std::string desc=""): EWBSync(mode), name(name), type(type),  desc(desc) {};
	virtual ~EWBParam() {};


	//! Type Mask (Used by EWBField)
	enum TMask {
		EWBF_TM_SIGN_MSB		= 0x1,
		EWBF_TM_SIGN_2COMP	= 0x2,
		EWBF_TM_SIGNESS		= 0x3,
		EWBF_TM_FIXED_POINT	= 0x4,
	};

	//! Type of EWBField available
	enum Type {
		//! Automatic Type
		EWBF_AUTO=0xFF,
		//! Unsigned integer field
		EWBF_32U=0,
		//! Signed integer field
		EWBF_32I=1,
		//! Fixed point field with highest bit signed
		EWBF_32FP = (EWBF_TM_FIXED_POINT | EWBF_TM_SIGN_MSB),
		//! Fixed point field with 2'complements signed
		EWBF_32F2C =(EWBF_TM_FIXED_POINT | EWBF_TM_SIGN_2COMP),
		//! String parameters
		EWBF_STRING,
	};

	const std::string& getName() const { return name; }		//!< Get the name
	const char *getCName() const { return name.c_str(); } 	//!< Get the name in "C" format for printf function
	const std::string& getDesc() const { return desc; }		//!< Get the description
	uint8_t getType() const { return type; }				//!< Get the type of field

protected:
	std::string name;	//!< Name of the EWBField
	uint8_t type;		//!< Type of data
	std::string desc;	//!< Description

};


/**
 * Generic class that represent a string parameter that need to
 * synchronized with the upper layer.
 */
class EWBParamStr: public EWBParam {
public:
	EWBParamStr(std::string name, uint8_t mode, std::string value, std::string desc=""): EWBParam(name,EWBParam::EWBF_STRING,mode,desc), value(value) {};
	virtual ~EWBParamStr() {};

protected:
	std::string value;
};



#endif /* EWBPARAM_H_ */
