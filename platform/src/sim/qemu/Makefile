#
# Copyright (c) 2017, Pensando Systems Inc.
#

QEMU_VERSION ?= 2.9.0
QEMU = qemu-$(QEMU_VERSION)
QEMU_TARBALL = $(QEMU).tar.xz
QEMU_PATCH = qemu.patch

BUILDENV = 1
TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc

ifneq ("$(strip $(BUILD_ARCH))","")

all: $(QEMU) apply-patch configure build

clean:
	$(RM) -r $(QEMU)
	$(RM) $(QEMU_TARBALL)
	$(RM) .patched .configured

endif # BUILD_ARCH

#
# Prepare for this with this manual sequence.
#     make -C $(QEMU) distclean
#     mv $(QEMU) $(QEMU)-new
#     tar xf $(QEMU_TARBALL)
#     mv $(QEMU) $(QEMU)-old
#
gen-patch: $(QEMU)-old $(QEMU)-new
	-diff -uprN $(QEMU)-old $(QEMU)-new > $(QEMU_PATCH)

apply-patch: .patched

.patched:
	(cd $(QEMU) && patch -p1 -f) < $(QEMU_PATCH) && touch $@

configure: .configured

.configured:
	(cd $(QEMU) && ./configure --target-list=x86_64-softmmu --enable-debug-info) && \
	touch $@

build: $(QEMU)
	$(MAKE) -C $(QEMU) PEN_TOP=$(shell (cd $(TOPDIR); /bin/pwd))

$(QEMU): $(QEMU_TARBALL)
	tar xf $?

$(QEMU_TARBALL):
	curl -sO https://download.qemu.org/$@

.PHONY: all build gen-patch apply-patch configure clean
