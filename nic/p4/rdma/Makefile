TOPDIR = ../..
CUSTOM = my_all
CUSTOM_CLEAN = my_clean
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi 
#TLS_PROJ = proxy
RDMA_RXPROJ = rdma_rxdma
RDMA_TXPROJ = rdma_txdma
RDMA_RXGEN_FILE = $(GEN_DIR)/$(RDMA_RXPROJ)/asm_out/ingress.h
RDMA_TXGEN_FILE = $(GEN_DIR)/$(RDMA_TXPROJ)/asm_out/ingress.h
inc_deps = $(shell find ../include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)

my_all: $(RDMA_RXGEN_FILE) $(RDMA_TXGEN_FILE)

$(RDMA_RXGEN_FILE): $(RDMA_RXPROJ).p4 rdma_rxdma_headers.p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(RDMA_RXPROJ).p4 --gen-dir $(GEN_DIR)

$(RDMA_TXGEN_FILE): $(RDMA_TXPROJ).p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(RDMA_TXPROJ).p4 --gen-dir $(GEN_DIR)

my_clean:
	$(RM) -fr $(GEN_DIR)/$(RDMA_RXPROJ)
	$(RM) -fr $(GEN_DIR)/$(RDMA_TXPROJ)
