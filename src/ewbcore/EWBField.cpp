/*
 * EWBField.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBField.h"

#include "EWBReg.h"
#include "EWBPeriph.h"
#include "EWBTrace.h"

#include "ewbbridge/EWBBridge.h"

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstdarg>
#include <ctype.h>
#include <limits>

#ifdef vxWorks
template<typename T>
inline bool isinf(T value)
{
return std::numeric_limits<T>::has_infinity &&
value == std::numeric_limits<T>::infinity();
}
#endif


#define TRACE_P_VDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)
#define TRACE_P_VVDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)

/**
 * Default constructor of the EWBField
 *
 * If pReg is valid it will automatically append this EWBField to a EWBReg only if
 * the bit mask of EWBField is not already used in EWBReg.
 *
 * \param[in] pReg Belonging EWBReg
 * \param[in] name Name of the field
 * \param[in] mask Bit mask of the field on the register
 * \param[in] shift how many bit we need to shift
 * \param[in] mode The access mode to the register given by \ref EWBSync::AMode
 * \param[in] desc Optional description
 * \param[in] signess Which type of signess we want to use EWBField::TMask
 * \param[in] nfb Number of fractional bit.
 * \param[in] iniVal Value to setup field at creation,
 * if set to "inf" we keep 0 as default value. Does nothing when pReg is NULL.
 */
EWBField::EWBField(EWBReg *pReg,
		const std::string &name, uint8_t width,
		uint8_t shift, uint8_t mode, const std::string &desc,
		uint8_t signess, uint8_t nfb, int index, double iniVal):
		EWBParam(name,EWBF_TM_TYPE_FIELD,mode,desc)

{
	if((width+shift)>32) TRACE_P_WARNING("width (%d) + shift (%d) <= 32 bits",width,shift);

	if(this->type==EWBF_32FP) {
		if((width > nfb)==false)
			TRACE_P_WARNING("%s Width (%d) must be superior than nbfp (%d)",getCName(),width,nfb);
	}
	else {
		if((width >= nfb)==false)
			TRACE_P_WARNING("%s Width (%d) must be superior or equal than nbfp (%d)",getCName(),width,nfb);
	}

	this->pReg=pReg;
	this->width=width;
	this->shift=shift;
	this->type |=(signess & EWBF_TM_SIGNESS);
	if(nfb>0) this->type|=EWBF_TM_FIXED_POINT;
	this->nfb=nfb;
	this->forceSync=false;
	this->checkOverflow=true;
	this->mask = (((1ULL<<width)-1) << shift);

	getLimit(vmin,vmax);

	if(nfb>0) { TRACE_P_DEBUG("%s type=0x%0x nfb=%d, dVal=%f (x%08x) [%f,%15f]",name.c_str(),type,nfb,iniVal,mask,vmin,vmax); }
	else { TRACE_P_DEBUG("%s type=0x%0x nfb=%d, dVal=%f (x%08x) [%d,%d]",name.c_str(),type,nfb,iniVal,mask,(uint32_t)vmin,(uint32_t)vmax); }

	if(pReg)
	{
		bool added;
		if(isinf(iniVal)) added=pReg->addField(this,index);
		else
		{
			float dVal32=(float)iniVal;
			this->convert(&dVal32,false);
			added=pReg->addField(this,index,true);
		}
		if(added==false) this->pReg=NULL; //Remove linking
	}
}

//EWBField::EWBField(EWBReg *pReg,const std::string &name, uint32_t mask)
//{
//	int i = 1;
//	for (; mask; mask >>= 1, i++)
//		this->width=i-shift;
//}


/**
 * Empty destructor
 */
EWBField::~EWBField()
{

}

void EWBField::getLimit(float &fmin, float &fmax)
{
	float ftmp;
	uint32_t data, val;
	bool checkOF_old;
	switch(type)
	{
	case EWBF_32U:				//Unsigned integer
		fmin=0.f;
		fmax=(float)((1ULL<<width)-1);
		if(width>30) fmax=(float)((1ULL<<30)-1);
		break;
	case EWBF_32I:				//MSB Signed Integer
		fmin=-((float)(1ULL<<(width-1))-1);
		fmax=(float)(1ULL<<(width-1))-1;
		break;
	case EWBF_32I2C:	//2C Signed Integer
		fmin=-(float)(1ULL<<(width-1));
		fmax=(float)(1ULL<<(width-1))-1;
		break;
	case EWBF_32FPU: 	//Unsigned Fixed Point
		fmin=0;
		fmax=(float)(1ULL<<(width-nfb));
		fmax-=1.f/(1ULL<<nfb);
		break;
	case EWBF_32FP: 			//MSB Signed Fixed point (0x5)
		fmax=(float)(1ULL<<(width-1-nfb));
		fmax-=1.f/(1ULL<<nfb);
		fmin=-fmax;
		break;
	case EWBF_32F2C:			//2C Signed Fixed point (0x6)
		checkOF_old=checkOverflow;
		checkOverflow=false;
		val=(1ULL<<(width-1));
		this->regCvt(&val,&data,false);
		if(this->regCvt(&ftmp,&data,true)) fmin=ftmp;
		val--;
		this->regCvt(&val,&data,false);
		if(this->regCvt(&ftmp,&data,true)) fmax=ftmp;
		checkOverflow=checkOF_old;
		break;
	}
}


