/*
 * AsynWBPortDrvr.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Benoit Rat
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsExport.h>
#include <iocsh.h>

#include "asynWBPortDrvr.h"
#include "awbpd_trace.h"
#define TRACE_P_VDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)
#define TRACE_P_VVDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)

/**
 * Constructor for the asynWBPortDrvr class.
 *
 * This class is an helper class:
 * must have a child class and call a parent class
 *
 * \param[in] portName The name of the asyn port driver to be created.
 * \param[in] max_nprm Maximum number of scope parameters
 *
 **/
asynWBPortDrvr::asynWBPortDrvr(const char *portName,int max_nprm)
: asynPortDriver(portName,
		1,
		max_nprm,
		(asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask), /* Interface mask */
		(asynInt32Mask | asynFloat64Mask | asynOctetMask), /* Interrupt mask */
		0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
		1, /* Autoconnect */
		0, /* Default priority */
		0), /* Default stack size*/
		pRoot(NULL), pMemCon(NULL), driverName(portName)
{


	//Reserve our space for param list
	AsynWBField afld;
	afld.pFld=NULL;
	afld.syncmode=AWB_SYNC_DERIVED; //when not define we derive
	fldPrms = std::vector<AsynWBField>(max_nprm,afld);

	//Create the only generic parameters to block sync or not (0: NoneBlock, 1:BlockRead, 2:BlockWrite, 3: All block)
	createParam("AWBPD_BlockSync", asynParamInt32,&P_BlkSyncIdx);
	setIntegerParam(P_BlkSyncIdx,0);
	syncNow=WB_AM_RW;
}

/**
 * Destructor for the asynWBPortDrvr class.
 *
 * It only delete the WBMemCon pointer in the case it has been
 * used.
 */
asynWBPortDrvr::~asynWBPortDrvr()
{
	fprintf(stderr,"0x%x\n", (uint32_t)this);

	if(pMemCon) delete pMemCon;
	pMemCon=NULL;

	if(pRoot) delete pRoot;
	pRoot=NULL;
}

/**
 * Synchronize parameters that have been setup internally but not sync to the peripheral
 *
 * This is use in two case:
 * 		- At initialization, when a WBField has been set with a non default value (see constructor)
 * 		 we need to send this parameters to our device so that it takes this value at the beginning of
 * 		 execution.
 * 		 - When we want to block all the parameters so that
 *
 * \param[in] amode Access mode (R, W, R/W)
 * \return Returns a asynSuccess if everything is okay.
 */
asynStatus asynWBPortDrvr::syncPending(WBAccMode amode)
{
	WBField* fld;
	WBReg *reg=NULL;
	AsynStatusObj status = asynSuccess;

	//First find which fldParams needs to be sync
	std::vector<int> tmpList;
	for(int i=0;i<fldPrms.size();i++)
	{
		fld=fldPrms[i].pFld;
		if(fld && fld->getReg() && fld->getReg()->isToSync())
		{
			tmpList.push_back(i);
		}
	}
	//Then obtain the registers of these field params
	for(int i=0;i<tmpList.size();i++)
	{
		fld=fldPrms[tmpList[i]].pFld;
		reg=(WBReg*)fld->getReg();
		//Sync them
		if(reg->isToSync())
		{
			TRACE_P_DEBUG("Syncing Reg>: %s (@0x%08X) 0x%08x",
					reg->getCName(),reg->getOffset(true),reg->getData());
			status&=reg->sync(pMemCon,amode);
			//Finally update the
		}
		if(fld->getType() && WBField::WBF_TM_FIXED_POINT)
			setDoubleParam(tmpList[i],fld->getFloat());
		else
			setIntegerParam(tmpList[i],fld->getU32());
	}

	return status;
}

