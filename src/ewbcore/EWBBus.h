/*
 * EWBBus.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBBUS_H_
#define EWBBUS_H_

#include <stdint.h>
#include <cstddef>
#include <vector>

class EWBBridge;
class EWBPeriph;

/**
 * Simple class that help us connecting different peripheral to a bus or a sub bus.
 */
class EWBBus {
public:
	EWBBus(EWBBridge *b, uint32_t base_offset, EWBBus *parent=NULL);
	virtual ~EWBBus();

	const EWBBridge* getBridge() const { return b; }
	EWBBridge* getBridge()  { return b; }
	uint32_t getOffset() const { return base_offset; }
	bool isValid(int level=-1) const;
	const std::vector<EWBBus*>& getChildren() const { return children; }
	const std::vector<EWBPeriph*>& getPeripherals() const { return periphs; }

	bool appendPeriph(EWBPeriph *pPrh);
	bool appendChild(EWBBus *bus);

protected:

	EWBBridge *b;
	uint32_t base_offset;
	EWBBus *parent;
	std::vector<EWBBus *> children;
	std::vector<EWBPeriph *> periphs;
};

#endif /* EWBBUS_H_ */
