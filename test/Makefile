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

## Cross Compile
CC =		$(CROSS_COMPILE)g++
LD =		$(CROSS_COMPILE)ld
AR =		$(CROSS_COMPILE)ar
OBJDUMP =	$(CROSS_COMPILE)objdump
OBJCOPY =	$(CROSS_COMPILE)objcopy
SIZE =		$(CROSS_COMPILE)size

## Config
-include $(CURDIR)/.config

## Obtain the version ($ is replaced by $$)
VERSION = $(shell git describe --always --dirty=+ | sed  's;^.*-\([v0-9\.]*\)\([a-z0-9\-+]*\)$$;\1\2;' )
DATE	= $(shell date +"%d %b. %Y")

## Flags
INCLUDE_DIR=-I../src/ewbcore/ -I../src/ewbbridge/ -I../src/asynwb
GTEST_DIR=/home/opt/gtest-1.7.0
CPPFLAGS += -isystem $(GTEST_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread -Wno-reorder  -DTRACE_STDERR -std=c++11 
LFLAGS=-L$(GTEST_DIR)/lib/ -L../src/output/ -lpthread
## File processing
ODIR=../src/output/

OBJ_MAIN=EWBParamStrCmd_test.o \
	EWBField_test.o \
	EWBReg_test.o \
	EWBPeriph_test.o \


# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h


GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)


all: ewb_test
	
main: $(TESTS_MAIN) 


../lib/linux-x86/%.a: 
	make -C .. /


# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	$(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	$(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

#Compiling objects
%.o: %.cpp 
	${CC} $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE_DIR) -c $*.cpp -o $@

#Final app
ewb_test: ewb_test.o EWBFakeWRConsole.o $(OBJ_MAIN) ../lib/linux-x86/libewbcore.a ../lib/linux-x86/libewbbridge.a gtest_main.a 
	${CC} $(CPPFLAGS) $(CXXFLAGS) $(LFLAGS) $^ -o $@
	
clean:
	rm -vf *.o gtest_main.* ewb_test

