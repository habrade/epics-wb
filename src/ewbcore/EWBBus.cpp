/*
 * EWBBus.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */
#include "EWBBus.h"
#include "EWBPeriph.h"
#include "EWBTrace.h"

#include "ewbbridge/EWBBridge.h"

EWBBus::EWBBus(EWBBridge *b, uint32_t base_offset, EWBBus *parent)
: b(b), base_offset(base_offset), parent(parent)
{
	if(parent)
	{
		bool added = parent->appendChild(this);
		if(added==false)
		{
			parent=NULL;
			TRACE_P_WARNING("Could not append to parent");
		}
	}
};


EWBBus::~EWBBus()
{
	for(size_t j=0;j<periphs.size();j++)
	{
		if(periphs[j]) delete periphs[j];
	}
	for(size_t j=0;j<children.size();j++)
	{
		if(children[j]) delete children[j];
	}
}

bool EWBBus::isValid(int level) const
{
	return ((level!=0)? (b && b->isValid()) : (b!=NULL) );
}

bool EWBBus::appendPeriph(EWBPeriph *pPrh)
{
	if(pPrh)
	{
		//Adding to vector
		periphs.push_back(pPrh);
		return true;
	}
	return false;
}

bool EWBBus::appendChild(EWBBus *pBus)
{
	if(pBus)
	{
		//Might check if already exist

		//Adding to vector
		children.push_back(pBus);
		return true;
	}
	return false;
}



