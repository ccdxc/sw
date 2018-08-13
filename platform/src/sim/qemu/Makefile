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
	$(RM) -r $(QEMU) $(QEMU)-old $(QEMU)-new
	$(RM) .patched .configured
	$(RM) $(QEMU_TARBALL)

endif # BUILD_ARCH

gen-patch: $(QEMU)
	mv $(QEMU) b
	tar xf $(QEMU_TARBALL) && mv $(QEMU) a
	./mkpatch > $(QEMU_PATCH)
	$(RM) -r a
	mv b $(QEMU)

apply-patch: .patched

.patched:
	(cd $(QEMU) && patch -p1 -f) < $(QEMU_PATCH) && touch $@

configure: .configured

.configured:
	(cd $(QEMU) && \
	./configure --target-list=x86_64-softmmu --enable-debug-info) && \
	touch $@

build: $(QEMU)
	$(MAKE) -C $(QEMU) PEN_TOP=$(shell (cd $(TOPDIR); /bin/pwd))

$(QEMU): $(QEMU_TARBALL) $(QEMU_PATCH)
	$(RM) -r $(QEMU) .patched .configured
	tar xf $(QEMU_TARBALL)

$(QEMU_TARBALL):
	curl -sO https://download.qemu.org/$@

.PHONY: all build gen-patch apply-patch configure clean
