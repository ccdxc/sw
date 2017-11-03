#
# Copyright (c) 2017, Pensando Systems Inc.
#

# XXX fix build for aarch64
LBUILD_ARCHES = x86_64

PCIUTILS_VERSION ?= 3.5.5
PCIUTILS = pciutils-$(PCIUTILS_VERSION)
PCIUTILS_TARBALL = $(PCIUTILS).tar.gz
PCIUTILS_PATCH = pciutils.patch
PROGS = lspci setpci
GENPROGS = $(PROGS:%=$(GENBIN)/%)

BUILDENV = 1
TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc

ifneq ("$(strip $(BUILD_ARCH))","")

all: $(PCIUTILS) apply-patch build genprogs

clean:
	$(RM) -r $(PCIUTILS) $(PCIUTILS)-orig
	$(RM) $(PCIUTILS_TARBALL)
	$(RM) $(GENPROGS)
	$(RM) .patched

endif # BUILD_ARCH

gen-patch: $(PCIUTILS)-orig $(PCIUTILS) clean-pciutils
	-diff -uprN $(PCIUTILS)-orig $(PCIUTILS) > $(PCIUTILS_PATCH)

apply-patch: .patched

.patched:
	(cd $(PCIUTILS) && patch -p1 -f) < $(PCIUTILS_PATCH) && touch $@

build: $(PCIUTILS)
	$(MAKE) -C $(PCIUTILS) PEN_TOP=../$(TOPDIR) BUILD_ARCH=$(BUILD_ARCH)

genprogs: $(GENPROGS)

$(PCIUTILS): $(PCIUTILS_TARBALL)
	tar xf $?

$(PCIUTILS_TARBALL):
	curl -sO ftp://atrey.karlin.mff.cuni.cz/pub/linux/pci/$@

$(GENBIN)/lspci: $(PCIUTILS)/lspci
	cp $? $@

$(GENBIN)/setpci: $(PCIUTILS)/setpci
	cp $? $@

clean-pciutils:
	$(MAKE) -C $(PCIUTILS) clean

.PHONY: all build genprogs gen-patch apply-patch reverse-patch clean
