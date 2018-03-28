
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
PROJ = eth_txdma_actions
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
ALT_ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/alt_asm_out/
INC_DIRS = -I. -I.. -I$(ASM_HDR_DIR) -I$(ALT_ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include -I$(TOPDIR)/p4/include -I$(TOPDIR)/include
SRC_DIR = .

OPTS = -d -b -e

eth_tx_queue_disabled:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_queue_disabled.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_spurious_db:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_spurious_db.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_spurious_db_1:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_spurious_db_1.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_fetch_desc_1:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_fetch_desc_1.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_fetch_desc_2:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_fetch_desc_2.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_fetch_desc_3:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_fetch_desc_3.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_fetch_desc_4:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_fetch_desc_4.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_fetch_desc_5:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_fetch_desc_5.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_ring_wrap:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_ring_wrap.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_ring_end:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_ring_end.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_ring_full:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_ring_full.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_cq_wrap:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_prep=0xffffffff eth_tx_cq_wrap.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx_1:
	$(CAPSIM) $(INC_DIRS) -P eth_tx_start=0xffffffff -P eth_tx_completion=0xffffffff eth_tx_1.ctl $(BINDIR)/eth_tx.bin
