
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
INCDIR = -I.. -I$(TOPDIR)/gen/nicmgr_txdma_actions/asm_out/ -I$(TOPDIR)/gen/nicmgr_txdma_actions/alt_asm_out/

OPTS = -d -b -e

all: stage0 stage1 stage2

stage0:
	$(CAPSIM) -I$(INCDIR) -P nicmgr_process_desc=0xffffffff nicmgr_fetch_desc.ctl $(BINDIR)/nicmgr_fetch_desc.bin

stage1: stage1_t0 stage1_t1

stage1_t0:
	$(CAPSIM) -I$(INCDIR) -P nicmgr_post_adminq=0xffffffff nicmgr_process_desc_hdr.ctl $(BINDIR)/nicmgr_process_desc_hdr.bin

stage1_t1:
	$(CAPSIM) -I$(INCDIR) -P nicmgr_post_adminq=0xffffffff nicmgr_process_desc_data.ctl $(BINDIR)/nicmgr_process_desc_data.bin

stage2:
	$(CAPSIM) -I$(INCDIR) -P nicmgr_commit=0xffffffff nicmgr_post_adminq.ctl $(BINDIR)/nicmgr_post_adminq.bin

stage2_ringwrap:
	$(CAPSIM) -I$(INCDIR) -P nicmgr_commit=0xffffffff nicmgr_post_adminq_ring_wrap.ctl $(BINDIR)/nicmgr_post_adminq.bin
