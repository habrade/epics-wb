/*
 * EWBField_test.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBField.h"
#include "gtest/gtest.h"
#include "files/wbtest.h"


namespace
{


TEST(EWBField,ConstructorRst)
{
	EWBField f(NULL,WB2_FIELD_ARGS(TEST,CSR,RST));
	EXPECT_FALSE(f.isValid());
	EXPECT_EQ(NULL,f.getReg());
	EXPECT_EQ(WB2_TEST_CSR_RST_ACCESS,f.getAccessMode());
	EXPECT_EQ(WB2_TEST_CSR_RST_MASK,f.getMask());
	EXPECT_EQ(EWBParam::EWBF_32U,f.getType());
	EXPECT_EQ(WB2_TEST_CSR_RST_NBFP,f.getNOfFractionBit());
	EXPECT_EQ(0,f.getFloat());

	//std::cout << f << std::endl;
}


TEST(EWBField,ConstructorNumber)
{
	EWBField f(NULL,WB2_FIELD_ARGS(TEST,CSR,NUMBER));
	EXPECT_FALSE(f.isValid());
	EXPECT_EQ(NULL,f.getReg());
	EXPECT_EQ(WB2_TEST_CSR_NUMBER_ACCESS,f.getAccessMode());
	EXPECT_EQ(WB2_TEST_CSR_NUMBER_MASK,f.getMask());
	EXPECT_EQ(EWBParam::EWBF_32U,f.getType());
	EXPECT_EQ(WB2_TEST_CSR_NUMBER_NBFP,f.getNOfFractionBit());
	EXPECT_EQ(0,f.getFloat());
}


TEST(EWBField,ReadOnly)
{
	EWBField f(NULL,WB2_FIELD_ARGS(TEST,CSR,NUMBER));
	uint32_t val=123;
	uint32_t reg;
	EXPECT_FALSE(f.isValid());
	EXPECT_FALSE(f.convert(&val,true)); //No EWBReg defined
	EXPECT_TRUE(f.regCvt(&val,&reg,true));
	f.getU32();
}


TEST(EWBField,CvtIQ)
{
	EWBField fI(NULL,WB2_FIELD_ARGS(TEST,DAC,I));
	EWBField fQ(NULL,WB2_FIELD_ARGS(TEST,DAC,Q));
	float I=0.5436,Q=0.0234;
	float abs_error=0.01;
	float Irbk,Qrbk;
	uint32_t reg1=0, reg2=0, reg_tmp;
	EXPECT_FALSE(fI.isValid());
	EXPECT_FALSE(fI.convert(&I,true)); //EWBReg is NULL
	EXPECT_TRUE(fI.regCvt(&I,&reg1,false));
	EXPECT_TRUE(fI.regCvt(&Irbk,&reg1,true));
	EXPECT_NEAR(I, Irbk, abs_error);
	reg_tmp=reg1;
	EXPECT_FALSE(fQ.isValid());
	EXPECT_FALSE(fQ.convert(&Q,true)); //EWBReg is NULL
	EXPECT_TRUE(fQ.regCvt(&Q,&reg1,false));
	EXPECT_TRUE(fQ.regCvt(&Qrbk,&reg1,true));
	EXPECT_NEAR(Q, Qrbk, abs_error);

	//Check Register mask
	EXPECT_NE(reg_tmp,reg1);
	EXPECT_EQ(reg_tmp,reg1 & WB2_TEST_DAC_I_MASK);
	EXPECT_TRUE(fQ.regCvt(&Q,&reg2,false));
	reg_tmp=reg2;
	EXPECT_TRUE(fI.regCvt(&I,&reg2,false));
	EXPECT_NE(reg_tmp,reg2);
	EXPECT_EQ(reg_tmp,reg2 & WB2_TEST_DAC_Q_MASK);
	EXPECT_EQ(reg2,reg1);
}

TEST(EWBField,CvtAmpPh)
{
	EWBField fA(NULL,WB2_FIELD_ARGS(TEST,ADC,AMP));
	EWBField fP(NULL,WB2_FIELD_ARGS(TEST,ADC,PHA));
	float Amp,Pha;
	float AmpRbk,PhaRbk;
	float AmpErr=0.01,PhaErr=0.1;
	uint32_t reg=0;

	//Start playing with conversion
	Amp=0;
	EXPECT_TRUE(fA.regCvt(&Amp,&reg,false));
	EXPECT_TRUE(fA.regCvt(&AmpRbk,&reg,true));
	EXPECT_NEAR(Amp,AmpRbk,AmpErr);

	Amp=-1/3;
	EXPECT_TRUE(fA.regCvt(&Amp,&reg,false));
	EXPECT_TRUE(fA.regCvt(&AmpRbk,&reg,true));
	EXPECT_NEAR(Amp,AmpRbk,AmpErr);

	Amp=-2;
	EXPECT_TRUE(fA.regCvt(&Amp,&reg,false));
	EXPECT_TRUE(fA.regCvt(&AmpRbk,&reg,true));
	EXPECT_NEAR(Amp,AmpRbk,AmpErr);

	Amp=1.11111;
	EXPECT_TRUE(fA.regCvt(&Amp,&reg,false));
	EXPECT_TRUE(fA.regCvt(&AmpRbk,&reg,true));
	EXPECT_NEAR(Amp,AmpRbk,AmpErr);

	Amp=1.8;
	EXPECT_TRUE(fA.regCvt(&Amp,&reg,false));
	EXPECT_TRUE(fA.regCvt(&AmpRbk,&reg,true));
	EXPECT_NEAR(Amp,AmpRbk,AmpErr);

	//TODO: Correct this case
	Amp=2;
	EXPECT_TRUE(fA.regCvt(&Amp,&reg,false));
	EXPECT_TRUE(fA.regCvt(&AmpRbk,&reg,true));
	EXPECT_TRUE(fA.isOverflowPrevented());
	EXPECT_NEAR(Amp,AmpRbk,AmpErr);


	Pha=0;
	EXPECT_TRUE(fP.regCvt(&Pha,&reg,false));
	EXPECT_TRUE(fP.regCvt(&PhaRbk,&reg,true));
	EXPECT_NEAR(Pha,PhaRbk,PhaErr);

	Pha=180;
	EXPECT_TRUE(fP.regCvt(&Pha,&reg,false));
	EXPECT_TRUE(fP.regCvt(&PhaRbk,&reg,true));
	EXPECT_NEAR(Pha,PhaRbk,PhaErr);

	Pha=-179;
	EXPECT_TRUE(fP.regCvt(&Pha,&reg,false));
	EXPECT_TRUE(fP.regCvt(&PhaRbk,&reg,true));
	EXPECT_NEAR(Pha,PhaRbk,PhaErr);

	Pha=(2*180/3.f);
	EXPECT_TRUE(fP.regCvt(&Pha,&reg,false));
	EXPECT_TRUE(fP.regCvt(&PhaRbk,&reg,true));
	EXPECT_NEAR(Pha,PhaRbk,PhaErr);

	Pha=128.5;
	EXPECT_TRUE(fP.regCvt(&Pha,&reg,false));
	EXPECT_TRUE(fP.regCvt(&PhaRbk,&reg,true));
	EXPECT_EQ(0x404,reg >> 16); //128<<3 |(int)(0.5f/0.125f));
	EXPECT_NEAR(Pha,PhaRbk,PhaErr);
	uint32_t tmp;
	EXPECT_TRUE(fP.regCvt(&tmp,&reg,true));
	EXPECT_EQ(0x404,tmp);
}

/**
 * This test check if everything works fine when we use
 * the 32bit as unsigned integer.
 */
