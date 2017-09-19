
TOPDIR = ../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = esp_ipv4_tunnel_h2n_txdma2
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../../include -I../../../common-p4+/include -I$(TOPDIR)/p4/ipsec-p4+ -I$(TOPDIR)/include -I../..
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

SRC_FILES := \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req.s \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req_ptr.s \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc.s \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_out_desc.s \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int.s \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet.s \
    esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table.s

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
