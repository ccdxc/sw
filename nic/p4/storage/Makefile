
TOPDIR = ../..
CUSTOM = all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile
STORAGE_TX_PROJ = storage_tx
STORAGE_NVME_PROJ = storage_nvme
STORAGE_TX_GEN_FILE = $(GEN_DIR)/$(STORAGE_TX_PROJ)/asm_out/ingress.h
STORAGE_NVME_GEN_FILE = $(GEN_DIR)/$(STORAGE_NVME_PROJ)/asm_out/ingress.h
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)

all: $(STORAGE_TX_GEN_FILE) $(STORAGE_NVME_GEN_FILE)

$(STORAGE_TX_GEN_FILE): $(STORAGE_TX_PROJ).p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(STORAGE_TX_PROJ).p4 --gen-dir $(GEN_DIR)

$(STORAGE_NVME_GEN_FILE): $(STORAGE_NVME_PROJ).p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(STORAGE_NVME_PROJ).p4 --gen-dir $(GEN_DIR)

clean:
	rm -rf $(GEN_DIR)/$(STORAGE_TX_PROJ)
	rm -rf $(GEN_DIR)/$(STORAGE_NVME_PROJ)

