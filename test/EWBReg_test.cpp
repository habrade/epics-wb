/*
 * EWBReg_test.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBReg.h"

#include "EWBField.h"
#include "gtest/gtest.h"
#include "files/wbtest.h"

TEST(EWBReg,SimpleConstructor)
{
	EWBReg r(NULL,WB2_REG_ARGS(TEST,CSR));
	EXPECT_FALSE(r.isValid());
	EXPECT_EQ(NULL,r.getPrtNode());
	EXPECT_EQ(EWBSync::EWB_AM_RW,r.getAccessMode());
	EXPECT_STREQ(WB2_TEST_REG_CSR_PREFIX,r.getCName());
	EXPECT_STREQ(WB2_TEST_REG_CSR_NAME,r.getDesc().c_str());
	EXPECT_EQ(WB2_TEST_REG_CSR,r.getOffset(false));
	EXPECT_EQ(WB2_TEST_REG_CSR_NFIELDS,r.getFields().size());
	EXPECT_EQ(0,r.getData());
}


TEST(EWBReg,NFldConstructor)
{
	EWBField *pF;
	EWBReg *pR0 = new EWBReg(NULL,"regtest0",0x12345670);
	EXPECT_EQ(0x12345670,pR0->getOffset(false));
	EXPECT_EQ(0,pR0->getFields().size());

	pF = new EWBField(pR0,WB2_FIELD_ARGS(TEST,BFIXED,SIGN1));
	EXPECT_TRUE(pF->isValid(0));

	pF = new EWBField(pR0,WB2_FIELD_ARGS(TEST,BFIXED,SIGN2));
	EXPECT_TRUE(pF->isValid(0));

	pF = new EWBField(pR0,WB2_FIELD_ARGS(TEST,BFIXED,U));
	EXPECT_TRUE(pF->isValid(0));

	pF = new EWBField(pR0,WB2_FIELD_ARGS(TEST,BFIXED,DEFAULT));
	EXPECT_TRUE(pF->isValid(0));

	delete pR0;
	//--------------------------------------------------------

	EWBReg *pR1 = new EWBReg(NULL,"regtest1",0x76543210,3);
	EXPECT_EQ(0x76543210,pR1->getOffset(false));
	EXPECT_EQ(3,pR1->getFields().size());

	pF = new EWBField(pR1,WB2_FIELD_ARGS(TEST,BFIXED,SIGN1));
	EXPECT_TRUE(pF->isValid(0));

	pF = new EWBField(pR1,WB2_FIELD_ARGS(TEST,BFIXED,SIGN2));
	EXPECT_TRUE(pF->isValid(0));

	pF = new EWBField(pR1,WB2_FIELD_ARGS(TEST,BFIXED,U));
	EXPECT_TRUE(pF->isValid(0));

	//Try to add a fourth field to a pre-defined register of 3 fields.
	pF = new EWBField(pR1,WB2_FIELD_ARGS(TEST,BFIXED,DEFAULT));
	EXPECT_FALSE(pF->isValid(0));
	EXPECT_EQ(NULL,pF->getReg());

	delete pR1;
}

TEST(EWBReg,FieldConstructor)
{
	EWBReg *pR = new EWBReg(NULL,WB2_REG_ARGS(TEST,BFIXED));
	EWBField *pF2 = new EWBField(pR,WB2_FIELD_ARGS(TEST,BFIXED,SIGN1));
	EWBField *pF3 = new EWBField(pR,WB2_FIELD_ARGS(TEST,BFIXED,SIGN2));
	EWBField *pF1 = new EWBField(pR,WB2_FIELD_ARGS(TEST,BFIXED,U));
	EWBField *pF4 = new EWBField(pR,WB2_FIELD_ARGS(TEST,BFIXED,DEFAULT));

	EXPECT_FALSE(pR->isValid());
	EXPECT_EQ(NULL,pR->getPrtNode());
	EXPECT_EQ(WB2_TEST_REG_BFIXED_NFIELDS,pR->getFields().size());
	EXPECT_EQ(0,pR->getData());

	EXPECT_TRUE(pF1->isValid(0));
	EXPECT_TRUE(pF2->isValid(0));
	EXPECT_TRUE(pF3->isValid(0));
	EXPECT_TRUE(pF4->isValid(0));

	EXPECT_EQ(pF2,pR->getField(WB2_TEST_BFIXED_SIGN1_PREFIX));
	EXPECT_EQ(pF2,(*pR)[WB2_TEST_BFIXED_SIGN1_PREFIX]);
	EXPECT_EQ(pF2,(*pR)[WB2_TEST_BFIXED_SIGN1_PREFIX]);

	std::vector<EWBField*> fVec=pR->getFields();
	EXPECT_EQ(pF1,fVec[WB2_TEST_BFIXED_U_INDEX]);
	EXPECT_EQ(pF2,fVec[WB2_TEST_BFIXED_SIGN1_INDEX]);
	EXPECT_EQ(pF3,fVec[WB2_TEST_BFIXED_SIGN2_INDEX]);
	EXPECT_EQ(pF4,fVec[WB2_TEST_BFIXED_DEFAULT_INDEX]);

	delete pR;
}


TEST(EWBReg,BadFieldConstructor)
{
	EWBReg *pR;
	EWBField *pF;

	pR = new EWBReg(NULL,"n",0x4);
	EWBField *pF1 = new EWBField(pR,"f1",5,0);
	EWBField *pF2 = new EWBField(pR,"f2",3,4);

	EXPECT_TRUE(pF1->isValid(0));
	EXPECT_FALSE(pF2->isValid(0));
	EXPECT_EQ(NULL,pF2->getReg());

	delete pR;

	pR = new EWBReg(NULL,"n",0x4,3);
	pF = new EWBField(pR,"f1",5,0,EWBSync::EWB_AM_RW,"",0,0,0);
	EXPECT_TRUE(pF->isValid(0));
	pF = new EWBField(pR,"f2",2,5,EWBSync::EWB_AM_RW,"",0,0,1);
	EXPECT_TRUE(pF->isValid(0));
	pF = new EWBField(pR,"f3",4,8,EWBSync::EWB_AM_RW,"",0,0,1); //Retry same index
	EXPECT_FALSE(pF->isValid(0));
	EXPECT_EQ(NULL,pF->getReg());

	delete pR;
	EXPECT_STREQ("f3",pF->getCName());
	delete pF; //Don't belong to pR so we delete it
}

