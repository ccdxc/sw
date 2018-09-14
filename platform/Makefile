#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

SUBDIRS = deps src tools
TOPDIR = .

include $(TOPDIR)/make/Makefile.inc

sim: all
	$(MAKE) -C $(TOPDIR)/src/sim/qemu
