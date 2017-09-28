
TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi

ADMINQ_TX_PROJ := adminq_txdma_actions

all:
	$(NCC) $(NCC_OPTIONS) $(ADMINQ_TX_PROJ).p4 --gen-dir $(GEN_DIR)

clean:
	rm -rf $(GEN_DIR)/$(PROJ)

default:
	all
