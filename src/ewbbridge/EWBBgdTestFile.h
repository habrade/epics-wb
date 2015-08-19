/**
 *  \file
 *  \brief Contains the class EWBMemTestFileCon.
 *
 *
 *  \date  Nov 4, 2013
 *  \author Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBMEMTFILECON_H_
#define EWBMEMTFILECON_H_

#include "EWBBridge.h"

#include <iostream>
#include <fstream>
#include <string>

class EWBBus; //!< Forward declaration

/**
 * Fake EWB memory connector by testing on a file
 *
 * The operation will read and write in the test file.
 *
 * A typical testfile is shown below:
 * \code
 * # Format as below
 * # 0x<hex_address> : <hex_value>
 * 0x10000024: 0ABCD000
 * 0x20000000: 01e1389c
 * 0x20000004: 00060005
 * 0x20000008: e99c4fb5
 * 0x2000000C: 00000000
 * \endcode
 *
 */
class EWBMemTestFileCon: public EWBBridge {
public:
	EWBMemTestFileCon(const std::string& fname);
	virtual ~EWBMemTestFileCon();

	void generate(EWBBus* root);

	virtual bool isValid() { return o_file.is_open(); }				//!< Return true if the file has been opened
	bool mem_access(uint32_t addr, uint32_t *data, bool from_dev);
	uint32_t get_block_buffer(uint32_t **hDma, bool to_dev);
	bool mem_block_access(uint32_t dev_addr, uint32_t nsize, bool to_dev);

private:
	long seek(std::fstream& tfile, uint32_t wb_addr, bool rewind=false);
	std::fstream o_file;
	std::string fname;
	long lastpos;
	uint32_t *pData;
};

#endif /* EWBMEMTFILECON_H_ */
