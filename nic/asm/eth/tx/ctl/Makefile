
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
INCDIR = $(TOPDIR)/gen/eth_txdma_actions/asm_out/

OPTS = -d -b -e

eth_tx_fetch_desc:
	$(CAPSIM) -I$(INCDIR) -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_1.ctl $(BINDIR)/eth_tx_fetch_desc.bin
	$(CAPSIM) -I$(INCDIR) -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_2.ctl $(BINDIR)/eth_tx_fetch_desc.bin
	$(CAPSIM) -I$(INCDIR) -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_3.ctl $(BINDIR)/eth_tx_fetch_desc.bin
	$(CAPSIM) -I$(INCDIR) -P eth_tx_packet=0xffffffff eth_tx_fetch_desc_4.ctl $(BINDIR)/eth_tx_fetch_desc.bin
