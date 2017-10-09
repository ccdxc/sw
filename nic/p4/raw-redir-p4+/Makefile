
TOPDIR = ../..
CUSTOM = all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi 
RAWR_RX_PROJ = rawr_rxdma
RAWR_TX_PROJ =
RAWR_RX_GEN_FILE = $(GEN_DIR)/$(RAWR_RX_PROJ)/asm_out/ingress.h

all: $(RAWR_RX_GEN_FILE)

clean:
	rm -rf $(GEN_DIR)/$(RAWR_RX_PROJ)

$(RAWR_RX_GEN_FILE): $(RAWR_RX_PROJ).p4 rawr_defines.h
	$(NCC) $(NCC_OPTIONS) $(RAWR_RX_PROJ).p4 --gen-dir $(GEN_DIR)


