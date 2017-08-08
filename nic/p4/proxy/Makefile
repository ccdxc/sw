
TOPDIR = ../..
CUSTOM = all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi 
#TLS_PROJ = proxy
TCP_RX_PROJ = tcp_proxy_rxdma
TCP_TX_PROJ = tcp_proxy_txdma

all:
	#$(NCC) $(NCC_OPTIONS) $(TLS_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(TCP_RX_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(TCP_TX_PROJ).p4 --gen-dir $(GEN_DIR)

default:
	all
