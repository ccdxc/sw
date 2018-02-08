
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
PROJ = eth_txdma_actions
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/

OPTS = -d -b -e

eth_tx_fetch_desc:
	$(CAPSIM) -I.. -I../tx/ -I$(ASM_HDR_DIR) -I$(TOPDIR)/gen/$(PROJ)/alt_asm_out/  -I$(TOPDIR)/asm/common-p4+/include  -I$(TOPDIR)/p4/include -I$(TOPDIR)/include -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_1.ctl $(BINDIR)/eth_tx_fetch_desc.bin
	$(CAPSIM) -I.. -I../tx/ -I$(ASM_HDR_DIR) -I$(TOPDIR)/gen/$(PROJ)/alt_asm_out/  -I$(TOPDIR)/asm/common-p4+/include  -I$(TOPDIR)/p4/include -I$(TOPDIR)/include -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_2.ctl $(BINDIR)/eth_tx_fetch_desc.bin
	$(CAPSIM) -I.. -I../tx/ -I$(ASM_HDR_DIR) -I$(TOPDIR)/gen/$(PROJ)/alt_asm_out/  -I$(TOPDIR)/asm/common-p4+/include  -I$(TOPDIR)/p4/include -I$(TOPDIR)/include -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_3.ctl $(BINDIR)/eth_tx_fetch_desc.bin
	$(CAPSIM) -I.. -I../tx/ -I$(ASM_HDR_DIR) -I$(TOPDIR)/gen/$(PROJ)/alt_asm_out/  -I$(TOPDIR)/asm/common-p4+/include  -I$(TOPDIR)/p4/include -I$(TOPDIR)/include -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_4.ctl $(BINDIR)/eth_tx_fetch_desc.bin

eth_tx:
	$(CAPSIM) -I.. -I../tx/ -I$(ASM_HDR_DIR) -I$(TOPDIR)/gen/$(PROJ)/alt_asm_out/  -I$(TOPDIR)/asm/common-p4+/include  -I$(TOPDIR)/p4/include -I$(TOPDIR)/include -P eth_tx_completion=0xffffffff eth_tx_1.ctl $(BINDIR)/eth_tx.bin
