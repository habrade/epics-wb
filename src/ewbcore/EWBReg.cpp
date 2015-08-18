/*
 * EEWBReg.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBReg.h"

#include "EWBField.h"
#include "EWBPeriph.h"
#include "EWBTrace.h"



#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstdarg>
#include <ctype.h>


#define TRACE_P_VDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)
#define TRACE_P_VVDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)



/**
 * Constructor of a EWBReg
 *
 * \param[in] pPrtNode if valid, we will append this register to the EWBNode.
 * \param[in] name The name of the register
 * \param[in] offset The EWB address offset respective to the peripheral (EWBNode)
 * \param[in] nfields Tell that this register is already set with a specific number of field
 * \param[in] desc a description of what does this registers in case it is needed.
 */
EWBReg::EWBReg(EWBPeriph *pPrtPeriph,const std::string &name, uint32_t offset, int nfields, const std::string &desc)
:EWBSync(EWB_AM_RW)
{
	this->pPeriph=pPrtPeriph;
	this->name=name;
	this->desc=desc;

	this->offset=offset;
	this->used_mask=0;
	this->data=0;
	this->toSync=false;
	this->nfields=nfields;

	if(nfields>0) fields.resize(nfields,NULL);
	bool added=false;
	if(this->pPeriph) added=this->pPeriph->appendReg(this);
	if(added==false) {
		this->pPeriph=NULL; //Remove linking
	}

	//TRACE_DEBUG("EWBNode::EWBNode() %s (%x, %x)\n",this->name.c_str(),&this->name,this);
}

/**
 * Destructor of EWBReg
 *
 * It also delete all the fields associate to this EWBReg.
 */
EWBReg::~EWBReg()
{
	for(size_t j=0;j<fields.size();j++)
	{
		if(fields[j]) delete fields[j];
	}
}

/**
 * Append a field to the register
 *
 * It will check if the field exist or if some bits of this mask
 * is already used.
 *
 *
 * \param[in] fld A pointer
 * \param[in] toSyncInit Tell if this field need to be sync at initialization
 * \return true if it was possible to add it, false otherwise.
 */
bool EWBReg::addField(EWBField *fld, int index, bool toSyncInit)
{
	const EWBReg* fld_reg=fld->getReg();
	if(fld==NULL || fld_reg==NULL || this!=fld_reg)
	{
		TRACE_P_WARNING("Field %s does not belong to this register %s (%s)",
				fld->getName().c_str(), this->getName().c_str(),fld_reg->getName().c_str());
		return false;
	}
	if((fld->getMask() & used_mask)!=0)
	{
		TRACE_P_WARNING("Can not add field %s (0x%08x), mask is already used in register %s (0x%08x)\n",
				fld->getCName(), fld->getMask(),this->getCName(),this->used_mask);
		return false;
	}
	if(index>0 && nfields>0 && (size_t)index>=fields.size())
	{
		TRACE_P_WARNING("This field index %s (%d) >= nfields %d\n",
				fld->getCName(),index,fields.size());
		return false;
	}
	if(index>0 && nfields>0 && fields[index]!=NULL)
	{
		TRACE_P_WARNING("This field index already exists %s. It can not be replaced by %s\n",
				fields[index]->getCName(),fld->getCName());
		return false;
	}


	if(index>=0 && nfields>0) fields[index]=fld;
	else fields.push_back(fld); //Append the field to the vector

	//append field mask to used mask of the whole register
	used_mask|=fld->getMask();
	this->toSync|=toSyncInit;

	return true;
}

/**
 * Get a pointer on the corresponding field
 *
 * \return A pointer on EWBField or NULL if it was not found
 */
const EWBField* EWBReg::getField(const std::string& name) const
{
	EWBField *f;
	for(size_t j=0;j<fields.size();j++)
	{
		f=fields[j];
		if(f && f->getName()==name) return f;
	}
	return NULL;
}

/**
 * Synchronize the 32bits of the EWBReg
 *
 * \note This method is also the only way to reset the toSync flag
 *
 * \param[in] con A pointer to a valid Memory Connector object.
 * \param[in] amode Access mode (R, W, R/W)
 * \return true if everything is okay, false otherwise
 */
bool EWBReg::sync(EWBSync::AMode amode)
{
	bool ret=true;
	if(!isValid()) return false;

	EWBBridge *b=pPeriph->getBridge();

	//first write to dev
	if(amode & EWB_AM_W)
	{
		ret &= b->mem_access(this->getOffset(true),&data,true); //Write EWB to dev
	}
	//then read from dev
	if(amode & EWB_AM_R)
	{
		ret &= b->mem_access(this->getOffset(true),&data,false); //Read EWB from dev
	}
	if(toSync) toSync=(ret==false); //Keep trying to sync if return was false
	return ret;
}

/**
 * Return @true if this EWBReg is valid.
 *
 * @note To be fully valid the register must be connected to
 * EWBBridge in order to sync with the device, you can use level
 * to check if it is valid.
 * EWBBride (0) < EWBBus (1) < EWBPeriph (2) < EWBReg (3) < EWBField (4)
 *
 *
 * @param[in] level check the validity in each level.
 * This check is stopped when it reaches 0 (never when staring at -1)
 */
bool EWBReg::isValid(int level) const
{
	if(level!=0) return (pPeriph && pPeriph->isValid(level-1));
	else return pPeriph;
}


/**
 * Return the offset of the register
 *
 * \param[in] absolute if true the offset is the absolute direction in relation with the EWBNode.
 *
 */
uint32_t EWBReg::getOffset(bool absolute) const
{
	if(absolute && isValid())
	{
		return pPeriph->getOffset(absolute)+offset;
	}
	return offset;
}

/**
 * operator that print the data of the EWBReg in a stream
 */
std::ostream & operator<<(std::ostream & o, const EWBReg &r)
{
	o << EWBTrace::string_format("@0x%08X (%s) : 0x%x",r.getOffset(true),r.getCName(),r.getData());
	return o;
}
