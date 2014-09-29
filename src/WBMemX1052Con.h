/**
 *  \file
 *  \brief Contains the class WBMemX1052Con.
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

#ifndef WBMEMX1052CON_H_
#define WBMEMX1052CON_H_

#include "WBNode.h"

#ifndef _WDC_LIB_H_
typedef void *WDC_DEVICE_HANDLE; //!< Hack to not include all the X1052 api in this header
#endif

/**
 * WB memory connector using X1052 driver
 *
 * \warning
 * The x1052_api and Jungo Windriver libraries must be linked
 * during the compilation if we want to compile this class.\n
 * You might also define the \b AWBPD_NO_X1052 preprocessor variable
 * if you want to disable the compilation of the cpp file.
 *
 */
class WBMemX1052Con: public WBMemCon {
public:
	WBMemX1052Con(uint32_t idPCIe,uint32_t magic_addr=0xFFFFFFFF, uint32_t magic_val=-1);
	virtual ~WBMemX1052Con();

	bool isValid();

	bool mem_access(uint32_t addr, uint32_t *data, bool from_dev);
	uint32_t get_block_buffer(uint32_t **hDma, bool from_dev);
	bool mem_block_access(uint32_t dev_addr, uint32_t nsize, bool from_dev);

private:
    WDC_DEVICE_HANDLE hDev;
};

#endif /* WBMEMX1052CON_H_ */
