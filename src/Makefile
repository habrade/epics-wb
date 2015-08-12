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
INCLUDE_DIR=-Iewbcore/ -Iewbdrige/ -Iasynwb
CXXFLAGS=-Wall -g -O3 -DTRACE_STDERR -std=c++11

## File processing
ODIR=output/

SRC_MAIN=$(wildcard ewbcore/*.cpp)
OBJ_MAIN=$(addprefix $(ODIR), $(SRC_MAIN:.cpp=.o))

all: main 

ODIR_%: $(ODIR)$(subst ODIR_,,$@)
	mkdir -p $(ODIR)$(subst ODIR_,,$@)
	@echo ""
	@echo ":==================================================================================================================>"
	@echo ""
	
main: ODIR_ewbcore $(ODIR)libewbmain.a
	@echo "----> libewbmain.a OK"
	
output/libewbmain.a: $(OBJ_MAIN)
	$(AR) rc $@ $^ $(LDFLAGS)

bridge:

asynwb:


output/%.o:		%.cpp
	${CC} $(CXXFLAGS) $(INCLUDE_DIR) $(LIB_DIR) -c $*.cpp -o $@
	

clean_%:
	rm -Rvf $(ODIR)$(subst clean_,,$@)
	
clean: clean_ewbcore 