/**
 * Create a asyn parameter and link it to a WB field
 *
 * With this method: the name given to the process variable will be auto-generated in the format
 * 		<reg_name>_<field_name>
 * \note If there is only one field named 'value' the generated name will only have the reg_name
 *
 * \param[in] fld  A WBField that is going to be link with the parameter
 * \param[in] syncmode Select in which mode we want to sync between PV and WBField (check \ref AsynWBSync enum).
 * \return Returns a asynSuccess if everything is okay. Otherwise asynParamAlreadyExists if the parameter already exists, or asynBadParamIndex if
 * adding this parameter would exceed the size of the parameter list and asynError is the WBField is not valid.
 * \see AsynWBSync for the type of synchronization
 */
asynStatus asynWBPortDrvr::createParam(WBField* fld,int *pIndex, int syncmode)
{
	std::stringstream ss;

	if(pIndex) *pIndex=-1;
	TRACE_CHECK_PTR(fld,asynError);
	TRACE_CHECK_PTR(fld->getReg(),asynError);

	if(fld->getName()==fld->getReg()->getName() || (fld->getName()=="value" && fld->getReg()->getFields().size()==1))
		ss << fld->getReg()->getName();
	else
		ss << fld->getReg()->getName() << "_" << fld->getName();

	this->createParam(ss.str().c_str(),fld, pIndex, syncmode);
}


/**
 * Create a asyn parameter and link it to a WB field
 *
 * \param[in] name The name of this parameter
 * \param[in] fld  A WBField that is going to be link with the parameter
 * \param[in] syncmode Select in which mode we want to sync between PV and WBField (check \ref AsynWBSync enum).
 * \return Returns a asynSuccess if everything is okay. Otherwise asynParamAlreadyExists if the parameter already exists, or asynBadParamIndex if
 * adding this parameter would exceed the size of the parameter list and asynError is the WBField is not valid.
 * \see AsynWBSync for the type of synchronization
 */
asynStatus asynWBPortDrvr::createParam(const char* name, WBField* fld,int *pIndex, int syncmode)
{
	int tmp;
	asynStatus status=asynError;
	asynParamType type;
	if(pIndex) *pIndex=-1;

	if(fld && fld->getReg() && fld->getReg()->getPrtNode())
	{
		pIndex=(pIndex)?pIndex:&tmp;
		if(fld->getType() && WBField::WBF_TM_FIXED_POINT)
			type=asynParamFloat64;
		else
			type=asynParamInt32;

		//if(list==1) list=0;

		status=asynPortDriver::createParam(name,type,pIndex);
		if(status==asynSuccess)
		{
			fldPrms[*pIndex].pFld=fld;
			fldPrms[*pIndex].syncmode=syncmode;
			if(fld->getType() && WBField::WBF_TM_FIXED_POINT)
				setDoubleParam(*pIndex,fld->getFloat());
			else
				setIntegerParam(*pIndex,fld->getU32());
		}

		TRACE_P_DEBUG("%10s#%02d: 0x%08X (msync=%d, %s) <= %s/%s_%s (PV/Fld)",
				fld->getReg()->getPrtNode()->getCName(),*pIndex,
				fld->getReg()->getOffset(true),syncmode,
				(type==asynParamInt32)?"I32":"U64",name,
						fld->getReg()->getCName(),fld->getCName());
	}
	return status;
}

/**
 * Create parameters for internal use of the IOC
 *
 * This parameters will not be synchronized with our device. This function override the standard
 * asynPortDriver::createParam(), and only add the syncmode to AWB_SYNC_PRMLIST or AWB_SYNC_DERIVED
 *
 * \param[in] name The name of this parameter
 * \param[in] type  The type of parameters A WBField that is going to be link with the parameter
 * \param[in] syncmode Select in which mode we want to sync. As we don't communicate with the device we can not set AWB_SYNC_DEVICE or AWB_SYNC_WBSTRUCT
 * \return Returns a asynSuccess if everything is okay. Otherwise asynParamAlreadyExists if the parameter already exists, or asynBadParamIndex if
 * adding this parameter would exceed the size of the parameter list and asynError is the syncmode is not valid.
 * \see AsynWBSync for the type of synchronization
 */
