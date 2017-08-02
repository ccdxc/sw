
TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --asm-out --pd-gen --gen-dir $(GEN_DIR)
PROJ = iris

all:
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4

clean:
	rm -rf $(GEN_DIR)/$(PROJ)

default:
	all