/**
 * Generic function to convert an integer value to/from a reg_data
 *
 * Simple transformation using the bit mask and the shift to transform the value.
 *
 * \warning In this transformation the sign is not taken in account.
 * \note This is a constant function so we do not modify any internal data of EWBField and we don't need a valid linked EWBReg.
 * \param[inout] value 		pointer to an integer value
 * \param[inout] reg_data 	pointer to a register data
 * \param[in] to_value if true, value will be out and reg_data in, when false this is swapped.
 * \return true if the operation success, false otherwise.
 */
bool EWBField::regCvt(uint32_t *value, uint32_t *reg_data, bool to_value) const
{
	if(to_value)
	{
		*value=(*reg_data&mask) >> shift;
	}
	else
	{
		*reg_data=((*value << shift) & mask) | (*reg_data & ~mask);
	}
	return true;
}

/**
 * Generic function to convert a float value to/from a reg_data
 *
 * The floating point value can be converted in different way according to the \ref getType()
 * 		- EWBField::EWBF_32U  truncate to u32 and use the integer \ref regCvt() function
 * 		- EWBField::EWBF_32FP convert using signed fixed point conversion
 * 		- EWBField::EWBF_32F2C convert using 2 complements fixed point conversion so that
 * 		we only use 1bit for zero. If \ref isOverflowPrevented() we limit the value to out bit width.\n
 * 		More info: http://en.wikipedia.org/wiki/Two%27s_complement
 *
 *
 *
 * \note This is a constant function so we do not modify any internal data of EWBField and we don't need a valid linked EWBReg.
 * \param[inout] value 		pointer to an integer value
 * \param[inout] reg_data 	pointer to a register data
 * \param[in] to_value if true, value will be out and reg_data in, when false this is swapped.
 * \return true if the operation success, false otherwise.
 */
bool EWBField::regCvt(float *value, uint32_t *reg_data, bool to_value) const
{
	bool ret=false;
	uint32_t fixed, utmp;
	float ftmp;
	if(to_value==false)
	{
		if(checkOverflow==false) ftmp=*value;
		else
		{
			if(*value>vmax)
				ftmp=vmax;
			else if(*value<vmin)
				ftmp=vmin;
			else
				ftmp=*value;
		}
	}


	switch(type)
	{
	case EWBF_32U:
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			*value=(float)fixed;
		}
		else
		{
			fixed=(uint32_t)round(ftmp);
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case EWBF_32I:
		utmp=(1<<(this->width-1));
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			if(fixed & utmp) *value=-1.f*(float)(fixed & ~utmp);
			else *value=(float)fixed;
		}
		else
		{
			fixed=(uint32_t)round(fabs(ftmp)) & ~utmp;
			if(ftmp<0) fixed |=utmp;
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case EWBF_32I2C:
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			if (fixed & (1 << (this->width-1))) //Negative 2C
			{
				fixed=((~fixed)+1) & (this->mask >> this->shift);
				*value=-1.f*(float)fixed;
			}
			else
				*value=(float)fixed;
		}
		else
		{
			fixed=(uint32_t)round(fabs(ftmp));
			if(ftmp<0) fixed=(~(fixed))+1; 				//convert absolute signed fixed point to 2C fixed point when value <0
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case EWBF_32FPU: //Unsigned Fixed point conversion
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			*value=(float)(fixed/pow(2,this->nfb));
		}
		else
		{
			fixed=(uint32_t)(round(ftmp * pow(2,this->nfb)));
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case EWBF_32FP: 			//Signed Fixed point conversion
		utmp=(1<<(this->width-1));
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			if(fixed & utmp) *value=-1.f*(float)((fixed & ~utmp)/pow(2,this->nfb));
			else *value=(float)(fixed/pow(2,this->nfb));
		}
		else
		{
			fixed=(uint32_t)(round(fabs(ftmp) * pow(2,this->nfb))) & ~utmp;
			if(ftmp<0) fixed |=utmp;
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case EWBF_32F2C: //2 complements fixed point conversion
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			if (fixed & (1 << (this->width-1)))
			{
				fixed=((~fixed)+1) & (this->mask >> this->shift) ; //Convert negative 2C to negative Fixed Point
				*value=(-1.f / (float)(1ULL<<this->nfb)) * (float)fixed; // then to floating
			}
			else
				*value= (1.f / (float)(1ULL<<this->nfb)) * (float)fixed;			//Convert directly Fixed Point to Floating Point
		}
		else
		{
			fixed=round((double)fabs(ftmp) * (double)(1ULL << this->nfb)); //convert to signed fixed point using absolute value
			if(ftmp<0) fixed=(~(fixed))+1; 				//convert absolute signed fixed point to 2C fixed point when value <0
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	default:
		TRACE_P_WARNING("%s: Type %d not defined",getCName(),getType());
		return false;
	}
	return ret;
}


