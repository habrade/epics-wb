Unit test of the epics-wb library using Gtest framework

Introduction
=============

In order to ensure a stable/bug-free library to 
bridge EPICS-IOC with our FPGA node we have decided to 
create a testing procedure that try some aspect of the
software in order to avoid bugs when porting to 
another environment or when adding features

Requierements
===============

* GCC >4.7 (C++11)
* Gtest-v1.7.0

Usage
==========

Please first compile using `make` command.
Then you should run the first serie of unit-tests
by executing:

	./ewb_test
