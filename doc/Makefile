#Makefile at top of application tree
TOP = ../
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocBoot))

GIT_VER  = $(shell git describe --always --dirty=+)

INSTALL_DOC=html
SRCDIR=../src

all: doc
doc: html
html: common
	@echo "Installing doc $(INSTALL_DOC)"
	rm -rf $(INSTALL_DOC)
	mkdir -p $(INSTALL_DOC)/..
	(echo "OUTPUT_DIRECTORY=$(INSTALL_DOC)/.."; \
	echo "PROJECT_NUMBER=rev:${GIT_VER}"; \
	echo TAGFILES = $(ASYN)/documentation/asyn_doxy.tag; \
	echo "INPUT=$(SRCDIR)"; echo QUIET=NO;) | cat $(SRCDIR)/doxyfile.int - > O.Common/doxy.in
	doxygen O.Common/doxy.in

common:
	mkdir -p O.Common

