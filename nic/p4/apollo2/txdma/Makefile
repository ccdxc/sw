TOPDIR = ../../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.apollo_txdma
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = $(TOPDIR)/tools/ncc/capri-ncc.py
OBJ_DIR = $(TOPDIR)/obj/apollo_txdma
NCC_OPTIONS = --asm-out --p4-plus --no-ohi --two-byte-profile --pd-gen --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin
PROJ = apollo_txdma

p4_src = $(shell find $(SOURCEDIR) -name '*.p4')
hdr_src = $(shell find $(SOURCEDIR) -name '*.h')
inc_src = $(shell find ../include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.apollo_txdma: $(p4_src) $(hdr_src) $(inc_src) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) --p4-plus-module txdma $(PROJ).p4
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(PROJ)
	rm -rf $(OBJ_DIR)/pgm_bin/$(PROJ)*.bin
	rm -rf $(GEN_DIR)/.apollo_txdma

default:
	all
