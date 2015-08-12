/*
 * EWBParamStrCmd_test.cpp *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBParamStrCmd.h"
#include "EWBFakeWRConsole.h"
#include "gtest/gtest.h"

namespace {


TEST(EWBParamStrCmd,Constructor)
{

	EWBParamStrCmd p(NULL,"ip","ip set","","Set the IP");
	EXPECT_FALSE(p.isValid());
	//EXPECT_STREQ(std::string("ip"),p.getName());
	EXPECT_STREQ("ip",p.getCName());
	EXPECT_EQ(EWBSync::EWB_AM_W,p.getAccessMode());
	EXPECT_STREQ("",p.getValue().c_str());
}

TEST(EWBParamStrCmd,Sync)
{
	EWBCmdConsole *pConsole = (EWBCmdConsole*)new EWBFakeWRConsole();

	EWBParamStrCmd p(pConsole,"mode","mode %s","mode","","Set/Get the PTP mode");
	EXPECT_TRUE(p.isValid());
	EXPECT_EQ(EWBSync::EWB_AM_RW,p.getAccessMode());
	EXPECT_STREQ("",p.getValue().c_str());
	p.sync(EWBSync::EWB_AM_R);
	EXPECT_STREQ("slave",p.getValue().c_str());

	delete pConsole;
}


TEST(EWBParamStrCmd,Regexp1stLine)
{
	EWBCmdConsole *pConsole = (EWBCmdConsole*)new EWBFakeWRConsole();

	EWBParamStrCmd p(pConsole,"wrc-version","","ver","[^:]*: (.*-v1.0)","Get the version of wrcore");
	EXPECT_TRUE(p.isValid());
	EXPECT_EQ(EWBSync::EWB_AM_R,p.getAccessMode());
	EXPECT_STREQ("",p.getValue().c_str());
	p.sync(EWBSync::EWB_AM_R);
	EXPECT_STREQ("wrpc-FAKE-v1.0",p.getValue().c_str());

	delete pConsole;
}

TEST(EWBParamStrCmd,Regexp2ndLine)
{
	EWBCmdConsole *pConsole = (EWBCmdConsole*)new EWBFakeWRConsole();

	EWBParamStrCmd p(pConsole,"wrc-builddate","","ver",".*\n(.*)\n.*","Get build date of wrcore");
	ASSERT_TRUE(p.isValid());
	EXPECT_EQ(EWBSync::EWB_AM_R,p.getAccessMode());
	EXPECT_STREQ("",p.getValue().c_str());
	p.sync(EWBSync::EWB_AM_R);
	EXPECT_STREQ("Built on Apr  7 2015, 19:02:39",p.getValue().c_str());

	delete pConsole;
}


TEST(EWBParamStrCmd,GUIRegexpRTT)
{
	EWBCmdConsole *pConsole = (EWBCmdConsole*)new EWBFakeWRConsole();

	//printf("%s",pConsole->getCmd("gui").c_str());

	EWBParamStrCmd p(pConsole,"rtt-delay","","gui","rtt delay:[ ]*([0-9]*) ps.*\n","Get Round-Trip Cable delay");
	ASSERT_TRUE(p.isValid());
	EXPECT_EQ(EWBSync::EWB_AM_R,p.getAccessMode());
	EXPECT_STREQ("",p.getValue().c_str());
	p.sync(EWBSync::EWB_AM_R);
	EXPECT_STREQ("119365",p.getValue().c_str());

	delete pConsole;
}




}


