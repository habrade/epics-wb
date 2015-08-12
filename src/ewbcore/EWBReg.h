/*
 * EWBReg.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBREG_H_
#define EWBREG_H_

#include "EWBSync.h"
#include "EWBPeriph.h"

class EWBField;

#include <vector>

/**
 * Class to manipulate Wishbone register with various EWBField
 *
 * \ref EWBNode
 * \ref EWBField
 */
class EWBReg: public EWBSync {
public:

	friend class EWBField;
	friend class EWBPeriph;
	friend std::ostream & operator<<(std::ostream & output, const EWBReg &r);

	EWBReg(EWBPeriph *pPrtNode,const std::string &name, uint32_t offset, const std::string &desc="");
	virtual ~EWBReg();

	uint32_t getOffset(bool absolute=false) const;
	bool sync(EWBSync::AMode amode=EWB_AM_RW);

	bool addField(EWBField *fld, bool toSyncInit=false);
	const EWBField* getField(const std::string& name) const;
	const EWBField* operator[](const std::string& name) const { return this->getField(name); }

	std::vector<EWBField*> getFields() { return fields; }
	const std::vector<EWBField*> getFields() const { return fields; }	//!< Get a vector on the belonging EWBField
	const EWBPeriph* getPrtNode() const { return pPeriph; }				//!< Get the parent EWBNode

	void 	setToSync() { toSync=true; }						//!< Set this register to be sync ASAP
	bool 	isToSync() const { return toSync; }					//!< Check if the register need to be sync ASAP
	uint32_t getData() const { return data; }					//!< Get the data
	const std::string& getName() const { return this->name; }	//!< Get the name
	const char *getCName() const { return this->name.c_str(); }	//!< Get the name in "C" format for printf function
	const std::string& getDesc() const { return this->desc; }	//!< Get the description
	bool isValid(bool connected=true) const { return (pPeriph && pPeriph->isValid(connected)); }

protected:
	EWBPeriph* getPeriph() { return pPeriph; }


	std::vector<EWBField*> fields;	//!< A list of the relative EWBFields
	std::string name;		//!< The name
	std::string desc;		//!< A description
	uint32_t offset;		//!< The offset relative to EWBNode
	uint32_t data;			//!< The corresponding data
	uint32_t used_mask;		//!< The mask used by other EWBField
	bool toSync;			//!< Boolean that tell if this register need to be sync ASAP

private:
	EWBPeriph *pPeriph;	//!< Parent Peripheral

};


#endif /* EWBREG_H_ */
