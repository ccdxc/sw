
TOPDIR = ../..
CUSTOM = my_all
CUSTOM_CLEAN = my_clean
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile
#TLS_PROJ = proxy
TCP_RX_PROJ = tcp_proxy_rxdma
TCP_TX_PROJ = tcp_proxy_txdma
TCP_RX_GEN_FILE = $(GEN_DIR)/$(TCP_RX_PROJ)/asm_out/ingress.h
TCP_TX_GEN_FILE = $(GEN_DIR)/$(TCP_TX_PROJ)/asm_out/ingress.h
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)

my_all: $(TCP_RX_GEN_FILE) $(TCP_TX_GEN_FILE)

$(TCP_RX_GEN_FILE): $(TCP_RX_PROJ).p4 tcp_proxy_common.p4 ../common-p4+/common_rxdma.p4 ../common-p4+/common_headers.p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(TCP_RX_PROJ).p4 --gen-dir $(GEN_DIR)

$(TCP_TX_GEN_FILE): $(TCP_TX_PROJ).p4 tcp_proxy_common.p4 ../common-p4+/common_txdma.p4 ../common-p4+/common_headers.p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(TCP_TX_PROJ).p4 --gen-dir $(GEN_DIR)

my_clean:
	rm -rf $(GEN_DIR)/$(TCP_RX_PROJ)
	rm -rf $(GEN_DIR)/$(TCP_TX_PROJ)
	rm -rf $(OBJ_DIR)/pgm_bin/$(TCP_RX_PROJ)*.bin
	rm -rf $(OBJ_DIR)/pgm_bin/$(TCP_TX_PROJ)*.bin

