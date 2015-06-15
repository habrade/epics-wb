/**
 *
 * WBNode.cpp
 *
 *  Created on: Oct 11, 2013
 *      Author: neub
 */

#include "WBNode.h"

#include <iostream>
#include <cmath>

#include "awbpd_trace.h"

#define TRACE_P_VDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)
#define TRACE_P_VVDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)


#include <string>
#include <vector>
#include <cstdarg>
std::string string_format(const std::string &fmt, ...) {
	int size = 512;
	char* buffer = 0;
	buffer = new char[size];
	va_list vl;
	va_start(vl, fmt);
	int nsize = vsnprintf(buffer, size, fmt.c_str(), vl);
	if(size<=nsize){ //fail delete buffer and try again
		delete[] buffer;
		buffer = 0;
		buffer = new char[nsize+1]; //+1 for /0
		nsize = vsnprintf(buffer, size, fmt.c_str(), vl);
	}
	std::string ret(buffer);
	va_end(vl);
	delete[] buffer;
	return ret;
}

//========================================================================

/**
 * Default constructor of the WBField
 *
 * If pReg is valid it will automatically append this WBField to a WBReg only if
 * the bit mask of WBField is not already used in WBReg.
 *
 * \param[in] pReg Belonging WBReg
 * \param[in] name Name of the field
 * \param[in] mask Bit mask of the field on the register
 * \param[in] shift how many bit we need to shift
 * \param[in] mode The access mode to the register given by \ref WBAccMode
 * \param[in] desc Optional description
 * \param[in] signess Which type of signess we want to use WBField::TMask
 * \param[in] nfb Number of fractional bit.
 * \param[in] iniVal Value to setup field at creation,
 * if set to "inf" we keep 0 as default value. Does nothing when pReg is NULL.
 */
WBField::WBField(WBReg *pReg,
		const std::string &name, uint32_t mask,
		uint8_t shift, uint8_t mode, const std::string &desc,
		uint8_t signess, uint8_t nfb, double iniVal)
{
	this->pReg=pReg;
	this->name=name;
	this->mask=mask;
	this->shift=shift;
	this->mode=mode;
	this->type=(signess & WBF_TM_SIGNESS);
	if(nfb>0) this->type|=WBF_TM_FIXED_POINT;
	this->nfb=nfb;
	this->desc=desc;
	this->forceSync=false;
	this->checkOverflow=true;

	TRACE_P_DEBUG("%s type=0x%0x nfb=%d, dVal=%f",name.c_str(),type,nfb,iniVal);
	int i = 1;
	for (; mask; mask >>= 1, i++)
		this->width=i-shift;


	if(pReg)
	{
		if(isinf(iniVal)) pReg->addField(this);
		else
		{
			float dVal32=(float)iniVal;
			this->convert(&dVal32,false);
			pReg->addField(this,true);
		}
	}
}

/**
 * Empty destructor
 */
WBField::~WBField()
{

}



/**
 * Generic function to convert an integer value to/from a reg_data
 *
 * Simple transformation using the bit mask and the shift to transform the value.
 *
 * \warning In this transformation the sign is not taken in account.
 * \note This is a constant function so we do not modify any internal data of WBField and we don't need a valid linked WBReg.
 * \param[inout] value 		pointer to an integer value
 * \param[inout] reg_data 	pointer to a register data
 * \param[in] to_value if true, value will be out and reg_data in, when false this is swapped.
 * \return true if the operation success, false otherwise.
 */
bool WBField::regCvt(uint32_t *value, uint32_t *reg_data, bool to_value) const
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
 * 		- WBField::WBF_32U  truncate to u32 and use the integer \ref regCvt() function
 * 		- WBField::WBF_32FP convert using signed fixed point conversion
 * 		- WBField::WBF_32F2C convert using 2 complements fixed point conversion so that
 * 		we only use 1bit for zero. If \ref isOverflowPrevented() we limit the value to out bit width.\n
 * 		More info: http://en.wikipedia.org/wiki/Two%27s_complement
 *
 *
 *
 * \note This is a constant function so we do not modify any internal data of WBField and we don't need a valid linked WBReg.
 * \param[inout] value 		pointer to an integer value
 * \param[inout] reg_data 	pointer to a register data
 * \param[in] to_value if true, value will be out and reg_data in, when false this is swapped.
 * \return true if the operation success, false otherwise.
 */
