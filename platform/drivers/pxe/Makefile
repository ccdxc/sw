#
# Copyright (c) 2017-2019, Pensando Systems Inc.
#
TOP = ../../..
IPXEGENDIR = $(TOP)/platform/gen/ipxe
EFI_PCI_IPXE = $(TOP)/platform/drivers/pxe/efi_pci.c
IONIC_IPXE = $(wildcard $(TOP)/platform/drivers/pxe/*.c) $(wildcard $(TOP)/platform/drivers/pxe/*.h)
IONIC_COMMON = $(wildcard $(TOP)/platform/drivers/common/*.h)
IONIC_SRC = $(IONIC_IPXE) $(IONIC_COMMON)
IPXE_TAR = ipxe-1cdf56f.tar.gz
IPXEORG = $(TOP)/platform/drivers/pxe/ipxe-org
IPXEMOD = $(TOP)/platform/drivers/pxe/ipxe-mod

all: ionic-oprom ionic-efiapp

ionic-efiapp: $(IPXEGENDIR)/ionic.efi

ionic-oprom: $(BLD_BIN_DIR)/ionic.efirom

ionic-ipxepatch: $(IPXEGENDIR)/ionic-ipxe.patch

$(IPXEGENDIR):
	mkdir -p $(IPXEGENDIR)

$(IPXEGENDIR)/ionic.efi: $(IPXEMOD)/src/bin-x86_64-efi/ionic.efi $(IPXEGENDIR)
	cp $(IPXEMOD)/src/bin-x86_64-efi/ionic.efi $@

$(BLD_BIN_DIR)/ionic.efirom: $(IPXEMOD)/src/bin-x86_64-efi/ionic.efirom
	cp $(IPXEMOD)/src/bin-x86_64-efi/ionic.efirom $@

$(IPXEMOD)/src/bin-x86_64-efi/ionic.efi: .ipxefiles
	make PLATFORM=efi ARCH=x86_64 -C $(IPXEMOD)/src bin-x86_64-efi/ionic.efi
#	make PLATFORM=efi ARCH=x86_64 -C $(IPXEMOD)/src bin-x86_64-efi/ionic.efi DEBUG=ionic:3,ionic_main:3

$(IPXEMOD)/src/bin-x86_64-efi/ionic.efirom: .ipxefiles
	make PLATFORM=efi ARCH=x86_64 -C $(IPXEMOD)/src bin-x86_64-efi/ionic.efirom
#	make PLATFORM=efi ARCH=x86_64 -C $(IPXEMOD)/src bin-x86_64-efi/ionic.efirom DEBUG=ionic:3,ionic_main:3

.ipxefiles: $(IPXEMOD) $(IONIC_SRC)
	cp $(IONIC_SRC) $(IPXEMOD)/src/drivers/net/
	cp $(EFI_PCI_IPXE) $(IPXEMOD)/src/interface/efi/
	touch $@

$(IPXEMOD):
	mkdir -p $(IPXEMOD); cd $(IPXEMOD); tar xf ../$(IPXE_TAR) --strip-components=1

$(IPXEGENDIR)/ionic-ipxe.patch: $(IPXEORG) .ipxefiles $(IPXEGENDIR)
	cd $(IPXEMOD)/src; make veryclean
	-diff -uprN $(IPXEORG) $(IPXEMOD) > $@

$(IPXEORG):
	mkdir -p $(IPXEORG); cd $(IPXEORG); tar xf ../$(IPXE_TAR) --strip-components=1

clean:
	rm -rf $(IPXEORG) $(IPXEMOD) $(IPXEGENDIR) $(BLD_BIN_DIR)/ionic.efirom .ipxefiles

.PHONY: all ionic-efiapp ionic-oprom ionic-ipxepatch clean
