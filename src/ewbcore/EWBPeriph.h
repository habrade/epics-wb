/*
 * EWBPeriph.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBPERIPH_H_
#define EWBPERIPH_H_

#include "EWBSync.h"
#include "EWBBus.h"

#include <map>

//Forward declaration to improve compilation
class EWBBridge;
class EWBReg;

#define EWB_NODE_MEMBCK_OWNADDR 0xFFFFFFFF //!< Used by WBNode::sync()

#define WB2_PRH_ARGS(pname,offset) \
	WB2_##pname##_PERIPH_PREFIX, \
	offset, \
	WB2_##pname##_PERIPH_VENID, \
	WB2_##pname##_PERIPH_DEVID, \
	WB2_##pname##_PERIPH_DESC


/**
 * Class that represent a WB peripheral in a tree structure.
 *
 * 		- A list EWBReg is also linked to this peripheral
 */
class EWBPeriph: public EWBSync {
public:
	EWBPeriph(EWBBus *bus,const std::string &name, uint32_t offset, uint64_t venID, uint32_t devID, const std::string &desc="");
	virtual ~EWBPeriph();

	bool appendReg(EWBReg *pReg);
	EWBReg* getReg(uint32_t offset) const;
	EWBReg* getNextReg(EWBReg *prev);
	EWBReg* getLastReg() const { return (registers.size()>0)?registers.rbegin()->second:NULL; }	//!< Get the highest WBReg in the node.

	bool sync(EWBSync::AMode amode=EWB_AM_RW);
//	bool sync(EWBSync::AMode amode, uint32_t dma_dev_offset=WB_NODE_MEMBCK_OWNADDR);
//	bool sync(uint32_t* pData32, uint32_t length, EWBSync::AMode amode, uint32_t doffset=0);

	bool isValid(int level=-1) const { return (level!=0)?(bus && bus->isValid(level-1)):bus!=NULL; } 	//!< Return true when all pointers are defined
	bool isID(uint64_t venID, uint32_t devID) const { return (venID==this->venID && devID==this->devID); }

	uint32_t getDeviceID() const { return this->devID; }	//!< Get the Device ID of this WBPeriph
	uint64_t getVendorID() const { return this->venID; }	//!< Get the Device ID of this WBPeriph
	int getIndex() const { return this->index; }	//!< Get unique index of WBPeriph
	const std::string& getName() const { return this->name; }	//!< Get the name
	const char *getCName() const { return this->name.c_str(); }	//!< Get the name in "C" format for printf function
	const std::string& getDesc() const { return this->desc; }	//!< Get the description
	const EWBBridge* getBridge() const { return (bus)?bus->getBridge():0; }
	EWBBridge* getBridge()  { return (bus)?bus->getBridge():0; }

	uint32_t getOffset(bool absolute) const;
	void print(std::ostream & o, int level=0) const;
	friend std::ostream & operator<<(std::ostream & o, const EWBPeriph &p) { p.print(o); return o; } //!< \ref print()

protected:
	std::string name;	//!< Name of the peripheral node
	std::string desc;	//!< Description of the peripheral node
	uint32_t offset;	//!< Address of the peripheral node
	uint32_t devID;		//!< Device ID (SDB) of this peripheral
	uint64_t venID;		//!< Vendor ID (SDB) of this peripheral

private:

	EWBBus *bus;
	static int sCount;
	int index;
	std::map<uint32_t,EWBReg*> registers;
	std::map<uint32_t,EWBReg*>::iterator ii_nxtreg;
};


#endif /* EWBPERIPH_H_ */
