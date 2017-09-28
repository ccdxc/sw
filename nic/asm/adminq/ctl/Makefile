
TOPDIR = ../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
INCDIR = $(TOPDIR)/gen/adminq_txdma_actions/asm_out/

OPTS = -d -b -e

all: stage0 stage1

stage0:
	$(CAPSIM) -I$(INCDIR) -I.. -P adminq_operation=0xffffffff adminq_fetch_desc.ctl $(BINDIR)/adminq_fetch_desc.bin

stage1:
	$(CAPSIM) -I$(INCDIR) -I.. adminq_operation.ctl $(BINDIR)/adminq_operation.bin
