TOPDIR = ../../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.ipsec_ntoh_p4
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../../obj
GEN_DIR = ../../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile

IPSEC_RX_PROJ := esp_v4_tunnel_n2h_rxdma
IPSEC_TX1_PROJ := esp_v4_tunnel_n2h_txdma1
IPSEC_TX2_PROJ := esp_v4_tunnel_n2h_txdma2

p4_src = $(shell find . -name '*.p4')
hdr_src = $(shell find .. -name '*.h')
cmn_src = $(shell find ../../common-p4+ -name '*')
inc_src = $(shell find ../../include -name '*')
iris_inc_src = $(shell find ../../iris/include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)
$(GEN_DIR)/.ipsec_ntoh_p4: $(p4_src) $(hdr_src) $(cmn_src) $(inc_src) $(iris_inc_src) $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_RX_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_TX1_PROJ).p4 --gen-dir $(GEN_DIR)
	$(NCC) $(NCC_OPTIONS) $(IPSEC_TX2_PROJ).p4 --gen-dir $(GEN_DIR)
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(IPSEC_RX_PROJ)
	rm -rf $(GEN_DIR)/$(IPSEC_TX1_PROJ)
	rm -rf $(GEN_DIR)/$(IPSEC_TX2_PROJ)
	rm -rf $(GEN_DIR)/.ipsec_ntoh_p4

default:
	all