asynStatus asynWBPortDrvr::createParam(const char* name, asynParamType type,int *pIndex,int syncmode)
{
	int tmp;
	asynStatus status=asynError;
	if(pIndex) *pIndex=-1;

	TRACE_CHECK_VA(syncmode != AWB_SYNC_DEVICE && syncmode != AWB_SYNC_WBSTRUCT,status,
			"Bad sync mode :%d",syncmode);

	pIndex=(pIndex)?pIndex:&tmp;
	status=asynPortDriver::createParam(name,type,pIndex);
	if( status==asynSuccess)
	{
		fldPrms[*pIndex].pFld=NULL;
		fldPrms[*pIndex].syncmode=syncmode;
		TRACE_P_DEBUG("%10s#%02d: type=0x%x",name,*pIndex,type);
	}

	return status;
}

/**
 * Return the index of a parameters given its name
 *
 * This function is pretty heavy and it should be preferred to
 * keep the index as an int field of the class to access it.
 *
 * If the name was not found in the paramList it returns -1
 */
int asynWBPortDrvr::getParamIndex(const char *name)
{
	int index;
	if(asynPortDriver::findParam(name, &index)==asynSuccess) return index;
	else return -1;
}

/** Called when asyn clients call pasynInt32->read().
 *
 * For default \ref AsynWBSync Mode we perform:
 * 		- Obtain the corresponding WBField using pasynUser->reason
 * 		- Synchronize or not the WBField using the memory connector.
 * 		- Convert it from WBField to value
 * 		- Set value in the parameter library
 *
 * \param[in] pasynUser pasynUser structure that encodes the reason and address.
 * \param[out] value The value read.
 * \return asynSuccess if okay, asynDisable if derived and another error code otherwise.
 */
asynStatus asynWBPortDrvr::readInt32(asynUser* pasynUser, epicsInt32* value)
{
	int function = pasynUser->reason;
	asynStatus status = asynDisabled;
	const char *paramName;
	AsynWBField aWF = fldPrms[function];
	uint32_t u32val;

	// Fetch the parameter string name for possible use in debugging
	getParamName(function, &paramName);
	TRACE_P_VDEBUG("#%02d %s",function,paramName);

	//Derived if need
	if(aWF.syncmode==AWB_SYNC_DERIVED) return asynDisabled;

	//Sync from WB structure
	if(aWF.syncmode==AWB_SYNC_WBSTRUCT || aWF.syncmode==AWB_SYNC_DEVICE)
	{
		//Check if we have an existing association
		TRACE_CHECK_PTR(aWF.pFld,asynError);
		TRACE_CHECK_PTR(aWF.pFld->getReg(),asynError);

		//Sync from device memory
		if(aWF.syncmode==AWB_SYNC_DEVICE && (syncNow & WB_AM_R))
			aWF.pFld->sync(pMemCon,WB_AM_R);

		//Convert the WBField to a float
		aWF.pFld->convert(&u32val,true);
		*value=(epicsInt32)u32val;

		TRACE_P_VVDEBUG("@0x%08X %s.%s : 0x%08x (val=%d)",
				aWF.pFld->getReg()->getOffset(true),
				aWF.pFld->getReg()->getCName(),aWF.pFld->getCName(),
				aWF.pFld->getReg()->getData(),*value);

		//And set value to the parameters list
		status = (asynStatus) setIntegerParam(function,*value);
	}
	else if(aWF.syncmode==AWB_SYNC_PRMLIST)
	{
		//Get the value from the parameter library
		status = (asynStatus) getIntegerParam(function, value);
	}


	//Do callbacks so higher layers see any changes
	status = (asynStatus) callParamCallbacks();

	if (status)
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
				"%s:%s: status=%d, function=%d, name=%s, value=%d",
				driverName.c_str(), __func__, status, function, paramName, *value);
	else
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
				"%s:%s: function=%d, name=%s, value=%d\n",
				driverName.c_str(), __func__, function, paramName, *value);
	return status;
}


