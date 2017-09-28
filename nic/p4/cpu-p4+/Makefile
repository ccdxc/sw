TOPDIR = ../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.cpu_p4plus
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --gen-dir $(GEN_DIR)

CPU_RX_PROJ = cpu_rxdma
CPU_TX_PROJ = cpu_txdma

p4_src = $(shell find . -name '*.p4')
hdr_src = $(shell find . -name '*.h')
cmn_src = $(shell find ../common-p4+ -name '*')
inc_src = $(shell find ../include -name '*')
nw_inc_src = $(shell find ../nw/include -name '*')
$(GEN_DIR)/.cpu_p4plus: $(p4_src) $(hdr_src) $(cmn_src) $(inc_src) $(nw_inc_src)
	$(NCC) $(NCC_OPTIONS) $(CPU_RX_PROJ).p4 
	$(NCC) $(NCC_OPTIONS) $(CPU_TX_PROJ).p4 
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(CPU_RX_PROJ)
	rm -rf $(GEN_DIR)/$(CPU_TX_PROJ)
	rm -rf $(GEN_DIR)/.cpu_p4plus

default:
	all
