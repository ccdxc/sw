TOPDIR = ../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../../obj
GEN_DIR = ../../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi

IPSEC_RX_PROJ := esp_ipv4_tunnel_h2n_rxdma
IPSEC_TX1_PROJ := esp_ipv4_tunnel_h2n_txdma1
IPSEC_TX2_PROJ := esp_ipv4_tunnel_h2n_txdma2

all:
	$(NCC) $(NCC_OPTIONS) $(IPSEC_RX_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_TX1_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_TX2_PROJ).p4 --gen-dir $(GEN_DIR)

clean:
	rm -rf $(GEN_DIR)/$(IPSEC_RX_PROJ)
	rm -rf $(GEN_DIR)/$(IPSEC_TX1_PROJ)
	rm -rf $(GEN_DIR)/$(IPSEC_TX2_PROJ)

default:
	all
