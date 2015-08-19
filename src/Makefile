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
TOP = ..
include $(TOP)/configure/CONFIG

### Cross Compile
CC =		$(CROSS_COMPILE)g++
LD =		$(CROSS_COMPILE)ld
AR =		$(CROSS_COMPILE)ar
OBJDUMP =	$(CROSS_COMPILE)objdump
OBJCOPY =	$(CROSS_COMPILE)objcopy
SIZE =		$(CROSS_COMPILE)size

### Obtain the version ($ is replaced by $$)
VERSION = $(shell git describe --always --dirty=+ | sed  's;^.*-\([v0-9\.]*\)\([a-z0-9\-+]*\)$$;\1\2;' )
DATE	= $(shell date +"%d %b. %Y")


### Create the user FLAGS
#USR_CFLAGS += -DDEBUG
CXXFLAGS=-Wall -g -DTRACE_STDERR -std=c++11
USR_CXXFLAGS +=${USR_FLAGS}
USR_CXXFLAGS +=${CXXFLAGS}
USR_CXXFLAGS +=-D__GIT_VER__="\"${GIT_VER}\""

EWB = $(TOP)/src

### Define the library
LIBRARY = ewb
ewb_LIBS += $(EPICS_BASE_IOC_LIBS)
ewb_LIBS += asyn


### Process ewb_core
CORE_SRC_DIR = $(EWB)/ewbcore
CORE_INC +=$(wildcard $(CORE_SRC_DIR)/*.h)
CORE_SRCS +=$(wildcard $(CORE_SRC_DIR)/*.cpp)
CORE_OBJS += $(addprefix $(ODIR), $(subst $(EWB)/,,$(CORE_SRCS:.cpp=.o)))

### Process ewb_bridge
BRIDGE_SRC_DIR = $(EWB)/ewbbridge
BRIDGE_INC +=$(BRIDGE_SRC_DIR)/EWBBridge.h
BRIDGE_INC +=$(BRIDGE_SRC_DIR)/EWBMemTestFileCon.h
BRIDGE_SRCS +=$(BRIDGE_SRC_DIR)/EWBMemTestFileCon.cpp


### Add external library for bridge
ifeq (${JUNGOWD_OFF},1)
USR_CXXFLAGS +=-DAWBPD_NO_X1052
else
BRIDGE_INC +=$(BRIDGE_SRC_DIR)/EWBMemTestFileCon.h
BRIDGE_SRCS +=$(BRIDGE_SRC_DIR)/EWBMemTestFileCon.cpp
USR_INCLUDES +=-I${JUNGOWD} -I${JUNGOWD}/include
USR_INCLUDES +=-I${X1052}   -I${X1052}/include
USR_SYS_LIBS += x1052_api
USR_SYS_LIBS += wdapi${JUNGOWDVER}
endif


### Append to the list
SRC_DIRS += $(CORE_SRC_DIR)
INC += $(CORE_INC)
ewb_SRCS += $(CORE_SRCS)
SRC_DIRS += $(BRIDGE_SRC_DIR)
INC += $(BRIDGE_INC)
ewb_SRCS += $(BRIDGE_SRCS)

LIBSRCS = $(ewb_SRCS) #Just to try

### Set to build
include $(TOP)/configure/RULES

###------------------------------------------------
### Custom compilation to test without EPICS

ODIR=output/

test:
	@echo "$(INC)"

core: $(ODIR)ewbcore $(ODIR)libewbcore.a
	@echo "----> libewbcore.a OK"

output/libewbcore.a: $(CORE_OBJS)
	$(AR) rc $@ $^ $(LDFLAGS)

output/%.o:		%.cpp
	${CC} $(CXXFLAGS) $(INCLUDE_DIR) $(LIB_DIR) -c $*.cpp -o $@
	
## Directories	
$(ODIR)ewbcore:
	mkdir -p $(ODIR)ewbcore

$(ODIR)ewbbridge:
	mkdir -p $(ODIR)ewbbridge

clean_%:
	rm -Rvf $(ODIR)$(subst clean_,,$@)
	
mrproper: clean_ewbcore 