/** Called when asyn clients call pasynFloat64->read().
 *
 * For default \ref AsynWBSync Mode we perform:
 * 		- Obtain the corresponding WBField using pasynUser->reason
 * 		- Synchronize or not the WBField using the memory connector.
 * 		- Convert it from WBField to value
 * 		- Set value in the parameter library
 *
 * \param[in] pasynUser pasynUser structure that encodes the reason and address.
 * \param[out] value The value read.
 * \return asynSuccess if okay, asynDisable if derived and another error code otherwise.
 **/
asynStatus asynWBPortDrvr::readFloat64(asynUser* pasynUser,
		epicsFloat64* value)
{
	int function = pasynUser->reason;
	asynStatus status = asynDisabled;
	const char *paramName;
	AsynWBField aWF = fldPrms[function];
	float f32val;

	// Fetch the parameter string name for possible use in debugging
	getParamName(function, &paramName);
	TRACE_P_VDEBUG("#%02d %s",function,paramName);

	//Derived if need
	if(aWF.syncmode==AWB_SYNC_DERIVED) return asynDisabled;

	//Sync from WB structure
	if(aWF.syncmode==AWB_SYNC_WBSTRUCT || aWF.syncmode==AWB_SYNC_DEVICE)
	{
		//Check if we have an existing association
		TRACE_CHECK_PTR(aWF.pFld,asynError);
		TRACE_CHECK_PTR(aWF.pFld->getReg(),asynError);

		//Sync WBField using the connector from device memory
		if(aWF.syncmode==AWB_SYNC_DEVICE && (syncNow & WB_AM_R))
			aWF.pFld->sync(pMemCon,WB_AM_R);

		//Convert the WBField to a float
		aWF.pFld->convert(&f32val,true);
		*value=(epicsFloat64)f32val;

		TRACE_P_VVDEBUG("@0x%08X %s.%s : 0x%08x (val=%f)",
				aWF.pFld->getReg()->getOffset(true),
				aWF.pFld->getReg()->getCName(),aWF.pFld->getCName(),
				aWF.pFld->getReg()->getData(),f32val);

		//And set value to the parameters list
		status = (asynStatus) setDoubleParam(function,*value);
	}
	else if(aWF.syncmode==AWB_SYNC_PRMLIST)
	{
		//Get the value from the parameter library
		status = (asynStatus) getDoubleParam(function, value);
	}

	//Do callbacks so higher layers see any changes
	status = (asynStatus) callParamCallbacks();

	if (status)
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
				"%s:%s: status=%d, function=%d, name=%s, value=%f",
				driverName.c_str(), __func__, status, function, paramName, *value);
	else
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
				"%s:%s: function=%d, name=%s, value=%f\n",
				driverName.c_str(), __func__, function, paramName, *value);
	return status;
}

/** Called when asyn clients call pasynIn32->write().
 *
 * For default \ref AsynWBSync Mode we perform:
 * 		- Obtain the corresponding WBField using pasynUser->reason
 * 		- Set value in the parameter library
 * 		- Convert it to WBField type
 * 		- Synchronize the WBField using the memory connector.
 *
 * \param[in] pasynUser pasynUser structure that encodes the reason and address.
 * \param[in] value Value to write.
 * \return asynSuccess if okay, asynDisable if derived and another error code otherwise.
 **/
