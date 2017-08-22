
TOPDIR = ../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = ipsec_rxdma_actions
OBJ_DIR = $(TOPDIR)/obj/asm_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../../include -I../../../common-p4+/include -I$(TOPDIR)/p4/ipsec-p4+
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

SRC_FILES := \
    ipsec_encap_rxdma_initial_table.s \
    allocate_input_desc_semaphore.s \
    allocate_output_desc_semaphore.s \
    allocate_input_page_semaphore.s \
    allocate_output_page_semaphore.s \
    allocate_input_desc_index.s \
    allocate_output_desc_index.s \
    allocate_input_page_index.s \
    allocate_output_page_index.s \
    update_input_desc_aol.s \
    update_output_desc_aol.s \
    ipsec_cb_tail_enqueue_input_desc.s 

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
