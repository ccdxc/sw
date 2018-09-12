#
# Copyright (c) 2017, Pensando Systems Inc.
#

default: all

TOPDIR = .

include $(TOPDIR)/make/Makefile.inc

all:
	$(MAKE) -C deps
	$(MAKE) -C src
	$(MAKE) -C tools

sim: all
	$(MAKE) -C $(TOPDIR)/src/sim/qemu
