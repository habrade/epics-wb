/**
 *  \file
 *  \brief Contains the class asynWBPortDrvr
 *
 *  \ref AsynWBSync
 *
 *  \date  Oct 28, 2013
 *  \author Benoit Rat
 */


#ifndef ASYNWBORTDRVR_H_
#define ASYNWBORTDRVR_H_

#include <string>
#include <map>

#include "WBNode.h"
#include <asynPortDriver.h>

//! Type of synchronization between the memory, Wishbone tree and Process variable
enum AsynWBSync {
	AWB_SYNC_DEVICE=0,	//!< Sync to/from the device using WBMemCon on the field
	AWB_SYNC_WBSTRUCT,	//!< Sync to/from the WBNode (no access to device)
	AWB_SYNC_PRMLIST,	//!< Sync quickly on paramList using Get/Set (no access to device)
	AWB_SYNC_DERIVED,	//!< Sync must be performed on the children class.
};

struct AsynWBField {
	WBField* pFld;
	int syncmode;
};

/**
 * Structure that overload standard operator of asynStatus enum in order to ease its manipulation.
 */
struct AsynStatusObj {
	asynStatus data;
	AsynStatusObj(): data(asynSuccess) {};
	AsynStatusObj(asynStatus data): data(data) {};
	AsynStatusObj& operator&=(bool data) { if(data==false) {this->data=asynError;} return *this; }
	AsynStatusObj& operator&=(const asynStatus &data) { if(data!=asynSuccess) {this->data=data;} return *this; }
	AsynStatusObj& operator=(const asynStatus &data)  { this->data=data; return *this; }
	bool operator==(asynStatus data) const { return (this->data==data); }
	asynStatus operator &() const { return this->data; };
	operator asynStatus() const { return this->data; };
};


/**
 * The generic abstract AsynWBPortDrv class
 *
 * This class has been designed to ease the development of a real application class.
 * It provide the method to synchronize easily from/to a wishbone-able device and the process
 * variable from EPICS.
 *
 * It mimic part of the asynPortDriver but it also includes the synchronize to the device
 * in an automatic mode if necessary
 *
 * \note In the children class you must redefine the setup() method.
 * There you can create all the parameters need using the WBNode tree structure
 * and also instantiate a memory connector.
 *
 *  \ref AsynWBSync
 */
class asynWBPortDrvr : public asynPortDriver {
public:
	asynWBPortDrvr(const char *portName, int max_nprm);
	virtual ~asynWBPortDrvr();
	virtual asynStatus setup()=0;	//!< Need to be overriden

	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
	virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);

    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,size_t *nActual);
    virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);

    bool isValid() { return pRoot!=NULL; } //!< return true if the child class has been properly setup()

protected:
    asynStatus createParam(WBField *fld, int *index=NULL,int syncmode=AWB_SYNC_DEVICE);
    asynStatus createParam(const char *name, WBField *fld, int *index=NULL, int syncmode=AWB_SYNC_DEVICE);
    asynStatus createParam(const char *name, asynParamType type,int *index=NULL,int syncmode=AWB_SYNC_PRMLIST);
    bool cvtWBNodetoPrmList(WBNode *node);
    int getParamIndex(const char *name);

    WBNode *pRoot;			//!< pointer on the WB tree structure.
    WBMemCon* pMemCon;		//!< generic pointer on the memory connector.

private:
    std::vector<AsynWBField> fldPrms;
    std::string driverName;
};

#endif
