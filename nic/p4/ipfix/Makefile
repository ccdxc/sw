TOPDIR = ../..
CUSTOM = build_p4
CUSTOM_CLEAN = clean_p4
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi
PROJ = ipfix
GEN_FILE = $(GEN_DIR)/$(PROJ)/asm_out/ingress.h

P4FILES = \
	$(PROJ).p4 \
	ipfix_metadata.p4 \
	$(NULL)

build_p4: $(GEN_FILE)

$(GEN_FILE): $(P4FILES)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4 --gen-dir $(GEN_DIR)

clean_p4:
	rm -rf $(GEN_DIR)/$(PROJ)
