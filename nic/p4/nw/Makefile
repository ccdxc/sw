TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.nw_p4
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
NCC_OPTIONS = --asm-out --pd-gen --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin
PROJ = iris

#p4_src = $(wildcard ./*.p4)
p4_src = $(shell find . -name '*.p4')
hdr_src = $(shell find . -name '*.h')
inc_src = $(shell find ../include -name '*')
$(GEN_DIR)/.nw_p4: $(p4_src) $(hdr_src) $(inc_src)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(PROJ)
	rm -rf $(OBJ_DIR)/pgm_bin/$(PROJ)*.bin
	rm -rf $(GEN_DIR)/.nw_p4

default:
	all
