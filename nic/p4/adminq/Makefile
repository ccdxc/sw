
TOPDIR = ../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.adminq_p4
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile

ADMINQ_PROJ := adminq_txdma_actions
NICMGR_PROJ := nicmgr_txdma_actions

p4_src = $(shell find . -name '*.p4')
hdr_src = $(shell find . -name '*.h')
cmn_src = $(shell find ../common-p4+ -name '*')
inc_src = $(shell find ../include -name '*')
iris_inc_src = $(shell find ../iris/include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.adminq_p4: $(p4_src) $(hdr_src) $(cmn_src) $(inc_src) $(iris_inc_src) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(ADMINQ_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(NICMGR_PROJ).p4 --gen-dir $(GEN_DIR)
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(ADMINQ_PROJ)
	rm -rf $(GEN_DIR)/$(NICMGR_PROJ)
	rm -rf $(GEN_DIR)/.adminq_p4

default:
	all