asynStatus asynWBPortDrvr::writeInt32(asynUser* pasynUser, epicsInt32 value)
{
	int function = pasynUser->reason;
	bool ret;
	asynStatus status = asynDisabled;
	const char *paramName;
	AsynWBField aWF = fldPrms[function];

	// Fetch the parameter string name for possible use in debugging
	getParamName(function, &paramName);
	TRACE_P_DEBUG("#%02d %s=> %d (sync=%d)",function,paramName,value,aWF.syncmode);

	if(aWF.syncmode==AWB_SYNC_DERIVED) return asynDisabled;

	//Write to the device
	if(aWF.syncmode==AWB_SYNC_DEVICE)
	{
		//Get the sync mode
		TRACE_CHECK_PTR(aWF.pFld,asynError);
		TRACE_CHECK_PTR(aWF.pFld->getReg(),asynError);

		//Convert the value to WBField value
		uint32_t u32val=value;

		aWF.pFld->convert(&u32val,false);

		TRACE_P_VVDEBUG("@0x%08X %s.%s : %08x",
				aWF.pFld->getReg()->getOffset(true),
				aWF.pFld->getReg()->getCName(),aWF.pFld->getCName(),
				aWF.pFld->getReg()->getData());

		//Finally sync WBField using the connector to memory
		if(syncNow & WB_AM_W)
		{
			ret=aWF.pFld->sync(pMemCon,WB_AM_W);
			if(ret==false) return asynError;
		}
		else aWF.pFld->setToSync();

		TRACE_P_VVDEBUG("===>>>>>>>>>>>>>>@0x%08X %s.%s : %08x",
				aWF.pFld->getReg()->getOffset(true),
				aWF.pFld->getReg()->getCName(),aWF.pFld->getCName(),
				aWF.pFld->getReg()->getData());
	}
	else if(function==P_BlkSyncIdx)
	{
		//When setting back to zeros we need to sync pending registers
		WBAccMode syncMode=(WBAccMode)((~value) & WB_AM_RW); //syncMode is the inverse of block value
		if(value==0 && syncNow!=WB_AM_RW) status = this->syncPending(syncMode);
		TRACE_P_INFO("Syncing Mode: Old=%s%s (%d) => New=%s%s (%d) ... block value=%d",
				(syncNow&WB_AM_R)?"R":"", (syncNow& WB_AM_W)?"W":"", syncNow,
						(syncMode&WB_AM_R)?"R":"", (syncMode& WB_AM_W)?"W":"",syncMode,value);
		syncNow=syncMode;
	}

	// Set the parameter in the parameter library
	if((syncNow & WB_AM_W) || aWF.syncmode!=AWB_SYNC_DEVICE)
		status = (asynStatus) setIntegerParam(function, value);


	//Do callbacks so higher layers see any changes
	status = (asynStatus) callParamCallbacks();

	if (status)
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
				"%s:%s: status=%d, function=%d, name=%s, value=%d",
				driverName.c_str(), __func__, status, function, paramName, value);
	else
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
				"%s:%s: function=%d, name=%s, value=%d\n",
				driverName.c_str(), __func__, function, paramName, value);
	return status;
}

/** Called when asyn clients call pasynFloat64->write().
 *
 * For default \ref AsynWBSync Mode we perform:
 * 		- Obtain the corresponding WBField using pasynUser->reason
 * 		- Set value in the parameter library
 * 		- Convert it to WBField type
 * 		- Synchronize the WBField using the memory connector.
 *
 * \param[in] pasynUser pasynUser structure that encodes the reason and address.
 * \param[in] value Value to write.
 * \return asynSuccess if okay, asynDisable if derived and another error code otherwise.
 **/
asynStatus asynWBPortDrvr::writeFloat64(asynUser* pasynUser,
		epicsFloat64 value)
{
	int function = pasynUser->reason;
	bool ret;
	asynStatus status = asynDisabled;
	const char *paramName;
	AsynWBField aWF = fldPrms[function];

	// Fetch the parameter string name for possible use in debugging
	getParamName(function, &paramName);
	TRACE_P_DEBUG("#%02d %s => %.5f (sync=%d)",function,paramName,value,aWF.syncmode);

	if(aWF.syncmode==AWB_SYNC_DERIVED) return asynDisabled;

	//Write to the device
	if(aWF.syncmode==AWB_SYNC_DEVICE)
	{
		//Check if we have an existing association
		TRACE_CHECK_PTR(aWF.pFld,asynError);
		TRACE_CHECK_PTR(aWF.pFld->getReg(),asynError);

		//Concert the value to WBField value
		float f32val=value;
		aWF.pFld->convert(&f32val,false);

		//Finally sync WBField using the connector to memory
		if(syncNow & WB_AM_W)
		{
			ret=aWF.pFld->sync(pMemCon,WB_AM_W);
			if(ret==false) return asynError;
		}
		else aWF.pFld->setToSync();

		//And readback from value
		aWF.pFld->convert(&f32val,true);
		value=f32val;
	}

	// Set the parameter in the parameter library
	if((syncNow & WB_AM_W) || aWF.syncmode!=AWB_SYNC_DEVICE)
		status = (asynStatus) setDoubleParam(function, value);

	//Do callbacks so higher layers see any changes
	status = (asynStatus) callParamCallbacks();

	if (status)
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
				"%s:%s: status=%d, function=%d, name=%s, value=%f",
				driverName.c_str(), __func__, (int)status, function, paramName, value);
	else
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
				"%s:%s: function=%d, name=%s, value=%f\n",
				driverName.c_str(), __func__, function, paramName, value);
	return status;
}



