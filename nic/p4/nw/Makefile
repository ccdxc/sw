TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.nw_p4
CUSTOM_CLEAN = p4_clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
NCC_OPTIONS = --asm-out --pd-gen --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin
PROJ = iris

ifeq ($(PLATFORM),haps)
    NCC_OPTIONS += --target=haps --fe-flags=-DPLATFORM_HAPS
endif

#p4_src = $(wildcard ./*.p4)
#hdr_src = $(shell find . -name '*.h')
local_deps = $(shell find . -name '*.p4' -o -name '*.h')
inc_deps = $(shell find ../include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.nw_p4: $(local_deps) $(inc_deps) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4
	touch $@

p4_clean:
	rm -rf $(GEN_DIR)/$(PROJ)
	rm -rf $(OBJ_DIR)/pgm_bin/$(PROJ)*.bin
	rm -rf $(GEN_DIR)/.nw_p4

default:
	all
