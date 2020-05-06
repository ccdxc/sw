#
# Copyright (c) 2017, Pensando Systems Inc.
#

# XXX fix build for aarch64
LBUILD_ARCHES = x86_64

PCIUTILS_VERSION ?= 3.5.6
PCIUTILS = pciutils-$(PCIUTILS_VERSION)
PCIUTILS_TARBALL = $(PCIUTILS).tar.gz
PCIUTILS_PATCH = pciutils.patch
PROGS = lspci setpci
GENPROGS = $(PROGS:%=$(GENBIN)/%)

BUILDENV = 1
TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc

ifneq ("$(strip $(BUILD_ARCH))","")

all: build genprogs

clean:
	$(RM) -r $(PCIUTILS) $(PCIUTILS)-orig
	$(RM) $(GENPROGS)

endif # BUILD_ARCH

gen-patch: $(PCIUTILS)-orig $(PCIUTILS) clean-pciutils
	-diff -uprN $(PCIUTILS)-orig $(PCIUTILS) > $(PCIUTILS_PATCH)

build: $(PCIUTILS)
	$(MAKE) -C $(PCIUTILS) \
		PEN_TOP=../$(TOPDIR) BUILD_ARCH=$(BUILD_ARCH) ASIC=$(ASIC)

genprogs: $(GENPROGS)

$(PCIUTILS): $(PCIUTILS_TARBALL) $(PCIUTILS_PATCH)
	$(RM) -r $(PCIUTILS)
	tar xf $(PCIUTILS_TARBALL)
	(cd $(PCIUTILS) && patch -p1 -f) < $(PCIUTILS_PATCH)

$(PCIUTILS)-orig: $(PCIUTILS_TARBALL)
	mkdir $(PCIUTILS)-orig
	cd $(PCIUTILS)-orig; tar xf ../$(PCIUTILS_TARBALL) --strip-components=1

$(PCIUTILS_TARBALL):
	curl -sO ftp://atrey.karlin.mff.cuni.cz/pub/linux/pci/$@

$(GENBIN)/lspci: $(PCIUTILS)/lspci
	mkdir -p $(GENBIN)
	cp $? $@

$(GENBIN)/setpci: $(PCIUTILS)/setpci
	mkdir -p $(GENBIN)
	cp $? $@

clean-pciutils:
	$(MAKE) -C $(PCIUTILS) clean

.PHONY: all build genprogs gen-patch apply-patch reverse-patch clean
