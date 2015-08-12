/*
 * EWBBridge.h
 *
 *  Created on: Jun 15, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBBRIDGE_H_
#define EWBBRIDGE_H_

#include <stdlib.h>
#include <stdint.h>
#include <string>

/**
 * Polymorphic & abstract class memory bridge to a EWB device.
 *
 * The inherited class:
 * 		- must overload single access to the memory
 * 		- might overload DMA access to the memory
 *
 * 	The child class will be defined for each type of physical driver used to access to our EWB board.
 */
class EWBBridge {
public:
	//! The type of the overridden class
	enum Type {
		TFILE=0,	//!< Connector to a test file
		X1052,		//!< Connector to the X1052 driver
		RAWRABBIT,	//!< Connector to the RawRabbit driver
		ETHERBONE	//!< Connector to the Etherbone driver
	};

	//! Constructor where we only give the type
	EWBBridge(int type,const std::string &name =""): type(type),name(name),block_busy(false) {};
	//! Empty destructor
	virtual ~EWBBridge() {};
	//! Return true if the handler of the overridden class if true, otherwise false.
	virtual bool isValid() { return false; }
	//! Generic single access to the wishbone memory of the device
	virtual bool mem_access(uint32_t addr, uint32_t *data, bool to_dev)=0;
	//! Retrieve the internal read/write buffer and its size
	virtual uint32_t get_block_buffer(uint32_t **hBuff, bool to_dev) { return 0; };
	//! Generic block access to the wishbone memory of the device
	virtual bool mem_block_access(uint32_t dev_addr, uint32_t nsize, bool to_dev) { return false; };
	//! Return which type of EWBBrdige overridden class we are using (force casting)
	int getType() { return type; }
	//! Return true if the block access is busy.
	bool isBlockBusy() { return block_busy; }

	virtual const std::string& getName() const { return name; }
	virtual const std::string& getVer() const { return ver; }
	virtual const std::string& getDesc() const { return desc; }

protected:
	int type; //!< type of the overridden class.
	std::string name;
	std::string desc;
	std::string ver;
    bool block_busy;
};


#endif /* EWBBRIDGE_H_ */
