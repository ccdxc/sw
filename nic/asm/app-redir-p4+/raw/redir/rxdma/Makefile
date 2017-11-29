
TOPDIR = ../../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = rawr_rxdma
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
P4_DIR = $(TOPDIR)/p4/app-redir-p4+
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
COMMON_APP_REDIR_DIR = ../../../common/include
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../../../../common-p4+/include \
           -I$(TOPDIR)/include -I$(COMMON_APP_REDIR_DIR)
SRC_DIR = .
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
NCC_BIN = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --p4-plus --asm-out --no-ohi

all: deps asm

deps: 

SRC_FILES := \
    $(SRC_DIR)/rawr_rx_start.s \
    $(SRC_DIR)/rawr_desc_sem_post_update.s \
    $(SRC_DIR)/rawr_desc_post_alloc.s \
    $(SRC_DIR)/rawr_ppage_sem_post_update.s \
    $(SRC_DIR)/rawr_ppage_post_alloc.s \
    $(SRC_DIR)/rawr_mpage_sem_post_update.s \
    $(SRC_DIR)/rawr_mpage_post_alloc.s \
    $(SRC_DIR)/rawr_chain_qidxr_stage_advance_a.s \
    $(SRC_DIR)/rawr_chain_qidxr_stage_advance_b.s \
    $(SRC_DIR)/rawr_chain_pindex_pre_alloc.s \
    $(SRC_DIR)/rawr_chain_txq_pindex_post_read.s \
    $(SRC_DIR)/rawr_chain_qidxr_post_read.s \
    $(SRC_DIR)/rawr_cleanup_discard.s \
    $(SRC_DIR)/rawr_desc_free.s \
    $(SRC_DIR)/rawr_ppage_free.s \
    $(SRC_DIR)/rawr_mpage_free.s \
    $(SRC_DIR)/rawr_chain_xfer.s \
	


BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

%.h: $(P4_DIR)/$(PROJ).p4 $(COMMON_P4_DIR)/common_headers.p4
	$(NCC_BIN) $(NCC_OPTIONS) $(P4_DIR)/$(PROJ).p4 --gen-dir $(GEN_DIR)
	

asm: $(BIN_FILES)

#TEST_FILES := $(patsubst %.s, %.ctl, $(SRC_FILES))
TEST_FILES := \

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

rawr_initial_action_ARGS :=

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
