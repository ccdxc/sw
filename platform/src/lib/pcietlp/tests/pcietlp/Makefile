#
# Copyright (c) 2018, Pensando Systems Inc.
#

PROGRAM = pcietlp
SOURCES = pcietlp.c
LBUILD_ARCHES = x86_64

LCFLAGS += \
	$(NULL)

LIBRARIES = \
	-lpcietlp \
	-lmisc \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc

# Only run test on x86_64
ifeq ("$(strip $(BUILD_ARCH))","x86_64")

.PHONY: run_tests test.enc test.dec

run_tests: test.enc test.dec

test.enc:
	$(Q)run_enc_tests
	$(Q)verify_enc_tests

test.dec:
	$(Q)run_dec_tests

endif # BUILD_ARCH
