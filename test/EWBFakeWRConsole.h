/*
 * EWBFakeWRConsoleConsole.h
 *
 *  Created on: Aug 10, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBFAKEWRCONSOLE_H_
#define EWBFAKEWRCONSOLE_H_

#include <map>
#include <EWBCmdConsole.h>


/**
 * Class to test the #WRC Console
 */
class EWBFakeWRConsole: public EWBCmdConsole {
public:
	EWBFakeWRConsole();
	virtual ~EWBFakeWRConsole();

	void writeCmd(std::string cmd, std::string value);
	std::string getCmd(std::string cmd);
	bool isValid() const { return true; }

	const std::string& getInfo() const { return info; };

private:
	void addCmd(const std::string& cmd, const std::string& ret);

	std::string info;
	std::map<std::string, std::string> cMap;
};

#endif /* EWBFAKEWRCONSOLE_H_ */
