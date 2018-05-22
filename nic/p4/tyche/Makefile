TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.tyche_p4
CUSTOM_CLEAN = tyche_clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj/tyche
NCC_OPTIONS = --asm-out --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin
NCC_OPTIONS +=  --phv-flits 12 --i2e-user --single-pipe
PROJ = tyche

local_deps = $(shell find . -name '*.p4' -o -name '*.h')
inc_deps = $(shell find ../include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.tyche_p4: $(local_deps) $(inc_deps) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4
	touch $@

tyche_clean:
	rm -rf $(GEN_DIR)/$(PROJ)
	rm -rf $(OBJ_DIR)/pgm_bin/$(PROJ)*.bin
	rm -rf $(GEN_DIR)/.tyche_p4

default:
	all
