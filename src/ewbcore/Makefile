########################################################################
## Makefile to compile C++ object from a folder
##
## References:
##
## Authors: 
##	- Benoit Rat (Seven Solutions, www.sevensols.com)
##
## GNU Lesser General Public License Usage
## This file may be used under the terms of the GNU Lesser
## General Public License version 2.1 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU Lesser General Public License version 2.1 requirements
## will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
########################################################################
TOP = ../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
GIT_VER  = $(shell git describe --always --dirty=+)


LIBRARY_Linux = ewbcore
#ewbcore_LIBS = 

ewbcore_SRCS +=EWBBus.cpp
ewbcore_SRCS +=EWBField.cpp
ewbcore_SRCS +=EWBParam.cpp
ewbcore_SRCS +=EWBParamStrCmd.cpp
ewbcore_SRCS +=EWBPeriph.cpp
ewbcore_SRCS +=EWBReg.cpp
ewbcore_SRCS +=EWBTrace.cpp

INC +=EWBSync.h
INC += $(ewbcore_SRCS:.cpp=.h)



USR_CXXFLAGS +=$(USR_FLAGS)
#USR_CXXFLAGS +=$(CXX_FLAGS)
USR_CXXFLAGS +=-std=c++0x
USR_CXXFLAGS +=-D__GIT_VER__="\"$(GIT_VER)\""
USR_CXXFLAGS +=-I../../

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


#ODIR=output/
#
show:
	@echo "INC=$(INC)"
	@echo "ewbcore_SRC=$(ewb_SRCS)"
#
#core: $(ODIR)ewbcore $(ODIR)libewbcore.a
#	@echo "----> libewbcore.a OK"
#
#output/libewbcore.a: $(CORE_OBJS)
#	$(AR) rc $@ $^ $(LDFLAGS)
#
#output/%.o:		%.cpp
#	${CC} $(CXXFLAGS) $(INCLUDE_DIR) $(LIB_DIR) -c $*.cpp -o $@
#	
### Directories	
#$(ODIR)ewbcore:
#	mkdir -p $(ODIR)ewbcore
#
#$(ODIR)ewbbridge:
#	mkdir -p $(ODIR)ewbbridge
#
#clean_%:
#	rm -Rvf $(ODIR)$(subst clean_,,$@)
#	
#mrproper: clean_ewbcore 

