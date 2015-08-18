/*
 * EWBPeriph_test.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBPeriph.h"

#include "EWBReg.h"
#include "EWBField.h"
#include "gtest/gtest.h"
#include "files/wbtest.h"

TEST(EWBPeriph,SimpleConstructor)
{
	EWBPeriph p(NULL,WB2_TEST_PERIPH_PREFIX,0x40000000,0x1234567,0xABCDEF);
	EXPECT_FALSE(p.isValid());
	EXPECT_EQ(NULL,p.getBridge());
	EXPECT_EQ(0x1234567,p.getVendorID());
	EXPECT_EQ(0xABCDEF,p.getDeviceID());
	EXPECT_EQ(EWBSync::EWB_AM_RW,p.getAccessMode());
	EXPECT_STREQ(WB2_TEST_PERIPH_PREFIX,p.getCName());
	EXPECT_STREQ("",p.getDesc().c_str());
	EXPECT_EQ(0x40000000,p.getOffset(false));

	EXPECT_EQ(NULL,p.getReg(0x0000));
	EXPECT_EQ(NULL,p.getReg(0x0004));
	EXPECT_EQ(NULL,p.getNextReg(NULL));
	EXPECT_EQ(NULL,p.getLastReg());
}


TEST(EWBPeriph,IndexConstructor)
{

	EWBPeriph p0(NULL,WB2_TEST_PERIPH_PREFIX,0x40000000,0x1234567,0xABCDEF);
	int idx_p0=p0.getIndex();

	EWBPeriph *pP1 = new EWBPeriph(NULL,WB2_TEST_PERIPH_PREFIX,0x40000000,0x1234567,0xABCDEF);
	EXPECT_EQ(idx_p0+1,pP1->getIndex());

	EWBPeriph p2(NULL,WB2_TEST_PERIPH_PREFIX,0x40000000,0x1234567,0xABCDEF);
	EXPECT_EQ(idx_p0+2,p2.getIndex());

	delete pP1;

	EWBPeriph p3(NULL,WB2_TEST_PERIPH_PREFIX,0x40000000,0x1234567,0xABCDEF);
	EXPECT_EQ(idx_p0+3,p3.getIndex());
}

TEST(EWBPeriph,BadRegOffset)
{
	EWBPeriph p(NULL,WB2_TEST_PERIPH_PREFIX,0x40000000,0x1234567,0xABCDEF);

	EWBReg *pR1 = new EWBReg(&p,"reg1",0x0004);
	EWBReg *pR2 = new EWBReg(&p,"reg2",0x0004);

	EXPECT_EQ(pR1,p.getReg(0x0004));
	EXPECT_TRUE(pR1->isValid(0));
	EXPECT_FALSE(pR2->isValid(0));
}



TEST(EWBPeriph,TreeStructure)
{
	EWBPeriph *pP = new EWBPeriph(NULL,WB2_PRH_ARGS(TEST,0x60000000));

	EWBReg *pRFix = new EWBReg(pP,WB2_REG_ARGS(TEST,BFIXED));
	EWBField *pRFixF2 = new EWBField(pRFix,WB2_FIELD_ARGS(TEST,BFIXED,SIGN1));
	EWBField *pRFixF3 = new EWBField(pRFix,WB2_FIELD_ARGS(TEST,BFIXED,SIGN2));
	EWBField *pRFixF1 = new EWBField(pRFix,WB2_FIELD_ARGS(TEST,BFIXED,U));
	EWBField *pRFixF4 = new EWBField(pRFix,WB2_FIELD_ARGS(TEST,BFIXED,DEFAULT));

	EWBReg *pRSig = new EWBReg(pP,WB2_REG_ARGS(TEST,BSIGN));
	EWBField *pRSigF3 = new EWBField(pRSig,WB2_FIELD_ARGS(TEST,BSIGN,SIGN2));
	EWBField *pRSigF2 = new EWBField(pRSig,WB2_FIELD_ARGS(TEST,BSIGN,SIGN1));
	EWBField *pRSigF1 = new EWBField(pRSig,WB2_FIELD_ARGS(TEST,BSIGN,U));

	EWBReg *pRAdc = new EWBReg(pP,WB2_REG_ARGS(TEST,ADC));

	//Check recursive valid
	EXPECT_FALSE(pRFix->isValid());
	EXPECT_TRUE(pRFix->isValid(0));
	EXPECT_FALSE(pRFix->isValid(1));
	EXPECT_TRUE(pRFixF1->isValid(1));
	EXPECT_TRUE(pRFixF2->isValid(1));
	EXPECT_TRUE(pRFixF3->isValid(1));
	EXPECT_TRUE(pRFixF4->isValid(1));
	EXPECT_FALSE(pRFixF4->isValid(2));

	EXPECT_EQ(pRFix,pP->getLastReg());

	EWBReg *pRTmp=NULL;
	pRTmp=pP->getNextReg(pRTmp);
	EXPECT_EQ(pRAdc,pRTmp);
	pRTmp=pP->getNextReg(pRTmp);
	EXPECT_EQ(pRSig,pRTmp);
	pRTmp=pP->getNextReg(pRTmp);
	EXPECT_EQ(pRFix,pRTmp);
	pRTmp=pP->getNextReg(pRTmp);
	EXPECT_EQ(NULL,pRTmp);


	EXPECT_EQ(pRSig,pP->getReg(WB2_TEST_REG_BSIGN));
	EXPECT_EQ(pRFix,pP->getReg(WB2_TEST_REG_BFIXED));
	EXPECT_EQ(NULL,pP->getReg(WB2_TEST_REG_DAC));
	EXPECT_EQ(pRAdc,pP->getReg(WB2_TEST_REG_ADC));

	float val=-0.125, valRdbk;
	pRFixF3->convert(&val,false);
	EXPECT_NE(0,pRFix->getData());
	pRFixF3->convert(&valRdbk,true);
	EXPECT_EQ(val,valRdbk);
	EXPECT_EQ(val,pRFixF3->getFloat());


	std::cout << *pP << std::endl;

	delete pP;
}
