/*
 * EWBCmdConsole.h
 *
 *  Created on: Aug 10, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBCMDCONSOLE_H_
#define EWBCMDCONSOLE_H_

#include <EWBSync.h>

class EWBCmdConsole {
public:
	enum Type { UNKNOWN=-1, SERIAL, MEMVUART, ETHVUART };

	EWBCmdConsole(Type t): _type(t) {};
	virtual ~EWBCmdConsole() {};

	virtual void writeCmd(std::string cmd, std::string value)=0;
	virtual std::string getCmd(std::string cmd)=0;
	virtual const std::string& getInfo() const =0;
	virtual bool isValid() const =0;

	Type getType() const { return _type; }

protected:
	Type _type;
};


class SerialPort;


class EWBSerialConsole: public EWBCmdConsole {
	EWBSerialConsole(SerialPort tty);
	bool sync(EWBSync::AMode mode);
};


/**
 * Class that wrap the EWBCmdConsole class to improve parsing of WR consoles
 */
class EWBWRConsole: public EWBCmdConsole {
	EWBWRConsole(EWBCmdConsole *term, int rgui_ms);
	virtual ~EWBWRConsole();

	void writeCmd(std::string cmd, std::string value) { term->writeCmd(cmd,value); }
	std::string getCmd(std::string cmd);
	const std::string& getInfo() const { return term->getInfo(); }
	Type getType() const { return term->getType(); }

private:
	EWBCmdConsole *term; //!< This is the real terminal
};

#endif /* EWBCMDCONSOLE_H_ */
