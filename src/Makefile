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

##Test with the flags (DOESN'T WORK)
USR_FLAGS=-DDEBUG_NO_WAY
CXXFLAGS=-Wall -g -DTRACE_STDERR -Wno-format
USR_CXXFLAGS +=$(CXXFLAGS)
USR_CXXFLAGS +=-Wno-maybe-uninitialized

DIRS += ewbcore

DIRS += ewbbridge
ewbbridge_DEPEND_DIRS = ewbcore

DIRS += ewbasyn
ewbbridge_DEPEND_DIRS = ewbcore ewbbridge

include $(TOP)/configure/RULES_DIRS