TEST(EWBField,FullU32)
{
	EWBField f(NULL,WB2_FIELD_ARGS(TEST,FULL,U32));
	uint32_t val,valRbk;
	uint32_t reg=0;

	val=0xFFFFFFFF;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
	EXPECT_EQ(val,reg);

	val=0xAAAAAAAA;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
	EXPECT_EQ(val,reg);

	val=0x55555555;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
	EXPECT_EQ(val,reg);

}

/**
 * This test check when the 32bits of the register
 * are used as fixed decimal bit.
 */
TEST(EWBField,FullFixed)
{
	EWBField f(NULL,WB2_FIELD_ARGS(TEST,FULLFIXED,ALL));
	float val,valRbk,valErr=0.000001;
	uint32_t reg=0;

	//Start playing with conversion
	val=-0.499999;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_NEAR(val,valRbk,valErr);

	//Start playing with conversion
	val= 0.499999;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_NEAR(val,valRbk,valErr);

	//Start playing with conversion
	val=-0.000000001;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_NEAR(val,valRbk,valErr);
	uint32_t regTmp=reg;

	//Start playing with conversion
	val=+0.000000001;
	EXPECT_TRUE(f.regCvt(&val,&reg,false));
	EXPECT_TRUE(f.regCvt(&valRbk,&reg,true));
	EXPECT_NEAR(val,valRbk,valErr);

	EXPECT_NEAR(0,reg-regTmp,10); //The two value should be really near (when 2c)
}


