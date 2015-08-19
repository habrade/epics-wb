/*
 * EWBConsoleWR.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBConsoleWR.h"


EWBConsoleWR::EWBConsoleWR(EWBCmdConsole *term, float rgui_s)
:EWBCmdConsole(UNKNOWN),term(term),rgui_nclick((clock_t)rgui_s*CLOCKS_PER_SEC), t_last(0)
{
	if(term) _type=(EWBCmdConsole::CmdType)term->getType();

	// TODO Auto-generated constructor stub

}

EWBConsoleWR::~EWBConsoleWR() {
	// TODO Auto-generated destructor stub
}


/**
 * This specific function improve the access to the GUI
 * interface by making only one access to `gui` command
 * each rgui_ms.
 */
std::string EWBConsoleWR::getCmd(std::string cmd)
{
	if(cmd=="gui")
	{
		if(clock() - t_last > rgui_nclick)
		{
			lastgui=term->getCmd(cmd);
			t_last=clock();
		}
		return lastgui;
	}
	return term->getCmd(cmd);
}
