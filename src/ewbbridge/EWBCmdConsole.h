/*
 * EWBCmdConsole.h
 *
 *  Created on: Aug 10, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBCMDCONSOLE_H_
#define EWBCMDCONSOLE_H_

#include <EWBSync.h>
#include <string>

class EWBCmdConsole {
public:
	enum CmdType { UNKNOWN=-1, SERIAL, MEMVUART, ETHVUART };

	EWBCmdConsole(EWBCmdConsole::CmdType t): _type((int)t) {};
	virtual ~EWBCmdConsole() {};

	virtual void writeCmd(std::string cmd, std::string value)=0;
	virtual std::string getCmd(std::string cmd)=0;
	virtual const std::string& getInfo() const =0;
	virtual bool isValid() const =0;

	int getType() const { return _type; }

protected:
	int _type;
};


class SerialPort;


class EWBSerialConsole: public EWBCmdConsole {
	EWBSerialConsole(SerialPort tty);
	bool sync(EWBSync::AMode mode);
};




#endif /* EWBCMDCONSOLE_H_ */
