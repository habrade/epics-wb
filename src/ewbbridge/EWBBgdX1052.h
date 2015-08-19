/**
 *  \file
 *  \brief Contains the class EWBMemX1052Con.
 *
 *
 *
 *  \see This class use the:
 *  	- X1052 driver API
 *  	- Jungo Windriver API (10.10)
*
 *
 *  \date  Oct 31, 2013
 *  \author Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBMEMX1052CON_H_
#define EWBMEMX1052CON_H_

#include "EWBBridge.h"

#ifndef _WDC_LIB_H_
typedef void * WDC_DEVICE_HANDLE; //!< Hack to not include all the X1052 api in this header
#endif
#ifndef _X1052_LIB_H_
typedef void * X1052_BIDMA_HANDLE; //!< Hack to not include all the X1052 api in this header
#endif

/**
 * EWB memory connector using X1052 driver
 *
 * \warning
 * The x1052_api and Jungo Windriver libraries must be linked
 * during the compilation if we want to compile this class.\n
 * You might also define the \b EWBPD_NO_X1052 preprocessor variable
 * if you want to disable the compilation of the cpp file.
 *
 */
class EWBMemX1052Con: public EWBBridge {
public:
	EWBMemX1052Con(int idPCIe,uint32_t magic_addr=0xFFFFFFFF, uint32_t magic_val=-1);
	virtual ~EWBMemX1052Con();

	bool isValid();

	bool mem_access(uint32_t addr, uint32_t *data, bool from_dev);
	uint32_t get_block_buffer(uint32_t **hDma, bool from_dev);
	bool mem_block_access(uint32_t dev_addr, uint32_t nsize, bool from_dev);

private:
    WDC_DEVICE_HANDLE hDev;
    X1052_BIDMA_HANDLE hBiDma;

    static int nHandles;
};

#endif /* EWBMEMX1052CON_H_ */
