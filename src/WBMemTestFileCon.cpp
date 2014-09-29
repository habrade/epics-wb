/*
 * WBMemTestFileCon.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "WBMemTestFileCon.h"
#include <cstring>

#include "awbpd_trace.h"


#define TRACE_P_VDEBUG(...) TRACE_P_DEBUG( __VA_ARGS__)
#define TRACE_P_VVDEBUG(...) //TRACE_P_DEBUG( __VA_ARGS__)

#define BUFF_MAX_SIZEB 4096 //Size in bytes


/**
 * Constructor of the WBMemTestFileCon
 *
 * This function will allocate one internal buffer of
 * BUFF_MAX_SIZEB for both read and write.
 * It will also try to open the file.
 */
WBMemTestFileCon::WBMemTestFileCon(const std::string& fname)
: WBMemCon(WBMemCon::TFILE), fname(fname), lastpos(0)
{
	o_file.open(fname.c_str(),std::fstream::out|std::fstream::in);
	TRACE_P_INFO("tfile=%d (%s)",o_file.is_open(),fname.c_str());

	pData = (uint32_t*)malloc(BUFF_MAX_SIZEB);
}

/**
 * Destructor
 *
 * 	- Close the file
 * 	- Free the internal buffer
 */
WBMemTestFileCon::~WBMemTestFileCon()
{
	o_file.close();


	free(pData);
}

/**
 * Function that fill the file with the all the registers in the wishbone structure.
 */
void WBMemTestFileCon::generate(WBNode* node)
{
	WBReg *reg=NULL;
	uint32_t data;
	if(node==NULL) return;

	//If the file does not exist it can't be open as in|out (only out)...
	if(o_file.is_open()==false)
	{
		o_file.open(fname.c_str(),std::fstream::out); //...,so we create it as out...
		o_file.sync();
		o_file.close();
		o_file.open(fname.c_str(),std::fstream::out|std::fstream::in); //and reopen as in|out.
	}

	TRACE_P_DEBUG("%s (%d)",node->getCName(), node->getChildren().size());

	while( (reg=node->getNextReg(reg)) != NULL)
	{
		TRACE_P_VDEBUG("%s (@0x%08X) 0x%08x",reg->getCName(),reg->getOffset(true),reg->getData());
		data=reg->getData();
		mem_access(reg->getOffset(true),&data,true);
	}

	std::vector<WBNode *> children = node->getChildren();
	for(size_t i=0;i<children.size();i++)
		generate(children[i]);
}

/**
 * Single line access of a wishbone register in the device.
 *
 * \param[in] wb_addr The address of the data we want to access.
 * \param[inout] data the read "read from/write to" the file.
 * \param[in] to_dev if true we write to the file (aka device).
 */
bool WBMemTestFileCon::mem_access(uint32_t wb_addr, uint32_t* data, bool to_dev)
{
	uint32_t rdata=0xDA1AFEED;
	char buff[50];
	std::string line;
	//std::fstream tfile(fname.c_str(), std::ios::in|std::ios::out);

	TRACE_CHECK(isValid(),false,"Not valid file");
	o_file.sync();

	//first seek position
	int pos=seek(o_file,wb_addr,true);
	if(to_dev) //writing to file
	{
		//Move to correct position
		if(pos>=0) o_file.seekp(pos,std::ios::beg);
		else o_file.seekp(0,std::ios::end);

		snprintf(buff,50,"0x%08X: %08x",wb_addr,*data);
		TRACE_P_VDEBUG("%d  <=> '%s' (g=%ld, p=%ld)",pos, buff, o_file.tellg(), o_file.tellp());
		o_file << buff << std::endl;
	}
	else //Reading from file
	{
		if(pos>0)
		{
			o_file.seekg(pos,std::ios::beg);
			if(getline (o_file,line))
			{
				std::string strdata = line.substr(12,8);
				char * p;
				rdata= strtoul( strdata.c_str(), & p, 16 );
				TRACE_P_VVDEBUG("%s  => '%s' => 0x%x",line.c_str(),strdata.c_str(),rdata);
			}
			//tfile >> "0x" >> tmp >> ": " >> rdata;
		}
		*data=rdata;
	}
	TRACE_P_VDEBUG("%s@%08X %s %08x (%d)",(to_dev)?"W":"R", wb_addr,(to_dev)?"=>":"<=",*data, pos);
	o_file.sync();
	o_file.flush();
	return true;
}