/**
 * Shortcut function for converting from/to float value
 *
 * This function will use the corresponding bit stored in the associate EWBReg
 * and will convert them to/from the given pVal. A valid EWBReg must be linked to
 * this object.
 *
 * \see Call \ref regCvt() method using EWBReg::data as reg_data.
 * \param[inout] pVal pointer to a value
 * \param[in] to_value if true, value will be out. Otherwise it will be in.
 * \return true if the operation success, false otherwise.
 */
bool EWBField::convert(uint32_t *pVal, bool to_value)
{
	return (pReg)?regCvt(pVal,&(pReg->data),to_value):false;
}

/**
 * Shortcut function for converting from/to float value
 *
 * This function will use the corresponding bit stored in the associate EWBReg
 * and will convert them to/from the given pVal.
 *
 * \see Call \ref regCvt() method using EWBReg::data as reg_data.
 * \param[inout] pVal pointer to a value
 * \param[in] to_value if true, value will be out. Otherwise it will be in.
 * \return true if the operation success, false otherwise.
 */
bool EWBField::convert(float *pVal, bool to_value)
{
	return (pReg)?regCvt(pVal,&(pReg->data),to_value):false;
}
/**
 * Shortcut to setup the register to be sync with the device ASAP.
 *
 * \note Once the corresponding register flag toSync is true it is not possible to reset
 * it unless calling to EWBReg::sync() method. Calling to EWBField::sync() will not reset this
 * flag.
 *
 * \return true if everything okay. otherwirse false if pointer on register is unvalid.
 */
bool EWBField::setToSync()
{
	TRACE_CHECK_PTR(pReg,false);
	this->toSync=true;
	pReg->toSync=true;
	return true;
}

/**
 * Shortcut function to return a float from the data in the field
 */
float EWBField::getFloat() const
{
	float val=0;
	if(pReg) regCvt(&val,&(pReg->data),true);
	return val;
}

/**
 * Shortcut function to return a uint32_t from the data in the field
 */
uint32_t EWBField::getU32() const
{
	uint32_t val=0;
	if(pReg) regCvt(&val,&(pReg->data),true);
	return val;
}

/**
 * Synchronize only the EWBField bits with memory
 *
 * This function has a mechanism that only write and read
 * on the desired EWBField.
 * 		- Writing: we first need to read the actual value on the device, so that we can keep
 * 		the non-corresponding to the value on the device.
 * 		- Reading: Only update the corresponding bit.
 *
 * \note in R/W mode we first perform write so that we can check back the value we have wrote.
 *
 * \param[in] con A pointer to a valid Memory Connector object.
 * \param[in] amode Access mode (R, W, R/W)
 * \return true if everything is okay, false otherwise
 */
bool EWBField::sync(EWBSync::AMode amode)
{
	bool ret=true;
	uint32_t oldval, value;

	//Perform some check
	if(!isValid(true)) return false;

	EWBBridge *b=pReg->getPeriph()->getBridge();

	//first write to dev
	if(amode & EWB_AM_W)
	{
		//Get current value
		ret &=b->mem_access(pReg->getOffset(true),&oldval,false); //Read EWB from dev
		value=(pReg->data & mask) | (oldval & ~mask); //Update only our field
		TRACE_P_DEBUG("%-10s (@0x%08X) ret=%d old=0x%x new=0x%x",getCName(),pReg->getOffset(true),ret,oldval,value);
		if(oldval != value || forceSync)
		{
			ret &=b->mem_access(pReg->getOffset(true),&value,true); //Write EWB to dev
			TRACE_P_DEBUG("%-10s (@0x%08X) ret=%d value=0x%0x",getCName(),pReg->getOffset(true),ret,value);
		}
	}
	//then read from dev
	if(amode & EWB_AM_R)
	{
		ret &=b->mem_access(pReg->getOffset(true),&value,false); //Read EWB from dev
		pReg->data = (pReg->data & ~mask) | (value & mask); //update only our field
	}

	return ret;
}

/**
 * Return @true if this EWBField is valid.
 *
 * @note To be fully valid the EWBField must be connected to
 * EWBBridge in order to sync with the device, you can use level
 * to check if it is valid.
 * EWBBride (0) < EWBBus (1) < EWBPeriph (2) < EWBReg (3) < EWBField (4)
 *
 *
 * @param[in] level check the validity in each level.
 * This check is stopped when it reaches 0 (never when staring at -1)
 */
bool EWBField::isValid(int level) const
{
	if(level!=0) return (pReg && pReg->isValid(level-1));
	else return pReg;
}

/**
 * operator that print the data of the EWBField in a stream
 */
std::ostream & operator<<(std::ostream & o, const EWBField &f)
{
	o << EWBTrace::string_format("0x%08X ",f.mask) << f.name;
	o << " (" << ((f.mode & EWBSync::EWB_AM_R)?"R":"") << ((f.mode & EWBSync::EWB_AM_W)?"W":"") << ")";
	if(f.type==EWBField::EWBF_32F2C)
		o << " FixedPoint with 2comp (nfb=" << std::dec << (int)f.nfb << ")";
	return o;
}
