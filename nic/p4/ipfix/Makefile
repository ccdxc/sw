TOPDIR = ../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.ipfix_p4
CUSTOM_CLEAN = clean_p4
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile
PROJ = ipfix
GEN_FILE = $(GEN_DIR)/$(PROJ)/asm_out/ingress.h

P4FILES = \
	$(PROJ).p4 \
	ipfix_metadata.p4 \
	$(NULL)

build_p4: $(GEN_FILE)

p4_src = $(shell find . -name '*.p4')
hdr_src = $(shell find . -name '*.h')
cmn_src = $(shell find ../common-p4+ -name '*')
inc_src = $(shell find ../include -name '*')
nw_inc_src = $(shell find ../nw/include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.ipfix_p4: $(p4_src) $(hdr_src) $(cmn_src) $(inc_src) $(nw_inc_src) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4 --gen-dir $(GEN_DIR)
	touch $@

clean_p4:
	rm -rf $(GEN_DIR)/$(PROJ)
	rm -rf $(GEN_DIR)/.ipfix_p4
