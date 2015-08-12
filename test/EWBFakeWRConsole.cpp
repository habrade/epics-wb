/*
 * EWBFakeWRConsole.cpp
 *
 *  Created on: Aug 10, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBFakeWRConsole.h"

EWBFakeWRConsole::EWBFakeWRConsole()
:EWBCmdConsole(UNKNOWN), info("Fake Console")
{

	addCmd("ver","WR Core build: wrpc-FAKE-v1.0\nBuilt on Apr  7 2015, 19:02:39\nBuilt for 00 kB RAM, stack is 0000 bytes ");
	addCmd("mode","slave");
	addCmd("mode slave","Locking PLL");
	addCmd("ptp stop","");
	addCmd("ptp start","Slave Only, clock class set to 255");
	addCmd("sfp erase","");
	addCmd("sfp detect","AXGE-1254-0531");
	addCmd("sfp match","Could not match to DB");
	addCmd("sfp add","");
	addCmd("gui","WR PTP Core Sync Monitor v 1.0\n\
Esc = exit\n\
\n\
TAI Time:                  Thu, Jan 1, 1970, 04:26:33\n\
\n\
wru1: Link up   (RX: 55761, TX: 15967), mode: WR Slave   Locked  Calibrated\n\
\n\
PTP status: slave\n\
\n\
Synchronization status:\n\
\n\
Servo state:               TRACK_PHASE\n\
Phase tracking:            ON\n\
Synchronization source:\n\
Aux clock status:\n\
\n\
Timing parameters:\n\
\n\
Round-trip time (mu):    770929 ps\n\
Master-slave delay:      409632 ps\n\
Master PHY delays:       TX: 174900 ps, RX: 255214 ps\n\
Slave PHY delays:        TX: 46407 ps, RX: 175043 ps\n\
Total link asymmetry:       -48335 ps\n\
Cable rtt delay:         119365 ps\n\
Clock offset:                   -1 ps\n\
Phase setpoint:                489 ps\n\
Skew:                           -3 ps\n\
Manual phase adjustment:         0 ps\n\
Update counter:               2486\n\
--");

}


EWBFakeWRConsole::~EWBFakeWRConsole() {
	// TODO Auto-generated destructor stub
}

void EWBFakeWRConsole::addCmd(const std::string& cmd, const std::string& ret)
{
	cMap.insert(std::pair<std::string,std::string>(cmd,ret));
}

void EWBFakeWRConsole::writeCmd(std::string cmd, std::string value)
{
	std::map<std::string,std::string>::const_iterator ii;
	ii=cMap.find(cmd);
	if(ii!=cMap.end())
	{
		printf("%s %s\n",cmd.c_str(),value.c_str());
	}
}

std::string EWBFakeWRConsole::getCmd(std::string cmd)
{
	std::map<std::string,std::string>::const_iterator ii;
	ii=cMap.find(cmd);
	if(ii!=cMap.end())
	{
		return ii->second;
	}
	return "";
}
