
TOPDIR = ../..
CUSTOM = all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi 
PROJ = storage_tx
GEN_FILE = $(GEN_DIR)/$(PROJ)/asm_out/ingress.h

all: $(GEN_FILE)

$(GEN_FILE): $(PROJ).p4
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4 --gen-dir $(GEN_DIR)

