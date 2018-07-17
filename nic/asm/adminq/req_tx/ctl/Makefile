
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
INCDIR = -I.. -I$(TOPDIR)/gen/adminq_txdma_actions/asm_out/ -I$(TOPDIR)/gen/adminq_txdma_actions/alt_asm_out/

OPTS = -d -b -e

all: stage0 stage1 stage2 stage3

stage0:
	$(CAPSIM) -I$(INCDIR) -P adminq_process_desc=0xffffffff adminq_fetch_desc.ctl $(BINDIR)/adminq_fetch_desc.bin

stage1:
	$(CAPSIM) -I$(INCDIR) -P adminq_post_nicmgr=0xffffffff adminq_process_desc.ctl $(BINDIR)/adminq_process_desc.bin

stage2:
	$(CAPSIM) -I$(INCDIR) -P adminq_commit=0xffffffff adminq_post_nicmgr.ctl $(BINDIR)/adminq_post_nicmgr.bin

stage3:
	$(CAPSIM) -I$(INCDIR) adminq_commit.ctl $(BINDIR)/adminq_commit.bin
