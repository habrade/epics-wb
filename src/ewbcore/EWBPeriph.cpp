/*
 * EWBPeriph.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBPeriph.h"

#include "EWBTrace.h"
#include "EWBField.h"
#include "EWBReg.h"
#include "EWBBus.h"


#include <string>
#include <vector>


int EWBPeriph::sCount=0;


/**
 * Constructor for a EWBPeriph
 *
 * \param[in] bus The bus to access to the device
 * \param[in] name The name of the EWBPeriph
 * \param[in] addr The absolute wishbone address of this peripheral on the device
 * \param[in] desc A description of this peripheral (optional)
 */
EWBPeriph::EWBPeriph(EWBBus *bus,const std::string &name, uint32_t offset, uint64_t venID, uint32_t devID, const std::string &desc)
: EWBSync(EWB_AM_RW)
{
	this->bus=bus;
	this->name=name;
	this->desc=desc;

	this->offset=offset;

	this->venID=venID;
	this->devID=devID;
	this->index=sCount++;


	TRACE_P_INFO("%s (%4x:%08x) => @0x%08X",name.c_str(),(uint32_t)venID,devID,offset);
}

/**
 * Destructor of EWBPeriph
 *
 * You should only call the root destructor.
 * Then it will call itself all the children and register destructor.
 */
