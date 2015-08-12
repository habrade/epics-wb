/*
 * EWBField_test.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBField.h"
#include "gtest/gtest.h"
#include "files/wbtest.h"


//EWBField f(NULL,WB2_FIELD_ARGS(TEST,CSR,RST));

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

	std::cout << f << std::endl;
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

//TEST(EWBField,ConstructorNumber)
//{
//	EXPECT_NEAR(val1, val2, abs_error);
//}

}
