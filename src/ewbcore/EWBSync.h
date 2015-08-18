/*
 * EWBSync.h
 *
 *  Created on: Jun 15, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBSYNC_H_
#define EWBSYNC_H_

#include <cstdint>


/**
 * Abstract interface to implement the sync functionality shared
 * by the different classes
 */
class EWBSync {
public:
	//! Access mode for a EWB Register or Field
	enum AMode {  EWB_AM_R=0x1, EWB_AM_W=0x2, EWB_AM_RW=0x3 };

	EWBSync(uint8_t mode): mode(mode), forceSync(false), toSync(false) {};
	virtual ~EWBSync() {};

	virtual bool sync(EWBSync::AMode mode) = 0;
	virtual bool isValid(int level=-1) const = 0;

	void setForceSync(bool val=true) { this->forceSync=val; };

	uint8_t getAccessMode() const { return mode; }			//!< Get the mode of access
	bool isModeRead() const  { return mode==EWB_AM_R; };		//!< Return @true if this parameter can be read from the device.
	bool isModeWrite() const  { return mode==EWB_AM_W; };	//!< Return @true if this parameter can be written to the device.

protected:
	uint8_t mode;
	bool forceSync;
	bool toSync;
};


//class EWBPVSync {
//
//public:
//
//	EWBPVSync();
//	virtual void preSync();
//	virtual void postSync();
//
//
//protected:
//	EWBParam *pPrm;
//	EWBSync *pSync;
//	int index;
//};

#endif /* EWBSYNC_H_ */
