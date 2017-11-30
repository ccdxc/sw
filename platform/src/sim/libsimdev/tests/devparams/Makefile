#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = devparams
SOURCES = devparams.c

LIBRARIES = \
	-lsimdev \
	-lsimlib \
	-lpciehsvc \
	-lpciehw \
	-lpciehost \
	-lcfgspace \
	-lpal \
	-lmisc \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc

# Only run test on x86_64
ifeq ("$(strip $(BUILD_ARCH))","x86_64")
LCLEANFILES += dotest.out

test: dotest.out
	$(Q)if cmp -s dotest.exp dotest.out; then \
		echo Test passed; \
		$(RM) dotest.out; \
	else \
		echo Test FAILED; \
		exit 1; \
	fi

dotest.out: $(GENLIB) $(GENBIN)/$(PROGRAM)
	$(Q)./dotest $(GENLIB) $(GENBIN)/$(PROGRAM) > dotest.out 2>&1
endif # BUILD_ARCH
