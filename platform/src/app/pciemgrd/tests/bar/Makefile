#
# Copyright (c) 2018, Pensando Systems Inc.
#

LBUILD_ARCHES = x86_64

BUILDENV = 1
TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc

# Only run test on x86_64
ifeq ("$(strip $(BUILD_ARCH))","x86_64")

.PHONY: test

test:
	./run_tests

endif # BUILD_ARCH
