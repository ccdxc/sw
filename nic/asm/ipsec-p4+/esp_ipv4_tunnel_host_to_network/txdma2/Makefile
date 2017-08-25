
TOPDIR = ../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = ipsec_txdma_actions_pass2
OBJ_DIR = $(TOPDIR)/obj/asm_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../../include -I../../../common-p4+/include -I$(TOPDIR)/p4/ipsec-p4+ -I$(TOPDIR)/include
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

SRC_FILES := \
    ipsec_encap_txdma2_load_barco_req.s \
    ipsec_encap_txdma2_load_barco_req_ptr.s \
    ipsec_encap_txdma2_load_in_desc.s \
    ipsec_encap_txdma2_load_out_desc.s \
    ipsec_encap_txdma2_load_ipsec_int.s \
    ipsec_build_encap_packet.s \
    ipsec_encap_txdma2_initial_table.s

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
