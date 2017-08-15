
TOPDIR = ../..
CUSTOM = my_all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi 
#TLS_PROJ = proxy
TCP_RX_PROJ = tcp_proxy_rxdma
TCP_TX_PROJ = tcp_proxy_txdma
TCP_RX_GEN_FILE = $(GEN_DIR)/$(TCP_RX_PROJ)/asm_out/ingress.h
TCP_TX_GEN_FILE = $(GEN_DIR)/$(TCP_TX_PROJ)/asm_out/ingress.h

my_all: $(TCP_RX_GEN_FILE) $(TCP_TX_GEN_FILE)

$(TCP_RX_GEN_FILE): $(TCP_RX_PROJ).p4
	$(NCC) $(NCC_OPTIONS) $(TCP_RX_PROJ).p4 --gen-dir $(GEN_DIR)

$(TCP_TX_GEN_FILE): $(TCP_TX_PROJ).p4
	$(NCC) $(NCC_OPTIONS) $(TCP_TX_PROJ).p4 --gen-dir $(GEN_DIR)

