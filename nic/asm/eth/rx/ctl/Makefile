
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
INCDIR = $(TOPDIR)/gen/eth_rxdma_actions/asm_out/

OPTS = -d -b -e

all:
	$(CAPSIM) -I$(INCDIR) -P eth_rx_packet=0xffffffff eth_rx_fetch_desc.ctl $(BINDIR)/eth_rx_fetch_desc.bin
