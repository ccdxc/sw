
TOPDIR = ../../../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.esp_ntoh_tx1_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = esp_v4_tunnel_n2h_txdma1
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../../include -I../../../common-p4+/include -I$(TOPDIR)/p4/ipsec-p4+ -I$(TOPDIR)/include -I../..


CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
    esp_ipv4_tunnel_n2h_allocate_barco_req_pindex.s \
    esp_ipv4_tunnel_n2h_load_head_desc_int_header.s \
    esp_ipv4_tunnel_n2h_txdma_initial_table.s \
    esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex.s \
    esp_v4_tunnel_n2h_txdma1_update_cb.s \
    esp_ipv4_tunnel_n2h_stage0.s \
    esp_ipv4_tunnel_n2h_write_barco_req.s 

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.esp_ntoh_tx1_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')
$(OBJ_DIR)/%.bin: %.s $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@

#BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))
#
#$(BIN_FILES): %.bin: %.s
#	@echo $@
#	@mkdir -p $(OBJ_DIR)
#	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)
#
#asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.esp_ntoh_tx1_asm
