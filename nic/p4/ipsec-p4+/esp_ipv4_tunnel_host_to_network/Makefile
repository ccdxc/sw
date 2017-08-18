TOPDIR = ../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../../obj
GEN_DIR = ../../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi

IPSEC_RX_PROJ := ipsec_rxdma_actions
IPSEC_TX1_PROJ := ipsec_txdma_actions_pass1
IPSEC_TX2_PROJ := ipsec_txdma_actions_pass2

all:
	$(NCC) $(NCC_OPTIONS) $(IPSEC_RX_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_TX1_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_TX2_PROJ).p4 --gen-dir $(GEN_DIR)

clean:
	rm -rf $(GEN_DIR)/$(PROJ)

default:
	all
