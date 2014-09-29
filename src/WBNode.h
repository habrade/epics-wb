/**
 * \file
 * \brief Contains the classes WBNode, WBReg, WBField and WBMemCon.
 *
 *  \see These classes use the *.h created by the wbgen2 tool. \n
 *  \n
 *  The file must be generated using the following command:\n
 *  <code>wbgen2 --cstyle=extended  --co=periph.h periph.wb</code> \n
 *  Check http://www.ohwr.org/projects/wishbone-gen
 *
 *  A typical tree structure example is the following
 *  \dot
 *  digraph tree {
 *  rankdir=BT;
 *      root [label="WBNode root" URL="\ref WBNode" shape=house];
 *      leafa [label="WBNode periph A" URL="\ref WBNode" shape=house];
 *      leafb [label="WBNode periph B" URL="\ref WBNode" shape=house];
 *      regr0 [label="WBReg 0" URL="\ref WBReg" shape=box];
 *      rega0 [label="WBReg 0" URL="\ref WBReg" shape=box];
 *      rega1 [label="WBReg 1" URL="\ref WBReg" shape=box];
 *      regb0 [label="WBReg 0" URL="\ref WBReg" shape=box];
 *      fldr00 [label="WBField 0" URL="\ref WBField" ];
 *      flda00 [label="WBField 0" URL="\ref WBField"];
 *      flda01 [label="WBField 1" URL="\ref WBField"];
 *      flda10 [label="WBField 0" URL="\ref WBField"];
 *      fldb00 [label="WBField 0" URL="\ref WBField"];
 *      fldb01 [label="WBField 1" URL="\ref WBField"];
 *      fldb02 [label="WBField 2" URL="\ref WBField"];
 *      leafa -> root [ arrowhead="open", style="dashed" ];
 *      leafb -> root [ arrowhead="open", style="dashed" ];
 *      regr0 -> root; fldr00->regr0;
 *      rega0 -> leafa; flda00->rega0; flda01->rega0;
 *      rega1 -> leafa; flda10->rega1;
 *      regb0 -> leafb; fldb00->regb0; fldb01->regb0; fldb02->regb0;
 *  }
 *  \enddot
 *
 *
 *  \date  Oct 11, 2013
 *  \author Benoit Rat
 */


#ifndef WBNODE_H_
#define WBNODE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

//! Generate field preprocessor variable name for wbgen2 header
#define WB2_TOKENPASTING_FIELD(periphname,regname,fieldname,type) \
		WB2_##periphname##_##regname##_##fieldname##type

//! Generate register preprocessor variable name for wbgen2 header
#define WB2_TOKENPASTING_REG(periphname,regname,type) \
		WB2_##periphname##_REG_##regname##type

//! Shortcut for WBReg constructor arguments
#define WB2_REG_ARGS(pname,rname) \
		WB2_TOKENPASTING_REG(pname,rname,_PREFIX),\
		WB2_TOKENPASTING_REG(pname,rname,)

//! Shortcut for WBField constructor arguments
#define WB2_FIELD_ARGS(pname,rname,fname) \
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_PREFIX),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_MASK),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SHIFT),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_ACCESS),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_NBFP),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_DESC)

//! Shortcut for WBField constructor arguments
#define WB2_FIELD_ARGS_NBFP(pname,rname,fname,nfb) \
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_PREFIX),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_MASK),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_SHIFT),\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_ACCESS),\
		nfb,\
		WB2_TOKENPASTING_FIELD(pname,rname,fname,_DESC)


class WBField;
class WBReg;
class WBNode;
class WBMemCon;

//! Access mode for a WB Register or Field
enum WBAccMode {  WB_AM_R=0x01, WB_AM_W=0x10, WB_AM_RW=0x11 };

#define WB_NODE_MEMBCK_OWNADDR 0xFFFFFFFF //!< Used by WBNode::sync()


