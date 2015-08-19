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

USR_CXXFLAGS +=$(USR_FLAGS)
USR_CXXFLAGS +=-D__GIT_VER__="\"$(GIT_VER)\""


LIBRARY_Linux = ewbasyn
ewbasyn_LIBS += ewbcore
ewbasyn_LIBS += ewbbridge
ewbasyn_LIBS += asyn

ewbasyn_SRCS +=EWBAsynPortDrvr.cpp

INC += $(ewbasyn_SRCS:.cpp=.h)





include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


#ODIR=output/
#
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

