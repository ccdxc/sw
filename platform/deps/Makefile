#
# Copyright (c) 2018, Pensando Systems Inc.
#

all:
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic sdk gen_proto
	$(MAKE) ARCH=$(BUILD_ARCH) -C $(TOPDIR)/../nic/gen/proto/hal -j4

TOPDIR = ..
include $(TOPDIR)/make/Makefile.inc
