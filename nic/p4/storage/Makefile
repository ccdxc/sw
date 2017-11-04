
TOPDIR = ../..
CUSTOM = all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile
PROJ = storage_tx
GEN_FILE = $(GEN_DIR)/$(PROJ)/asm_out/ingress.h
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)

all: $(GEN_FILE)

$(GEN_FILE): $(PROJ).p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4 --gen-dir $(GEN_DIR)

clean:
	rm -rf $(GEN_DIR)/$(PROJ)