EWBPeriph::~EWBPeriph() {
	EWBReg *r;
	for(std::map<uint32_t,EWBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
	{
		r=(*ii).second;
		if(r) delete r;
	}
}

/**
 * Simply add a EWBReg to the EWBPeriph structure
 */
bool EWBPeriph::appendReg(EWBReg *pReg)
{
	if(pReg) {
		std::pair<std::map<uint32_t,EWBReg*>::iterator,bool> ret;
		ret = registers.insert(std::pair<uint32_t,EWBReg*>(pReg->getOffset(),pReg));
		TRACE_CHECK_VA(ret.second,false,"Could not append '%s' because offset @x%0x is already used by '%s'",
				pReg->getCName(),pReg->getOffset(),ret.first->second->getCName());
		return ret.second;
	}
	return false;
}

/**
 * Get the EWBReg at a particular offset
 *
 * \return the EWBReg or NULL if the offset is not correct
 */
EWBReg* EWBPeriph::getReg(uint32_t offset) const
{
	std::map<uint32_t,EWBReg*>::const_iterator ii;
	ii=registers.find(offset);
	TRACE_CHECK_VA(ii!=registers.end(),NULL,"offset 0x%08x does not exist in %s",
			offset,this->getCName());
	return (ii->second);
}


/**
 * Quick way to iterate over the list of EWBReg.
 *
 * We suggest to use this methods in the following way:
 * \code
 * 		EWBReg * pReg=NULL;
 * 		while((pReg=pNode->getNextReg(pReg))!=NULL)
 * 		{
 * 			//Your code for the current pReg.
 * 		}
 * \endcode
 *
 * \note: This function will not properly work on multi-thread as the iterator belong to the object and not the thread.
 *
 * \param[in] prev If NULL the iteration start from the beginning, otherwise we check if iteration is the same as the given EWBReg*
 * \return  A pointer on the EWBReg at next iteration
 */
EWBReg* EWBPeriph::getNextReg(EWBReg* prev)
{
	if(prev==NULL) ii_nxtreg=registers.begin();
	if(prev==(*ii_nxtreg).second) ++ii_nxtreg;
	return (ii_nxtreg==registers.end())?NULL:(*ii_nxtreg).second;
}


/**
 * Return the offset of the peripheral.
 *
 * @param absolute when @true the offset is absolute otherwise
 * it is relative to the EWBBus base offset.
 */
uint32_t EWBPeriph::getOffset(bool absolute) const
{
	if(absolute)
	{
		TRACE_CHECK(isValid(),0,"EWBPeriph is not valid");
		return bus->getOffset()+offset;
	}
	else
	{
		return offset;
	}
}

/**
 * Sync all registers in this EWBPeriph with the devices
 *
 * This method just iterate over each register to
 * call their sync() method.
 *
 * \ref EWBReg::sync()
 *
 * \param[in] con   An abstract class to connect to the memory.
 * \param[in] amode The operation mode (R,W,RW)
 * \return true if everything ok, false otherwise.
 */
bool EWBPeriph::sync(EWBSync::AMode amode) {

	bool ret=true;
	for(std::map<uint32_t,EWBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
	{
		EWBReg* pReg = (*ii).second;
		ret &= pReg->sync(amode);
	}
	return ret;
}

///**
// * Sync EWBPeriph using DMA buffer
// *
// * To write/read in a fast way we can use the DMA memory connection.
// *   - In write mode: we fill all our EWBPeriph structure into a buffer that is
// *   send to the device at a specified offset.
// *   - In read mode: we get a specific piece of memory using DMA and we then
// *   extract the value from the buffer to the EWBPeriph structure
// *
// * \param[in] con   An abstract class to connect to the memory.
// * \param[in] amode The operation mode (R,W,RW)
// * \param[in] dma_dev_offset The position on the device where we are going to
// *   perform the R/W. By setting EWB_NODE_DMA_OWNADDR, the address define for
// *   the node is used.
// * \return true if everything ok, false otherwise.
// *
// */
//bool EWBPeriph::sync(EWBSync::AMode amode, uint32_t dma_dev_offset)
//{
//	bool ret=true;
//	uint32_t *pData32, prh_bsize, ker_bsize;
//	TRACE_CHECK_PTR(con,false);
//	if(dma_dev_offset==EWB_NODE_MEMBCK_OWNADDR) dma_dev_offset=offset;
//
//	//Check if the latest register has the latest size.
//	prh_bsize=getLastReg()->getOffset()+4;
//
//	TRACE_P_DEBUG("%s 0x%08X + [0x0,0x%X] (DMA sync)",getCName(),dma_dev_offset,getLastReg()->getOffset());
//
//	//first write to dev
//	if(amode & EWB_AM_W)
//	{
//		//Get the internal to_dev buffer
//		ker_bsize=con->get_block_buffer(&pData32,true);
//		if(prh_bsize>ker_bsize) return false;
//
//		//Fill it with the data of all registers
//		for(std::map<uint32_t,EWBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
//		{
//			pData32[(*ii).first/sizeof(uint32_t)]=((*ii).second)->getData();
//		}
//
//		//send it to the device
//		ret &= con->mem_block_access(dma_dev_offset,prh_bsize,true); //Write buffer to dev
//	}
//
//	//then read from dev
//	if(amode & EWB_AM_R)
//	{
//		//fill the user space buffer from the memory device
//		ret &= con->mem_block_access(dma_dev_offset,prh_bsize,false); //Read buffer from dev
//
//		//Get the internal from_dev kernel buffer
//		ker_bsize=con->get_block_buffer(&pData32,false);
//		TRACE_CHECK_VA(prh_bsize<=ker_bsize,false,"size of periph is %d bytes (max=%d)",prh_bsize,ker_bsize);
//
//		//Extract each value to the corresponding register
//		for(std::map<uint32_t,EWBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
//		{
//			((*ii).second)->data=pData32[(*ii).first/sizeof(uint32_t)];
//			TRACE_P_VDEBUG("%20s @0x%08X (%02d) <= 0x%x",((*ii).second)->getCName(),
//					((*ii).second)->getOffset(true),(*ii).first/sizeof(uint32_t),
//					((*ii).second)->getData());
//		}
//
//	}
//	return ret;
//}
///**
// * Sync EWBPeriph using internal memory
// *
// * This method is similar as EWBPeriph::sync(EWBMemCon,EWBSync::AMode,uint32_t) method
// * except that here have direct access to the internal/kernel buffer that is going
// * to be used with the EWBMemCon.
// *
// * If we want to partially sync the memory with our EWBPeriph this is the method
// * to use. For example if we want to read only the the words (32bits) 0x10 to 0x16
// * from the internal buffer to our EWBPeriph we should perform the following:
// *
// * <code>
// * uint32_t *pData32, r_bsize;
// * //First setup the internal buffer from the device
// * pCon->mem_block_access(pNode->getAddress(),pNode->getLastReg()->getOffset()/sizeof(uint32_t),false);
// * r_bsize=pCon->get_block_buffer(&pData32,false);
// * //Then read from the internal buffer to the EWBPeriph only words [0x10-0x16]
// * pNode->sync(pData32,0x6,EWB_AM_R,0x10);
// * </code>
// *
// * \param[in] pData32  Pointer on an internal buffer.
// * \param[in] bsize    Size of the buffer that we want to R/W in bytes
// * \param[in] amode    The operation mode (R,W,RW)
// * \param[in] doffset  Offset on the data buffer and its correspondence in registers of EWBPeriph (bytes).
// * \return true if everything ok, false otherwise.
// *
// */
//bool EWBPeriph::sync(uint32_t* pData32, uint32_t bsize, EWBSync::AMode amode,  uint32_t doffset)
//{
//	bool ret=true;
//	uint32_t prh_bsize;
//	TRACE_CHECK_PTR(pData32,false);
//
//	//Check if the latest register has the latest size.
//	prh_bsize=getLastReg()->getOffset()+4;
//	bsize=std::min(prh_bsize-doffset,bsize);
//	TRACE_CHECK_VA((doffset+4)<=prh_bsize,false,
//			"Size overflow: offset=%d+4 > prh_bsize=%d",
//			doffset,prh_bsize);
//
//	TRACE_P_DEBUG("%s 0x%08X + [0x%x,0x%X] (DataBuff)",getCName(),(uint32_t)pData32,doffset,doffset+bsize);
//
//	//first write to buffer
//	if(amode & EWB_AM_W)
//	{
//		//Fill it with the data of all registers
//		for(std::map<uint32_t,EWBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
//		{
//			if(doffset <= (*ii).first && (*ii).first <= doffset+bsize)
//				pData32[(*ii).first/sizeof(uint32_t)]=((*ii).second)->getData();
//		}
//	}
//
//	//then read from buffer
//	if(amode & EWB_AM_R)
//	{
//
//		//Extract each value to the corresponding register
//		for(std::map<uint32_t,EWBReg*>::iterator ii=registers.begin(); ii!=registers.end(); ++ii)
//		{
//			if(doffset <= (*ii).first && (*ii).first <= doffset+bsize)
//			{
//				((*ii).second)->data=pData32[(*ii).first/sizeof(uint32_t)];
//				TRACE_P_VDEBUG("%20s @0x%08X (%02d) <= 0x%x",((*ii).second)->getCName(),
//						((*ii).second)->getOffset(true),(*ii).first/sizeof(uint32_t),
//						((*ii).second)->getData());
//			}
//		}
//
//	}
//	return ret;
//}


/**
 * Print the EWBPeriph in a cpp stream
 *
 * It will print all the EWBReg and their corresponding EWBField of this node
 * and then print the children EWBPeriph.
 *
 * \param[inout] o the stream that will be modified
 * \param[in] level The indentation of the print change according to level
 */
void EWBPeriph::print(std::ostream & o, int level) const
{
	int i;
	char pre[255];
	for(i=0;i<level;i++) pre[i]='\t';
	pre[i]='\0';

	o << pre << "Periph: " << this->name << EWBTrace::string_format(" @0x%08X (%4x:%08x #%d)",this->offset,(uint32_t)this->venID,this->devID,this->index) << std::endl;

	EWBReg * reg=NULL;
	for(std::map<uint32_t,EWBReg*>::const_iterator ii=this->registers.begin(); ii!=this->registers.end(); ++ii)
	{
		if((*ii).second==NULL) continue;
		else reg=(*ii).second;

		o << pre << "   " << *reg << std::endl;

		const std::vector<EWBField*> f = reg->getFields();
		for(size_t j=0;j<f.size();j++)
		{
			if(f[j]==NULL) continue;
			o << pre << "     " << *(f[j]) << std::endl;
		}
	}

//	level++;
//	for(size_t i=0;i<this->children.size();i++)
//		if(this->children[i])this->children[i]->print(o,level);

	o << std::endl;
}