TEST(EWBField,Signess)
{
	uint32_t reg=0;
	float val,valRbk;

	//Unsigned byte
	EWBField fU=EWBField(NULL,WB2_FIELD_ARGS(TEST,BSIGN,U));
	val=255;
	EXPECT_TRUE(fU.regCvt(&val,&reg,false));
	EXPECT_TRUE(fU.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);

	val=-128;
	EXPECT_TRUE(fU.regCvt(&val,&reg,false));
	EXPECT_TRUE(fU.regCvt(&valRbk,&reg,true));
	EXPECT_NE(val,valRbk);

	EWBField f1=EWBField(NULL,WB2_FIELD_ARGS(TEST,BSIGN,SIGN1));
	val=127;
	EXPECT_TRUE(f1.regCvt(&val,&reg,false));
	EXPECT_TRUE(f1.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
	uint32_t regTmp=reg;

	val=-127;
	EXPECT_TRUE(f1.regCvt(&val,&reg,false));
	EXPECT_TRUE(f1.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
	EXPECT_NE(regTmp,reg);

	val=-128;
	EXPECT_TRUE(f1.regCvt(&val,&reg,false));
	EXPECT_TRUE(f1.regCvt(&valRbk,&reg,true));
	EXPECT_NE(val,valRbk);


	//printf("x%x x%x\n",reg,regTmp);

	EWBField f2=EWBField(NULL,WB2_FIELD_ARGS(TEST,BSIGN,SIGN2));
	val=127;
	EXPECT_TRUE(f2.regCvt(&val,&reg,false));
	EXPECT_TRUE(f2.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);

	val=-128;
	EXPECT_TRUE(f2.regCvt(&val,&reg,false));
	EXPECT_TRUE(f2.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
}


TEST(EWBField,FixedSigness)
{
	uint32_t reg=0;
	float val,valRbk;
	float step8=1/(256.f);
	float step7=1/(128.f);

	//Unsigned byte
	EWBField fU=EWBField(NULL,WB2_FIELD_ARGS(TEST,BFIXED,U));
	val=0.00390625; // 1/256
	EXPECT_TRUE(fU.regCvt(&val,&reg,false));
	EXPECT_TRUE(fU.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);

	val=-0.125;
	EXPECT_TRUE(fU.regCvt(&val,&reg,false));
	EXPECT_TRUE(fU.regCvt(&valRbk,&reg,true));
	EXPECT_NE(val,valRbk);
	EXPECT_TRUE(valRbk>=0);

	EWBField f1=EWBField(NULL,WB2_FIELD_ARGS(TEST,BFIXED,SIGN1));
	val=(1-step7);
	EXPECT_TRUE(f1.regCvt(&val,&reg,false));
	EXPECT_TRUE(f1.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);

	val=-(1-step7);
	EXPECT_TRUE(f1.regCvt(&val,&reg,false));
	EXPECT_TRUE(f1.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);


	EWBField f2=EWBField(NULL,WB2_FIELD_ARGS(TEST,BFIXED,SIGN2));
	val=0.5-step8;
	EXPECT_TRUE(f2.regCvt(&val,&reg,false));
	EXPECT_TRUE(f2.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);

	val=-0.5;
	EXPECT_TRUE(f2.regCvt(&val,&reg,false));
	EXPECT_TRUE(f2.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(val,valRbk);
}



TEST(EWBField,Overflow)
{
	uint32_t reg=0;
	float val,valRbk;
	float step8=1/(256.f);
	float step7=1/(128.f);

	EWBField fA(NULL,WB2_FIELD_ARGS(TEST,ADC,AMP));
	val=3;
	EXPECT_TRUE(fA.regCvt(&val,&reg,false));
	EXPECT_TRUE(fA.regCvt(&valRbk,&reg,true));
	EXPECT_NE(2,valRbk);
	EXPECT_NEAR(2,valRbk,0.01);


	EWBField fU=EWBField(NULL,WB2_FIELD_ARGS(TEST,BSIGN,U));
	val=-128;
	EXPECT_TRUE(fU.regCvt(&val,&reg,false));
	EXPECT_TRUE(fU.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(0,valRbk);

	val=256;
	EXPECT_TRUE(fU.regCvt(&val,&reg,false));
	EXPECT_TRUE(fU.regCvt(&valRbk,&reg,true));
	EXPECT_EQ(255,valRbk);
}





}