/**
 * Class that represent a Wishbone field on a WBReg
 *
 * The WBField is a part (or not) of a WBReg.
 * We use a mask to show which bits correspond to this WBField in
 * the linked WBReg.
 *
 * \ref WBReg
 */
class WBField
{
public:
	//! Type of WBField available
	enum Type {
		WBF_32U,	//!< Unsigned integer field
		WBF_32I,	//!< Signed integer field
		WBF_32FP,	//!< Fixed point field with highest bit signed
		WBF_32F2C 	//!< Fixed point field with 2'complements signed
	};
	friend std::ostream & operator<<(std::ostream & output, const WBField &n);


	WBField(WBReg *pReg,const std::string &name, uint32_t mask, uint8_t shift, uint8_t mode=WB_AM_RW, uint8_t nfb=0,const std::string &desc="");
	virtual ~WBField();

	bool regCvt(uint32_t *value, uint32_t *regdata, bool from_data) const;
	bool regCvt(float *value, uint32_t *regdata, bool from_data) const;

	bool convert(uint32_t *value, bool to_value);
	bool convert(float *value, bool to_value);

	float getFloat() const;
	uint32_t getU32() const;

	bool sync(WBMemCon *con, WBAccMode amode=WB_AM_RW);

	uint32_t getMask() const { return mask; }				//!< Get the bit mask
	const std::string& getName() const { return name; }		//!< Get the name
	const char *getCName() const { return name.c_str(); } 	//!< Get the name in "C" format for printf function
	const std::string& getDesc() const { return desc; }		//!< Get the description
	uint8_t getNOfFractionBit() const { return nfb; }		//!< Get the number of fractional bit (0 for WBF_32U)
	uint8_t getAccessMode() const { return mode; }			//!< Get the mode of access
	uint8_t getType() const { return type; }				//!< Get the type of field
	const WBReg* getReg() const { return pReg; }			//!< Get the linked register
	bool isOverflowPrevented() const { return checkOverflow; }	//!< When true prevent overflow during FP conversion \ref regCvt(), \ ref convert()

protected:
	std::string name;	//!< Name of the WBField
	uint32_t mask;		//!< Corresponding mask
	uint8_t shift;		//!< Number of bit to be shift
	uint8_t width;		//!< Width of the field
	uint8_t mode;		//!< Access mode on the device
	uint8_t type;		//!< Type of data
	uint8_t nfb;		//!< Number of fraction bits
	std::string desc;	//!< Description
	bool checkOverflow;	//!< Limit overflow during FP conversion

private:
	WBReg *pReg; //! parent register which belong this field
};

/**
 * Class to manipulate Wishbone register width various WBField
 *
 * \ref WBNode
 * \ref WBField
 */
class WBReg {
public:
	friend class WBField;
	friend class WBNode;
	friend std::ostream & operator<<(std::ostream & output, const WBReg &r);

	WBReg(WBNode *pPrtNode,const std::string &name, uint32_t offset, const std::string &desc="");
	virtual ~WBReg();

	uint32_t getOffset(bool absolute=false) const;
	bool sync(WBMemCon *con, WBAccMode amode=WB_AM_RW);

	bool addField(WBField *fld);
	const WBField* getField(const std::string& name) const;
	const WBField* operator[](const std::string& name) const { return this->getField(name); }

	const std::vector<WBField*> getFields() const { return fields; }	//!< Get a vector on the belonging WBField
	const WBNode* getPrtNode() const { return pPrtNode; }				//!< Get the parent WBNode

	uint32_t getData() const { return data; }					//!< Get the data
	const std::string& getName() const { return this->name; }	//!< Get the name
	const char *getCName() const { return this->name.c_str(); }	//!< Get the name in "C" format for printf function
	const std::string& getDesc() const { return this->desc; }	//!< Get the description

protected:
	std::vector<WBField*> fields;	//!< A list of the relative WBFields
	std::string name;		//!< The name
	std::string desc;		//!< A description
	uint32_t offset;		//!< The offset relative to WBNode
	uint32_t data;			//!< The corresponding data
	uint32_t used_mask;		//!< The mask used by other WBField

private:
	WBNode *pPrtNode;

};


