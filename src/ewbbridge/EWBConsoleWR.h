/*
 * EWBConsoleWR.h
 *
 *  Created on: Aug 20, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBCONSOLEWR_H_
#define EWBCONSOLEWR_H_

#include "EWBCmdConsole.h"
#include <ctime>
/**
 * Class that wrap the EWBCmdConsole class to improve parsing of WR consoles
 */
class EWBConsoleWR: public EWBCmdConsole {
public:
	EWBConsoleWR(EWBCmdConsole *term, float rgui_s=0.1);
	virtual ~EWBConsoleWR();
	void writeCmd(std::string cmd, std::string value) { term->writeCmd(cmd,value); }
	std::string getCmd(std::string cmd);
	const std::string& getInfo() const { return term->getInfo(); }
	int getType() const { return term->getType(); }

private:
	EWBCmdConsole *term; //!< This is the real terminal
	clock_t rgui_nclick;
	clock_t t_last;
	std::string lastgui;
};

#endif /* EWBCONSOLEWR_H_ */
