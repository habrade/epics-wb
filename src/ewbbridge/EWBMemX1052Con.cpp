/*
 * EWBMemX1052Con.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */
#include "awbpd_trace.h"

#ifndef EWBPD_NO_X1052

#define TRACE_H
#include <x1052_api.h>
#include <wdc_lib.h>

//TODO: Quick fix
#ifndef BOOL
#define BOOL int
#endif

#include EWBMemX1052Con.h"


#define TRACE_P_VDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)
#define TRACE_P_VVDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)
int EWBMemX1052Con::nHandles = -1; //!< Initiate static nHandles to count number of PCIe slot opened


/**
 * Constructor of the EWBMemX1052Con class.
 *
 * The constructor init the X1052 library and open an handler
 * on the corresponding device.
 * You need to use a valid WinDriver licence to open this driver.
 *
 * \param[in] idxPCIe The index of PCIe slot for X1052 Devices
 */
WBMemX1052Con::WBMemX1052Con(int idxPCIe,uint32_t magic_addr, uint32_t magic_val)
: EWBMemCon(WBMemCon::X1052,"X1052"), hBiDma(NULL), hDev(NULL)
{
	uint32_t dwStatus, tmp;
	if(nHandles<0)
	{
		dwStatus = X1052_LibInit();
		if (WD_STATUS_SUCCESS != dwStatus)
		{
			TRACE_P_ERROR("Failed to init the X1052 library: %s",
			X1052_GetLastErr());
		}
		else
		{
			nHandles=0;
		}
	}

	//Obtain version info
	std::stringstream ss1,ss2;
	int ret;
	char buf[MAX_DESC];
	X1052_GetInfo(buf,'v');
	ss1 << buf;
	X1052_GetInfo(buf,'r');
	ss1 << "; " << buf;
	ret = X1052_GetInfo(buf,'w');
	ss1 <<" (WD:" << ret/100 << "." << ret%100 << ")";
	ver=ss1.str();

	ss2 << "Slot index: "<< idxPCIe;
	desc=ss2.str();

	/* Find and open a X1052 device (by default ID) */
	hDev = X1052_DeviceOpen(idxPCIe);
	if(hDev) nHandles++;
	TRACE_P_INFO("Slot Index #%d : hDev=0x%x (nHandles=%d)",idxPCIe,hDev,nHandles);

	hBiDma= X1052_BiDMAGetHandle(hDev);

	if(hBiDma==NULL)
	{
		X1052_DeviceClose(hDev);
		hDev=NULL;
		TRACE_P_ERROR("Failed to Create BiDMA object");
	}

	/* Finaly check the magic address value */
	if(magic_addr!=0xFFFFFFFF)
	{
		this->mem_access(magic_addr,&tmp,false);
		if(tmp!=magic_val)
		{
			X1052_DeviceClose(hDev);
			hDev=NULL;
			TRACE_P_ERROR("Failed to Open device magic_number: 0x%0x (!=0x%0x)",tmp,magic_val);
		}


	}

}

/**
 * Desctructor of the EWBMemX1052 class
 *
 * It will close the handler on the PCIe device.
 */
WBMemX1052Con::~WBMemX1052Con()
{
	uint32_t dwStatus;
	if(hDev)
	{
		/* Perform necessary cleanup before exiting the program */
		X1052_DeviceClose(hDev);
		nHandles--;
	}

	if(nHandles<=0)
	{
		dwStatus = X1052_LibUninit();
		if (WD_STATUS_SUCCESS != dwStatus)
		{
			TRACE_P_ERROR("Failed to un-init the X1052 library: %s",
			X1052_GetLastErr());
		}
	}
}

/**
 * Return true if the handler on the PCIe is valid
 */
bool EWBMemX1052Con::isValid()
{
	 return hDev!=NULL;
}

/**
 * Single 32bit access to the wishbone device (seen as a memory map)
 *
 * \param[in] wb_addr The address of the data we want to access.
 * \param[inout] data the read "read from/write to" the device.
 * \param[in] to_dev if true we write to the device.
 * \see X1052_Wishbone_CSR() function
 */
bool EWBMemX1052Con::mem_access(uint32_t wb_addr, uint32_t* data, bool to_dev)
{
	int status;
	TRACE_CHECK_PTR(hDev,false);

	status=X1052_Wishbone_CSR(hDev,wb_addr,data,(int)to_dev);
	TRACE_CHECK_VA(status==S_OK,false,"%s@%08X %s %08x (%d)",(to_dev)?"W":"R", wb_addr,(to_dev)?"=>":"<=",*data,status);
	TRACE_P_VDEBUG("%s@%08X %s %08x (%d)",(to_dev)?"W":"R", wb_addr,(to_dev)?"=>":"<=",*data,status);

	return (status==S_OK);
}


/**
 * Block access to the wishbone device (seen as a memory map)
 *
 * This use the DMA of the device to access by blocks of [0x80-0x8000]
 * to the device.
 *
 * \param[in] dev_addr The address on the device of the data we want to access.
 * \param[in] nsize The size in byte that we want to read/write.
 * \param[in] to_dev if true we write to the device.
 * \see X1052_DMAToDev() and X1052_DMAFromDev() function.
 */
bool EWBMemX1052Con::mem_block_access(uint32_t dev_addr, uint32_t nsize,bool to_dev)
{
	int mbps;
	TRACE_CHECK_PTR(hDev,false);
	block_busy=true;

	if(nsize<0x80) nsize=X1052_DMA_TRANSFER_MINB;
	if(nsize>X1052_DMA_TRANSFER_MAXB)
	{
		TRACE_P_WARNING("@0x%08X: buffer size %d truncated to %d bytes",dev_addr,nsize, X1052_DMA_TRANSFER_MAXB);
	}

	mbps=X1052_DMATransfer(hBiDma, to_dev,0,dev_addr, nsize);

	block_busy=false;
	TRACE_P_DEBUG("%s@%08X %s (nsize=%d (0x%x), %.2f Gpbs)",(to_dev)?"W":"R", dev_addr,(to_dev)?"=>":"<=",nsize,nsize,mbps/1000.0);

	TRACE_CHECK_VA(mbps>0,false,
			"DMA failed #%d %s \n%s@%08X %s (nsize=%d)",
			mbps,X1052_GetLastErr(),
			(to_dev)?"W":"R", dev_addr,(to_dev)?"=>":"<=",nsize);

	return true;
}

/**
 * Retrieve the internal block buffer
 *
 * This directly retrieve the R or W buffer used for DMA transfer
 * by the X1052 library.
 *
 *  \param[out] hDma handler on the dma buffer
 *  \param[in] to_dev if true we return the write to dev buffer,
 * otherwise we return the read to dev buffer.
 *  \return The size in byte of the retrieved buffer
 */
uint32_t EWBMemX1052Con::get_block_buffer(uint32_t** hDma, bool to_dev)
{
	(*hDma)=(uint32_t*)X1052_BiDMAGetUserSpaceBuffer(hBiDma,(int)to_dev);
	return X1052_DMA_TRANSFER_MAXB;
}

#endif
