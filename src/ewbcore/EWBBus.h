/*
 * EWBBus.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBBUS_H_
#define EWBBUS_H_

#include "../ewbbridge/EWBBridge.h"
#include <cstdint>
#include <cstddef>

class EWBBridge;

/**
 * Simple class that help us connecting different peripheral to a bus or a sub bus.
 */
class EWBBus {
public:
	EWBBus(EWBBridge *b, uint32_t base_offset): b(b), base_offset(base_offset) {};
	virtual ~EWBBus();

	const EWBBridge* getBridge() const { return b; }
	EWBBridge* getBridge()  { return b; }
	uint32_t getOffset() const { return base_offset; }
	bool isValid(bool connected=true) const { return (connected? (b && b->isValid()) : (b!=NULL) ); }

protected:

	EWBBridge *b;
	uint32_t base_offset;
};

#endif /* EWBBUS_H_ */
