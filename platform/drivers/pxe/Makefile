#
# Copyright (c) 2017-2019, Pensando Systems Inc.
#
TOP = ../../..
IPXEDIR = $(TOP)/platform/gen/ipxe

all: copyfiles buildipxe buildipxeefi

copyfiles:
	cp -f $(CURDIR)/ionic* $(CURDIR)/ipxe/src/drivers/net/
	cp -f $(CURDIR)/../common/ionic* $(CURDIR)/ipxe/src/drivers/net/

buildipxe:
	make PLATFORM=efi ARCH=x86_64 -C $(CURDIR)/ipxe/src bin-x86_64-efi/ionic.efirom
	cp -f $(CURDIR)/ipxe/src/bin-x86_64-efi/ionic.efirom $(BLD_BIN_DIR)/.

buildipxeefi:
	rm -rf $(IPXEDIR)
	mkdir -p $(IPXEDIR)
	make PLATFORM=efi ARCH=x86_64 -C $(CURDIR)/ipxe/src bin-x86_64-efi/ionic.efi
	cp -f $(CURDIR)/ipxe/src/bin-x86_64-efi/ionic.efi $(IPXEDIR)/.

clean:
	make PLATFORM=efi ARCH=x86_64 -C $(CURDIR)/ipxe/src bin-x86_64-efi/ionic.efirom clean

.PHONY: printvars
printvars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
