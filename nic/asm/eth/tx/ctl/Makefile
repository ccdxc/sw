
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(OBJDIR)/p4plus_bin/
INCDIR = $(TOPDIR)/gen/eth_txdma_actions/asm_out/

OPTS = -d -b -e

all:
	$(GDB) $(CAPSIM) -I$(INCDIR) -P eth_tx_packet=0xffffffff eth_tx_fetch_desc.ctl $(BINDIR)/eth_tx_fetch_desc.bin
