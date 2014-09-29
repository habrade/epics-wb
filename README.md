Introduction
================

EPICS support for Wishbone peripherals: The following library is used as
a Generic EPICS IOC AsynDriver to support wishbone peripheral. 
It include the following features:

* Driver for X1052, Gennum, Etherbone WB master.
* Direct access to any register in the wishbone bus
* Auto-generation of EPICS Database file using wbgen2
* Automatic real number convertion (2 complements, fixed point, signess) using .wb file
* Support for WR Core and other internal bus protocols (i2c, spi, etc.)
