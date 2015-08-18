/*
 * EWBParamStrCmd.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBPARAMSTRCMD_H_
#define EWBPARAMSTRCMD_H_

#include "EWBParam.h"
#include <regex.h>

class EWBCmdConsole; //!< Pre-call to improve compilation


/**
 * This class is used to handle parameters that can be configured through a command
 * interface such as the wrc# console.
 */
class EWBParamStrCmd: public EWBParamStr {
public:
	EWBParamStrCmd(EWBCmdConsole *pConsole,const std::string& name, const std::string& cmdW, const std::string& cmdR, const std::string& rgxpR="", const std::string& desc="");
	virtual ~EWBParamStrCmd();
	const std::string& getValue() const { return value; }		//!< Get the name
	bool sync(EWBSync::AMode mode);
	bool isValid(int level=-1) const;

protected:

	EWBCmdConsole *pConsole;	//!< Pointer on the cmd console

	std::string cmdW;	//!< Consoled command corresponding to this parameter
	std::string cmdR;	//!< Consoled command corresponding to this parameter
	regex_t rgxpR;	//!< Regular to retrieve the value of this command

};

#endif /* EWBPARAMSTRCMD_H_ */