/**
 * Writing strings to the parameter library
 *
 * \warning this function DOES NOT write to device
 *
 * \param[in] pasynUser pasynUser structure that encodes the reason and address.
 * \param[in] value Our string that we have written
 * \param[in] maxChar The maximum size that our string can use
 * \param[out] nActual The number of character of the string written.
 */
asynStatus asynWBPortDrvr::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,size_t *nActual)
{
	int function = pasynUser->reason;
	bool ret;
	asynStatus status = asynDisabled;
	const char *paramName;
	AsynWBField aWF = fldPrms[function];

	// Fetch the parameter string name for possible use in debugging
	getParamName(function, &paramName);
	TRACE_P_DEBUG("#%02d %s => %.5f (%d)",function,paramName,value,aWF.syncmode);

	if(aWF.syncmode==AWB_SYNC_DERIVED) return asynDisabled;

	//Write to the device
	if(aWF.syncmode==AWB_SYNC_DEVICE)
	{
		TRACE_P_WARNING("Writing String to device is not yet implemented");
		return asynError;
	}

	// Set the parameter in the parameter library
	status = (asynStatus) setStringParam(function,value);

	//Do callbacks so higher layers see any changes
	status = (asynStatus) callParamCallbacks();

	return status;
}


/**
 * Read octet strings from our driver
 *
 *
 * \param[in] pasynUser pasynUser structure that encodes the reason and address.
 * \param[out] value Our string that we have written
 * \param[in] maxChar The maximum size that our string can use
 * \param[out] nActual The number of character of the string read.
 * \param[out] eomReaseon ???
 */
asynStatus asynWBPortDrvr::readOctet(asynUser *pasynUser, char *value, size_t maxChars,
		size_t *nActual, int *eomReason)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName;
	AsynWBField aWF = fldPrms[function];

	// Fetch the parameter string name for possible use in debugging
	getParamName(function, &paramName);
	TRACE_P_DEBUG("#%02d %s => %s (%d) (%d)",function,paramName,value,maxChars,aWF.syncmode);

	if(aWF.syncmode==AWB_SYNC_DERIVED) return asynDisabled;

	//Write to the device
	if(aWF.syncmode==AWB_SYNC_DEVICE)
	{
		TRACE_P_WARNING("Reading String to device is not yet implemented");
		return asynError;
	}

	status= getStringParam(function,maxChars,value);
	*nActual=strlen(value);


	/* Do callbacks so higher layers see any changes */
	status = (asynStatus) callParamCallbacks();

	return status;
}



bool asynWBPortDrvr::cvtWBNodetoPrmList(WBNode *node)
{
	WBReg *reg=NULL;
	uint32_t u32val;
	float f32val;
	bool ret=true;

	TRACE_CHECK_PTR(node,false);

	for(size_t i=0;i<fldPrms.size();i++)
	{
		if(fldPrms[i].pFld==NULL) continue;

		while((reg=node->getNextReg(reg))!=NULL)
		{
			if(reg!=fldPrms[i].pFld->getReg()) continue;

			if(fldPrms[i].pFld->getType()==WBField::WBF_32U)
			{
				ret &= fldPrms[i].pFld->convert(&u32val,true);
				ret &= setIntegerParam(i,u32val);
			}
			else
			{
				ret &= fldPrms[i].pFld->convert(&f32val,true);
				ret &= setDoubleParam(i,f32val);
			}
		}
	}
	return ret;
}