uint32_t WBMemTestFileCon::get_block_buffer(uint32_t** hDma, bool to_dev)
{
	*hDma=pData;
	return BUFF_MAX_SIZEB;
}

/**
 * Block access to the test file
 *
 * First seek the starting position of the block given by dev_addr and then read/write
 * consecutively all the other position to the file
 *
 * \warning This method is intend to be very basic. It is working well on pre-formated test file
 * obtained by the generate() method.
 * 	- When writing this function will override all the line after the first position
 * 	- When reading this function will just check that the next line address corresponds to
 * 	its next expected line. If there are some swapping or missing reg on the text file the function
 * 	will not behave correctly.
 *
 * \param[in] dev_addr The address on the device of the data we want to access.
 * \param[in] nsize The size in byte that we want to read/write.
 * \param[in] to_dev if true we write to the device.
 */
bool WBMemTestFileCon::mem_block_access(uint32_t dev_addr, uint32_t nsize,bool to_dev)
{
	char buff[50], *p;
	std::string line;
	uint32_t defdata=0xDA1AFEED;
	block_busy=true;
	std::fstream tfile(fname.c_str(), std::ios::in|std::ios::out);

	int pos=seek(tfile,dev_addr,true);
	TRACE_P_DEBUG("0x%08X nsize=%03d (%s) fpos=%d",dev_addr,nsize,(to_dev)?"W":"R",pos);
	if(to_dev)
	{
		TRACE_P_WARNING("Write not implemented");
		block_busy=false;
		return false;
	}
	else
	{
		tfile.seekg(pos,std::ios::beg);
		if(nsize>BUFF_MAX_SIZEB)
		{
			TRACE_P_WARNING("nsize=%d > BUFF_MAX_SIZEB=%d",nsize,BUFF_MAX_SIZEB);
			nsize=BUFF_MAX_SIZEB;
		}
		for(size_t i=0;i<nsize;i++)
		{
			pData[i]=defdata;
			if(pos>0 && getline (tfile,line))
			{
				snprintf(buff,50,"0x%08X",dev_addr+i*sizeof(uint32_t));
				if(strncmp(buff,line.substr(0,10).c_str(),10)==0)
					pData[i]= strtoul( line.substr(12,8).c_str(), & p, 16 );

				TRACE_P_VDEBUG("#%03d@0x%08X : 0x%8x (%s)",
						i,dev_addr+i*sizeof(uint32_t),pData[i],line.c_str());
			}
		}
	}
	block_busy=false;
	return true;

}

/**
 * Search for the first occurence of wb_addr on the test file.
 *
 * This method also modified the lastpos value.
 *
 * \param[inout] tfile the given test file (only the position pointer will be moved)
 * \param[in] wb_addr the address that we are looking for
 * \param[in] rewind if true we start the search from the beginning, otherwise from the current position.
 * \return the position of the line that contains the address or -1 if it was not found.
 */
long WBMemTestFileCon::seek(std::fstream& tfile, uint32_t wb_addr, bool rewind)
{
	std::string line;

	//Create comparation buffer
	char buff[20];
	snprintf(buff,20,"0x%08X",wb_addr);
	std::string test(buff);

	//Put the file to the correct place
	if(rewind)  tfile.seekg(0, std::ios::beg);
	else tfile.seekg(lastpos,std::ios::beg);
	long pos=tfile.tellg();



	while (getline (tfile,line))
	{
		if(line.size()>10 && (line[0] != '#'))
			//if(line.size()>10)
		{
			TRACE_P_VVDEBUG("'%s'=='%s'",line.substr(0,10).c_str(),test.c_str());
			if(line.substr(0,10)==test)
			{
				lastpos=pos;
				return lastpos;
			}
		}
		pos=tfile.tellg();
	}
	//If we reach the eof we reset
	if(tfile.eof())
	{
		tfile.clear(tfile.eofbit);
		lastpos=0;
		tfile.seekg(lastpos, std::ios::beg);
	}
	return -1;
}



