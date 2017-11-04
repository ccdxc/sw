TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.adminq_p4
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile

ADMINQ_TX_PROJ := adminq_txdma_actions

local_deps = $(shell find . -name '*.p4' -o -name '*.h')
inc_deps = $(shell find ../include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.adminq_p4: $(local_deps) $(inc_deps) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(ADMINQ_TX_PROJ).p4 --gen-dir $(GEN_DIR)
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(ADMINQ_TX_PROJ)
	rm -rf $(GEN_DIR)/.adminq_p4

default:
	all
