
TOPDIR = ../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.eth_p4
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile

ETH_RX_PROJ := eth_rxdma_actions
ETH_TX_PROJ := eth_txdma_actions

p4_src = $(shell find . -name '*.p4')
hdr_src = $(shell find . -name '*.h')
cmn_src = $(shell find ../common-p4+ -name '*')
inc_src = $(shell find ../include -name '*')
nw_inc_src = $(shell find ../nw/include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.eth_p4: $(p4_src) $(hdr_src) $(cmn_src) $(inc_src) $(nw_inc_src) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(ETH_RX_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(ETH_TX_PROJ).p4 --gen-dir $(GEN_DIR)
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(ETH_RX_PROJ)
	rm -rf $(GEN_DIR)/$(ETH_TX_PROJ)
	rm -rf $(GEN_DIR)/.eth_p4

default:
	all
