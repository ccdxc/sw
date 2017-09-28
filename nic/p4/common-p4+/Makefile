TOPDIR = ../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.cmn_p4plus
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --asm-out --p4-plus --no-ohi --pd-gen --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin

p4_src = $(shell find $(SOURCEDIR) -name '*.p4')
hdr_src = $(shell find $(SOURCEDIR) -name '*.h')
inc_src = $(shell find ../include -name '*')
nw_inc_src = $(shell find ../nw/include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.cmn_p4plus: $(p4_src) $(hdr_src) $(inc_src) $(nw_inc_src) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) --p4-plus-module rxdma common_rxdma_actions.p4
	$(NCC) $(NCC_OPTIONS) --p4-plus-module txdma common_txdma_actions.p4
	touch $@

clean:
	rm -rf $(GEN_DIR)/common_rxdma_actions
	rm -rf $(GEN_DIR)/common_txdma_actions
	rm -rf $(OBJ_DIR)/pgm_bin/rxdma*.bin
	rm -rf $(OBJ_DIR)/pgm_bin/txdma*.bin
	rm -rf $(GEN_DIR)/.cmn_p4plus

default:
	all