/**
 * Class that represent a WB peripheral in a tree structure.
 *
 * 		- This class can have a WBNode parent otherwise it is the root structure.
 * 		- This class can have one or more WBNode children
 * 		- A list WBReg is also linked to this peripheral
 */
class WBNode {
public:
	WBNode(const std::string &name, uint32_t addr, const std::string &desc="");
	WBNode(WBNode *parent,const std::string &name, uint32_t addr, const std::string &desc="");
	virtual ~WBNode();

	void appendReg(WBReg *pReg);
	WBReg* getReg(uint32_t offset) const;
	WBReg* getNextReg(WBReg *prev);
	WBReg* getLastReg() const { return registers.rbegin()->second; }	//!< Get the highest WBReg in the node.
	std::vector<WBNode*> getChildren() const { return children; }		//!< Get a vector of children node.

	bool sync(WBMemCon *con, WBAccMode amode=WB_AM_RW);
	bool sync(WBMemCon *con, WBAccMode amode, uint32_t dma_dev_offset=WB_NODE_MEMBCK_OWNADDR);

	int getID() const { return this->ID; }						//!< Get unique ID of WBNode
	uint32_t getAddress() const { return this->address; }		//!< Get Address of WBnode
	const std::string& getName() const { return this->name; }	//!< Get the name
	const char *getCName() const { return this->name.c_str(); }	//!< Get the name in "C" format for printf function
	const std::string& getDesc() const { return this->desc; }	//!< Get the description
	const bool isRoot() const { return this->is_root; }			//!< Check if this note is the root.

	void print(std::ostream & o, int level=0) const;
	friend std::ostream & operator<<(std::ostream & o, const WBNode &n) { n.print(o); return o; } //!< \ref print()

protected:
	std::string name;	//!< Name of the peripheral node
	std::string desc;	//!< Description of the peripheral node
	uint32_t address;	//!< Address of the peripheral node


private:
	static int sID;
	int ID;
	WBNode *parent;
	std::map<uint32_t,WBReg*> registers;
	std::map<uint32_t,WBReg*>::iterator ii_nxtreg;
	std::vector<WBNode*> children;
	bool is_root;
};


/**
 * Virtual class to connect to a device memory in order to sync the WBnode structure.
 *
 * The child class:
 * 		- must redefine single access to the memory
 * 		- might redefine DMA access to the memory
 *
 * 	The child class will be defined for each type of physical driver used to access to our WB board.
 */
class WBMemCon {
public:
	//! The type of the overridden class
	enum Type {
		TFILE=0,	//!< Connector to a test file
		X1052,		//!< Connector to the X1052 driver
		RAWRABBIT,	//!< Connector to the RawRabbit driver
		ETHERBONE	//!< Connector to the Etherbone driver
	};

	//! Constructor where we only give the type
	WBMemCon(int type,const std::string &name =""): type(type),name(name),block_busy(false) {};
	//! Empty destructor
	virtual ~WBMemCon() {};
	//! Return true if the handler of the overridden class if true, otherwise false.
	virtual bool isValid() { return false; }
	//! Generic single access to the wishbone memory of the device
	virtual bool mem_access(uint32_t addr, uint32_t *data, bool to_dev)=0;
	//! Retrieve the internal read/write buffer and its size
	virtual uint32_t get_block_buffer(uint32_t **hBuff, bool to_dev) { return 0; };
	//! Generic block access to the wishbone memory of the device
	virtual bool mem_block_access(uint32_t dev_addr, uint32_t nsize, bool to_dev) { return false; };
	//! Return which type of WBMemCon overridden class we are using (force casting)
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




#endif /* WBNODE_H_ */
