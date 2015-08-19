/*
 * EWBParamStrCmd.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#include "EWBParamStrCmd.h"

#include "ewbbridge/EWBCmdConsole.h"

#include <iostream>
#include <string>


EWBParamStrCmd::EWBParamStrCmd(EWBCmdConsole *pConsole,const std::string& name, const std::string& cmdW, const std::string& cmdR, const std::string& eStrR, const std::string& desc)
:EWBParamStr(name,EWBSync::EWB_AM_RW,"",desc), pConsole(pConsole), cmdW(cmdW), cmdR(cmdR)
{
	//Update mode
	if(cmdR.empty()) mode=EWBSync::EWB_AM_W;
	if(cmdW.empty()) mode=EWBSync::EWB_AM_R;

//	try {
//		this->rgxpR=std::regex(rgxpR);
//	}
//	catch(std::regex_error &e)
//	{
//		std::cout << "ERROR: " << e.what() << "; code: " << parseCode(e.code()) << std::endl;
//	}

	int reti = regcomp(&rgxpR, eStrR.c_str(),  REG_EXTENDED);
	if (reti) {
	    fprintf(stderr, "ERROR: Could not compile regex\n");
	}


}

/**
 * Empty Destructor
 *
 */
EWBParamStrCmd::~EWBParamStrCmd()
{
	regfree(&rgxpR);
}


bool EWBParamStrCmd::isValid(int level) const
{
	return (level!=0)?(pConsole && pConsole->isValid()):pConsole!=NULL;
}


//std::string EWBParamStrCmd::parseCode(std::regex_constants::error_type etype) {
//    switch (etype) {
//    case std::regex_constants::error_collate:
//        return "error_collate: invalid collating element request";
//    case std::regex_constants::error_ctype:
//        return "error_ctype: invalid character class";
//    case std::regex_constants::error_escape:
//        return "error_escape: invalid escape character or trailing escape";
//    case std::regex_constants::error_backref:
//        return "error_backref: invalid back reference";
//    case std::regex_constants::error_brack:
//        return "error_brack: mismatched bracket([ or ])";
//    case std::regex_constants::error_paren:
//        return "error_paren: mismatched parentheses(( or ))";
//    case std::regex_constants::error_brace:
//        return "error_brace: mismatched brace({ or })";
//    case std::regex_constants::error_badbrace:
//        return "error_badbrace: invalid range inside a { }";
//    case std::regex_constants::error_range:
//        return "erro_range: invalid character range(e.g., [z-a])";
//    case std::regex_constants::error_space:
//        return "error_space: insufficient memory to handle this regular expression";
//    case std::regex_constants::error_badrepeat:
//        return "error_badrepeat: a repetition character (*, ?, +, or {) was not preceded by a valid regular expression";
//    case std::regex_constants::error_complexity:
//        return "error_complexity: the requested match is too complex";
//    case std::regex_constants::error_stack:
//        return "error_stack: insufficient memory to evaluate a match";
//    default:
//        return "";
//    }
//}


bool EWBParamStrCmd::sync(EWBSync::AMode m) {

	if(isValid())
	{
		if(m & EWBSync::EWB_AM_W)
		{
			pConsole->writeCmd(cmdW,value);
		}
		if(m & EWBSync::EWB_AM_R)
		{
			value=pConsole->getCmd(cmdR);
			if(rgxpR.re_nsub>0) //Check if we have a compile regex when reading
			{
				/* Execute regular expression */
				regmatch_t pMatch[rgxpR.re_nsub+1];
				const char *s=value.c_str();
				int reti = regexec(&rgxpR,s,rgxpR.re_nsub+1, pMatch, 0);
				if (!reti) {
//					printf("%d matched from %d (%c) to %d (%c)\n",rgxpR.re_nsub,
//							pMatch[1].rm_so, s[pMatch[1].rm_so],pMatch[1].rm_eo, s[pMatch[1].rm_eo]);
					value=value.substr(pMatch[1].rm_so,pMatch[1].rm_eo-pMatch[1].rm_so);
				}
				else if (reti == REG_NOMATCH) {
					puts("No match");
				}
				else {
					char msgbuf[256];
					regerror(reti, &rgxpR, msgbuf, sizeof(msgbuf));
					fprintf(stderr, "Regex match failed: %s\n", msgbuf);
				}
			}

//			//C++11 (Need at least GCC v4.9)
//			if(rgxpR.mark_count()>0)
//			{
//				std::smatch sm;
//
//				std::regex_match(value,sm,rgxpR);
//				if(sm.size()>1)
//				{
//					value=sm[1];
//					std::cout << "YEAH="<<sm[1] << "--- ; " << value << std::endl;
//				}
//			}
		}
		return true;
	}
	return false;
}
