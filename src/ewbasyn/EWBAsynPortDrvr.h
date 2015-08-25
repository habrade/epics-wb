/**
 *  \file
 *  \brief Contains the class asynWBPortDrvr
 *
 *  \ref AsynWBSync
 *
 *  \date  Oct 28, 2013
 *  \author Benoit Rat
 */


#ifndef EWBASYNPORTDRVR_H_
#define EWBASYNPORTDRVR_H_

#include <string>
#include <map>

#include "EWBBus.h"
#include "EWBParam.h"
#include "EWBField.h"
#include <asynPortDriver.h>

//! Type of synchronization between the memory, Wishbone tree and Process variable
enum AsynWBSync {
	AWB_SYNC_DEVICE=0,	//!< Sync to/from the device using WBMemCon on the field
	AWB_SYNC_WBSTRUCT,	//!< Sync to/from the WBNode (no access to device)
	AWB_SYNC_PRMLIST,	//!< Sync quickly on paramList using Get/Set (no access to device)
	AWB_SYNC_DERIVED,	//!< Sync must be performed on the children class.
};

struct EWBAsynPrm {
	EWBParam* pPrm;
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
class EWBAsynPortDrvr : public asynPortDriver {
public:
	EWBAsynPortDrvr(const char *portName, int max_nprm);
	virtual ~EWBAsynPortDrvr();
	virtual asynStatus setup()=0;	//!< Need to be overriden

	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
	virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);

    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,size_t *nActual);
    virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);

    bool isValid() { return pRoot!=NULL; } //!< return true if the child class has been properly setup()

protected:
    asynStatus syncPending(EWBSync::AMode amode=EWBSync::EWB_AM_RW);
    asynStatus createParam(EWBField *fld, int *index=NULL,int syncmode=AWB_SYNC_DEVICE);
    asynStatus createParam(const char *name, EWBParam *pPrm, int *index=NULL, int syncmode=AWB_SYNC_DEVICE);
    asynStatus createParam(const char *name, asynParamType type,int *index=NULL,int syncmode=AWB_SYNC_PRMLIST);

    bool setParams(EWBPeriph *pPrh);
    int getParamIndex(const char *name);

    EWBBus *pRoot;			//!< pointer on the WB root tree structure.

    std::vector<EWBAsynPrm> fldPrms;
private:
    std::string driverName;
    int P_BlkSyncIdx, syncNow;
};

#endif
