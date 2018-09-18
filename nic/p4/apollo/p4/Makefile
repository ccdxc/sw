TOPDIR = ../../..
SWDIR = $(TOPDIR)/../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.apollo_p4
CUSTOM_CLEAN = apollo_clean

include $(TOPDIR)/make/Makefile.inc

NCC = $(TOPDIR)/tools/ncc/capri-ncc.py
PROJ = apollo
OBJ_DIR = $(TOPDIR)/obj/$(PROJ)_p4
NCC_OPTIONS = --asm-out --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin --i2e-user
NCC_OPTIONS += --fe-flags="-I$(SWDIR)"

local_deps = $(shell find . -name '*')
inc_deps = $(shell find $(TOPDIR)/p4/include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.apollo_p4: $(local_deps) $(inc_deps) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4
	touch $@

apollo_clean:
	rm -rf $(GEN_DIR)/$(PROJ)
	rm -rf $(OBJ_DIR)/pgm_bin/$(PROJ)*.bin
	rm -rf $(GEN_DIR)/.apollo_p4

default:
	all
