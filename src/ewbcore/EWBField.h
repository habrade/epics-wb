/*
 * EWBField.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBFIELD_H_
#define EWBFIELD_H_

#include "EWBParam.h"
#include "EWBReg.h"

//! Generate field preprocessor variable name for wbgen2 header
#define WB2_TOKENPASTING_FIELD(periphname,regname,fieldname,type) \
		WB2_##periphname##_##regname##_##fieldname##type

//! Generate register preprocessor variable name for wbgen2 header
#define WB2_TOKENPASTING_REG(periphname,regname,type) \
		WB2_##periphname##_REG_##regname##type

//! Shortcut for WBReg constructor arguments
#define WB2_REG_ARGS(pname,rname) \
		WB2_TOKENPASTING_REG(pname,rname,_PREFIX),\
		WB2_TOKENPASTING_REG(pname,rname,), \
		WB2_TOKENPASTING_REG(pname,rname,_NFIELDS), \
		WB2_TOKENPASTING_REG(pname,rname,_NAME)

//! Shortcut for WBField constructor arguments
#define WB2_FIELD_ARGS(pname,rname,fname) \
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_PREFIX),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SIZE),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SHIFT),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_ACCESS),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_DESC), \
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SIGN),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_NBFP),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_INDEX)

//! Shortcut for WBField constructor arguments
#define WB2_FIELD_ARGS_VA(pname,rname,fname,...) \
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_PREFIX),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SIZE),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SHIFT),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_ACCESS),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_DESC),\
		__VA_ARGS__


#include <iostream>

/**
 * Class that represent a Wishbone field on a EWBReg
 *
 * The EWBField is a part (or not) of a EWBReg.
 * We use a mask to show which bits correspond to this EWBField in
 * the linked EWBReg.
 *
 * \todo Verify the all the combination of regCvt() according to type !!!
 *
 * \ref EWBReg
 */
class EWBField: public EWBParam {
public:
	friend std::ostream & operator<<(std::ostream & output, const EWBField &n);

	EWBField(EWBReg *pReg,const std::string &name, uint8_t width, uint8_t shift,
			uint8_t mode=EWB_AM_RW, const std::string &desc="",
			uint8_t signess=0, uint8_t nfb=0, int index=-1, double defVal=(1.0/0.0));

	virtual ~EWBField();

	bool regCvt(uint32_t *value, uint32_t *regdata, bool from_data) const;
	bool regCvt(float *value, uint32_t *regdata, bool from_data) const;

	bool convert(uint32_t *value, bool to_value);
	bool convert(float *value, bool to_value);

	float getFloat() const;
	uint32_t getU32() const;
	bool setToSync();



	bool sync(EWBSync::AMode amode=EWB_AM_RW);

	uint32_t getMask() const { return mask; }				//!< Get the bit mask
	uint8_t getNOfFractionBit() const { return nfb; }		//!< Get the number of fractional bit (0 for EWBF_32U)
	const EWBReg* getReg() const { return pReg; }			//!< Get the linked register (RO)
	EWBReg* getReg() { return pReg; }						//!< Get the linked register
	bool isOverflowPrevented() const { return checkOverflow; }	//!< When true prevent overflow during FP conversion \ref regCvt(), \ ref convert()
	bool isValid(int level=-1) const;

protected:
	void getLimit(float &fmin, float &fmax);

	uint32_t mask;		//!< Corresponding mask
	uint8_t shift;		//!< Number of bit to be shift
	uint8_t width;		//!< Width of the field
	uint8_t nfb;		//!< Number of fraction bits
	bool checkOverflow;	//!< Limit overflow during FP conversion
	float vmin,vmax;		//!< Range that the user can use for this value

private:
	EWBReg *pReg; //! parent register which belong this field
};



#endif /* EWBFIELD_H_ */