bool WBField::regCvt(float *value, uint32_t *reg_data, bool to_value) const
{
	bool ret=false;
	uint32_t fixed, utmp;
	float ftmp;
	switch(type)
	{
	case WBF_32U:
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			*value=(float)fixed;
		}
		else
		{
			fixed=(uint32_t)round(*value);
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case WBF_TM_FIXED_POINT: //Unsigned Fixed point conversion
	case WBF_32FP: 			//Signed Fixed point conversion
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			*value=(float)(fixed/pow(2,this->nfb));
		}
		else
		{
			fixed=(uint32_t)(round(*value * pow(2,this->nfb)));
			ret=this->regCvt(&fixed,reg_data,to_value);
		}
		break;
	case WBF_32F2C: //2 complements fixed point conversion
		if(to_value)
		{
			ret=this->regCvt(&fixed,reg_data,to_value);
			if (fixed & (1 << (this->width-1)))
			{
				fixed=((~fixed)+1) & (this->mask >> this->shift) ; //Convert negative 2C to negative Fixed Point
				*value=(-1.f / (float)(1<<this->nfb)) * (float)fixed; // then to floating
			}
			else
				*value= (1.f / (float)(1<<this->nfb)) * (float)fixed;			//Convert directly Fixed Point to Floating Point
		}
		else
		{
			ftmp=fabs(*value);
			if(checkOverflow)
			{
				utmp=1 << ((this->width-this->nfb)-1);
				if(ftmp >= utmp) ftmp=(float)utmp;
			}
			fixed=round(ftmp * (float)(1 << this->nfb)); //convert to signed fixed point using absolute value
			if(*value<0) fixed=(~(fixed))+1; 				//convert absolute signed fixed point to 2C fixed point when value <0
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
 * This function will use the corresponding bit stored in the associate WBReg
 * and will convert them to/from the given pVal. A valid WBReg must be linked to
 * this object.
 *
 * \see Call \ref regCvt() method using WBReg::data as reg_data.
 * \param[inout] pVal pointer to a value
 * \param[in] to_value if true, value will be out. Otherwise it will be in.
 * \return true if the operation success, false otherwise.
 */
bool WBField::convert(uint32_t *pVal, bool to_value)
{
	return (pReg)?regCvt(pVal,&(pReg->data),to_value):false;
}

/**
 * Shortcut function for converting from/to float value
 *
 * This function will use the corresponding bit stored in the associate WBReg
 * and will convert them to/from the given pVal.
 *
 * \see Call \ref regCvt() method using WBReg::data as reg_data.
 * \param[inout] pVal pointer to a value
 * \param[in] to_value if true, value will be out. Otherwise it will be in.
 * \return true if the operation success, false otherwise.
 */
bool WBField::convert(float *pVal, bool to_value)
{
	return (pReg)?regCvt(pVal,&(pReg->data),to_value):false;
}
/**
 * Shortcut to setup the register to be sync with the device ASAP.
 *
 * \note Once the corresponding register flag toSync is true it is not possible to reset
 * it unless calling to WBReg::sync() method. Calling to WBField::sync() will not reset this
 * flag.
 *
 * \return true if everything okay. otherwirse false if pointer on register is unvalid.
 */
bool WBField::setToSync()
{
	TRACE_CHECK_PTR(pReg,false);
	pReg->toSync=true;
	return true;
}

/**
 * Shortcut function to return a float from the data in the field
 */
float WBField::getFloat() const
{
	float val=0;
	if(pReg) regCvt(&val,&(pReg->data),true);
	return val;
}

/**
 * Shortcut function to return a uint32_t from the data in the field
 */
uint32_t WBField::getU32() const
{
	uint32_t val=0;
	if(pReg) regCvt(&val,&(pReg->data),true);
	return val;
}

/**
 * Synchronize only the WBField bits with memory
 *
 * This function has a mechanism that only write and read
 * on the desired WBField.
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
bool WBField::sync(WBMemCon *con, WBAccMode amode)
{
	bool ret=true;
	uint32_t oldval, value;

	//Perform some check
	if(pReg==NULL) return false;
	if(con==NULL && !con->isValid()) return false;

	//first write to dev
	if(amode & WB_AM_W)
	{
		//Get current value
		ret &=con->mem_access(pReg->getOffset(true),&oldval,false); //Read WB from dev
		value=(pReg->data & mask) | (oldval & ~mask); //Update only our field
		TRACE_P_DEBUG("%-10s (@0x%08X) ret=%d old=0x%x new=0x%x",getCName(),pReg->getOffset(true),ret,oldval,value);
		if(oldval != value || forceSync)
		{
			ret &=con->mem_access(pReg->getOffset(true),&value,true); //Write WB to dev
			TRACE_P_DEBUG("%-10s (@0x%08X) ret=%d value=0x%0x",getCName(),pReg->getOffset(true),ret,value);
		}
	}
	//then read from dev
	if(amode & WB_AM_R)
	{
		ret &=con->mem_access(pReg->getOffset(true),&value,false); //Read WB from dev
		pReg->data = (pReg->data & ~mask) | (value & mask); //update only our field
	}

	return ret;
}

/**
 * operator that print the data of the WBField in a stream
 */
std::ostream & operator<<(std::ostream & o, const WBField &f)
{
	o << string_format("0x%08X ",f.mask) << f.name;
	o << " (" << ((f.mode & WB_AM_R)?"R":"") << ((f.mode & WB_AM_W)?"W":"") << ")";
	if(f.type==WBField::WBF_32F2C)
		o << " FixedPoint with 2comp (nfb=" << std::dec << (int)f.nfb << ")";
	return o;
}


//========================================================================

/**
 * Constructor of a WBReg
 *
 * \param[in] pPrtNode if valid, we will append this register to the WBNode.
 * \param[in] name The name of the register
 * \param[in] offset The WB address offset respective to the peripheral (WBNode)
 * \param[in] desc a description of what does this registers in case it is needed.
 */
WBReg::WBReg(WBNode *pPrtNode,const std::string &name, uint32_t offset, const std::string &desc)
{
	this->pPrtNode=pPrtNode;
	this->name=name;
	this->desc=desc;

	this->offset=offset;
	this->used_mask=0;
	this->data=0;
	this->toSync=false;

	pPrtNode->appendReg(this);

	//TRACE_DEBUG("WBNode::WBNode() %s (%x, %x)\n",this->name.c_str(),&this->name,this);
}

/**
 * Destructor of WBReg
 *
 * It also delete all the fields associate to this WBReg.
 */
WBReg::~WBReg()
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
bool WBReg::addField(WBField *fld, bool toSyncInit)
{
	const WBReg* fld_reg=fld->getReg();
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

	//Append the field to the vector
	fields.push_back(fld);

	//append field mask to used mask of the whole register
	used_mask|=fld->getMask();
	this->toSync|=toSyncInit;

	return true;
}

/**
 * Get a pointer on the corresponding field
 *
 * \return A pointer on WBField or NULL if it was not found
 */
const WBField* WBReg::getField(const std::string& name) const
{
	for(size_t j=0;j<fields.size();j++)
	{
		if(fields[j]->getName()==name) return fields[j];
	}
	return NULL;
}

/**
 * Synchronize the 32bits of the WBReg
 *
 * \note This method is also the only way to reset the toSync flag
 *
 * \param[in] con A pointer to a valid Memory Connector object.
 * \param[in] amode Access mode (R, W, R/W)
 * \return true if everything is okay, false otherwise
 */
bool WBReg::sync(WBMemCon* con, WBAccMode amode)
{
	bool ret=true;
	if(con==NULL) return false;

	//first write to dev
	if(amode & WB_AM_W)
	{
		ret &= con->mem_access(this->getOffset(true),&data,true); //Write WB to dev
	}
	//then read from dev
	if(amode & WB_AM_R)
	{
		ret &= con->mem_access(this->getOffset(true),&data,false); //Read WB from dev
	}
	if(toSync) toSync=(ret==false); //Keep trying to sync if return was false
	return ret;
}

/**
 * Return the offset of the register
 *
 * \param[in] absolute if true the offset is the absolute direction in relation with the WBNode.
 *
 */
uint32_t WBReg::getOffset(bool absolute) const
{
	if(absolute && pPrtNode)
	{
		return pPrtNode->getAddress()+offset;
	}
	return offset;
}

/**
 * operator that print the data of the WBReg in a stream
 */
std::ostream & operator<<(std::ostream & o, const WBReg &r)
{
	o << string_format("@0x%08X (%s) : 0x%x",r.getOffset(true),r.getCName(),r.getData());
	return o;
}

//========================================================================

int WBNode::sID=0;



/**
 * Constructor for the Root WBNode of the tree structure (ID=0)
 */
WBNode::WBNode(const std::string &name, uint32_t addr, const std::string &desc)
{
	this->name=name;
	this->desc=desc;

	this->address=addr;
	this->is_root=true;
	this->parent=NULL;

	this->sID=0;
	this->ID=sID++;

	TRACE_P_INFO("%s (%d) => @0x%08X \t (p=%s)",name.c_str(),ID,addr,(parent)?parent->getCName():"NULL");
}


/**
 * Constructor for a WBNode
 *
 * \param[in] parent A father (or root) WBNode
 * \param[in] name The name of the WBNode
 * \param[in] addr The absolute wishbone address of this peripheral on the device
 * \param[in] desc A description of this peripheral (optional)
 */
WBNode::WBNode(WBNode *parent,const std::string &name, uint32_t addr, const std::string &desc)
{
	this->parent=parent;
	this->name=name;
	this->desc=desc;

	this->address=addr;
	this->is_root=false;

	this->ID=sID++;

	if(this->parent) //TODO: Check if children already exist
		this->parent->children.push_back(this);

	TRACE_P_INFO("%s (%d) => @0x%08X \t (p=%s)",name.c_str(),ID,addr,(parent)?parent->getCName():"NULL");
}

/**
 * Destructor of WBNode
 *
 * You should only call the root destructor.
 * Then it will call itself all the children and register destructor.
 */
WBNode::~WBNode() {
	for(std::map<uint32_t,WBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
	{
		if((*ii).second) delete ((*ii).second);
	}

	for(size_t j=0;j<children.size();j++)
	{
		if(children[j]) delete children[j];
	}
}

/**
 * Simply add a WBReg to the WBNode structure
 */
void WBNode::appendReg(WBReg *pReg)
{
	if(pReg) registers.insert(std::pair<uint32_t,WBReg*>(pReg->getOffset(),pReg));
}

/**
 * Get the WBReg at a particular offset
 *
 * \return the WBReg or NULL if the offset is not correct
 */
WBReg* WBNode::getReg(uint32_t offset) const
{
	std::map<uint32_t,WBReg*>::const_iterator ii;
	ii=registers.find(offset);
	TRACE_CHECK_VA(ii!=registers.end(),NULL,"offset 0x%08x does not exist in %s",
			offset,this->getCName());
	return (ii->second);
}


/**
 * Quick way to iterate over the list of WBReg.
 *
 * We suggest to use this methods in the following way:
 * \code
 * 		WBReg * pReg=NULL;
 * 		while((pReg=pNode->getNextReg(pReg))!=NULL)
 * 		{
 * 			//Your code for the current pReg.
 * 		}
 * \endcode
 *
 * \note: This function will not properly work on multi-thread as the iterator belong to the object and not the thread.
 *
 * \param[in] prev If NULL the iteration start from the beginning, otherwise we check if iteration is the same as the given WBReg*
 * \return  A pointer on the WBReg at next iteration
 */
WBReg* WBNode::getNextReg(WBReg* prev)
{
	if(prev==NULL) ii_nxtreg=registers.begin();
	if(prev==(*ii_nxtreg).second) ++ii_nxtreg;
	return (ii_nxtreg==registers.end())?NULL:(*ii_nxtreg).second;
}

/**
 * Sync all registers in this WBNode with the devices
 *
 * This method just iterate over each register to
 * call their sync() method.
 *
 * \ref WBReg::sync()
 *
 * \param[in] con   An abstract class to connect to the memory.
 * \param[in] amode The operation mode (R,W,RW)
 * \return true if everything ok, false otherwise.
 */
bool WBNode::sync(WBMemCon* con, WBAccMode amode) {

	bool ret=true;
	if(con==NULL) return false;
	for(std::map<uint32_t,WBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
	{
		WBReg* pReg = (*ii).second;
		ret &= pReg->sync(con,amode);
	}
	return ret;
}

/**
 * Sync WBNode using DMA buffer
 *
 * To write/read in a fast way we can use the DMA memory connection.
 *   - In write mode: we fill all our WBNode structure into a buffer that is
 *   send to the device at a specified offset.
 *   - In read mode: we get a specific piece of memory using DMA and we then
 *   extract the value from the buffer to the WBNode structure
 *
 * \param[in] con   An abstract class to connect to the memory.
 * \param[in] amode The operation mode (R,W,RW)
 * \param[in] dma_dev_offset The position on the device where we are going to
 *   perform the R/W. By setting WB_NODE_DMA_OWNADDR, the address define for
 *   the node is used.
 * \return true if everything ok, false otherwise.
 *
 */
bool WBNode::sync(WBMemCon* con, WBAccMode amode, uint32_t dma_dev_offset)
{
	bool ret=true;
	uint32_t *pData32, prh_bsize, ker_bsize;
	TRACE_CHECK_PTR(con,false);
	if(dma_dev_offset==WB_NODE_MEMBCK_OWNADDR) dma_dev_offset=address;

	//Check if the latest register has the latest size.
	prh_bsize=getLastReg()->getOffset()+4;

	TRACE_P_DEBUG("%s 0x%08X + [0x0,0x%X] (DMA sync)",getCName(),dma_dev_offset,getLastReg()->getOffset());

	//first write to dev
	if(amode & WB_AM_W)
	{
		//Get the internal to_dev buffer
		ker_bsize=con->get_block_buffer(&pData32,true);
		if(prh_bsize>ker_bsize) return false;

		//Fill it with the data of all registers
		for(std::map<uint32_t,WBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
		{
			pData32[(*ii).first/sizeof(uint32_t)]=((*ii).second)->getData();
		}

		//send it to the device
		ret &= con->mem_block_access(dma_dev_offset,prh_bsize,true); //Write buffer to dev
	}

	//then read from dev
	if(amode & WB_AM_R)
	{
		//fill the user space buffer from the memory device
		ret &= con->mem_block_access(dma_dev_offset,prh_bsize,false); //Read buffer from dev

		//Get the internal from_dev kernel buffer
		ker_bsize=con->get_block_buffer(&pData32,false);
		TRACE_CHECK_VA(prh_bsize<=ker_bsize,false,"size of periph is %d bytes (max=%d)",prh_bsize,ker_bsize);

		//Extract each value to the corresponding register
		for(std::map<uint32_t,WBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
		{
			((*ii).second)->data=pData32[(*ii).first/sizeof(uint32_t)];
			TRACE_P_VDEBUG("%20s @0x%08X (%02d) <= 0x%x",((*ii).second)->getCName(),
					((*ii).second)->getOffset(true),(*ii).first/sizeof(uint32_t),
					((*ii).second)->getData());
		}

	}
	return ret;
}
/**
 * Sync WBNode using internal memory
 *
 * This method is similar as WBNode::sync(WBMemCon,WBAccMode,uint32_t) method
 * except that here have direct access to the internal/kernel buffer that is going
 * to be used with the WBMemCon.
 *
 * If we want to partially sync the memory with our WBNode this is the method
 * to use. For example if we want to read only the the words (32bits) 0x10 to 0x16
 * from the internal buffer to our WBNode we should perform the following:
 *
 * <code>
 * uint32_t *pData32, r_bsize;
 * //First setup the internal buffer from the device
 * pCon->mem_block_access(pNode->getAddress(),pNode->getLastReg()->getOffset()/sizeof(uint32_t),false);
 * r_bsize=pCon->get_block_buffer(&pData32,false);
 * //Then read from the internal buffer to the WBNode only words [0x10-0x16]
 * pNode->sync(pData32,0x6,WB_AM_R,0x10);
 * </code>
 *
 * \param[in] pData32  Pointer on an internal buffer.
 * \param[in] bsize    Size of the buffer that we want to R/W in bytes
 * \param[in] amode    The operation mode (R,W,RW)
 * \param[in] doffset  Offset on the data buffer and its correspondence in registers of WBNode (bytes).
 * \return true if everything ok, false otherwise.
 *
 */
bool WBNode::sync(uint32_t* pData32, uint32_t bsize, WBAccMode amode,  uint32_t doffset)
{
	bool ret=true;
	uint32_t prh_bsize;
	TRACE_CHECK_PTR(pData32,false);

	//Check if the latest register has the latest size.
	prh_bsize=getLastReg()->getOffset()+4;
	bsize=std::min(prh_bsize-doffset,bsize);
	TRACE_CHECK_VA((doffset+4)<=prh_bsize,false,
			"Size overflow: offset=%d+4 > prh_bsize=%d",
			doffset,prh_bsize);

	TRACE_P_DEBUG("%s 0x%08X + [0x%x,0x%X] (DataBuff)",getCName(),(uint32_t)pData32,doffset,doffset+bsize);

	//first write to buffer
	if(amode & WB_AM_W)
	{
		//Fill it with the data of all registers
		for(std::map<uint32_t,WBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
		{
			if(doffset <= (*ii).first && (*ii).first <= doffset+bsize)
				pData32[(*ii).first/sizeof(uint32_t)]=((*ii).second)->getData();
		}
	}

	//then read from buffer
	if(amode & WB_AM_R)
	{

		//Extract each value to the corresponding register
		for(std::map<uint32_t,WBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
		{
			if(doffset <= (*ii).first && (*ii).first <= doffset+bsize)
			{
				((*ii).second)->data=pData32[(*ii).first/sizeof(uint32_t)];
				TRACE_P_VDEBUG("%20s @0x%08X (%02d) <= 0x%x",((*ii).second)->getCName(),
						((*ii).second)->getOffset(true),(*ii).first/sizeof(uint32_t),
						((*ii).second)->getData());
			}
		}

	}
	return ret;
}


/**
 * Print the WBNode in a cpp stream
 *
 * It will print all the WBReg and their corresponding WBField of this node
 * and then print the children WBNode.
 *
 * \param[inout] o the stream that will be modified
 * \param[in] level The indentation of the print change according to level
 */
void WBNode::print(std::ostream & o, int level) const
{
	int i;
	char pre[255];
	for(i=0;i<level;i++) pre[i]='\t';
	pre[i]='\0';

	o << pre << "Node: " << this->name << string_format(" @0x%08X (ID=%d)",this->address,this->ID) << std::endl;

	WBReg * reg=NULL;
	for(std::map<uint32_t,WBReg*>::const_iterator ii=this->registers.begin(); ii!=this->registers.end(); ++ii)
	{
		if((*ii).second==NULL) continue;
		else reg=(*ii).second;

		o << pre << "   " << *reg << std::endl;

		const std::vector<WBField*> f = reg->getFields();
		for(size_t j=0;j<f.size();j++)
		{
			if(f[j]==NULL) continue;
			o << pre << "     " << *(f[j]) << std::endl;
		}
	}

	level++;
	for(size_t i=0;i<this->children.size();i++)
		if(this->children[i])this->children[i]->print(o,level);

	o << std::endl;
}





