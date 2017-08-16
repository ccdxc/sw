TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --asm-out --p4-plus --no-ohi --pd-gen --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin

all:
	$(NCC) $(NCC_OPTIONS) --p4-plus-module rxdma common_rxdma_actions.p4
	$(NCC) $(NCC_OPTIONS) --p4-plus-module txdma common_txdma_actions.p4

clean:
	rm -rf $(GEN_DIR)/common_rxdma_actions
	rm -rf $(GEN_DIR)/common_txdma_actions
	rm -rf $(OBJ_DIR)/pgm_bin/rxdma*.bin
	rm -rf $(OBJ_DIR)/pgm_bin/txdma*.bin

default:
	all
