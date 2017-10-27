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
	-lmisc \
	$(NULL)

TOPDIR = ../../../../..
include $(TOPDIR)/make/Makefile.inc

ifneq ("$(strip $(BUILD_ARCH))","")
LCLEANFILES += dotest.out

test: dotest.out
	$(Q)if cmp -s dotest.exp dotest.out; then \
		echo Test passed; \
		$(RM) dotest.out; \
	else \
		echo Test FAILED; \
	fi

dotest.out: $(GENLIB) $(GENBIN)/$(PROGRAM)
	$(Q)./dotest $(GENLIB) $(GENBIN)/$(PROGRAM) > dotest.out 2>&1
endif # BUILD_ARCH
