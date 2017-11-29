
TOPDIR = ../../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = proxyr_txdma
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
    $(SRC_DIR)/proxyr_tx_start.s \
    $(SRC_DIR)/proxyr_my_txq_entry_consume.s \
    $(SRC_DIR)/proxyr_desc_post_read.s \
    $(SRC_DIR)/proxyr_mpage_sem_pindex_post_update.s \
    $(SRC_DIR)/proxyr_mpage_post_alloc.s \
    $(SRC_DIR)/proxyr_flow_key_post_read.s \
    $(SRC_DIR)/proxyr_chain_pindex_pre_alloc.s \
    $(SRC_DIR)/proxyr_chain_xfer.s \
    $(SRC_DIR)/proxyr_cleanup_discard.s \
    $(SRC_DIR)/proxyr_desc_free.s \
    $(SRC_DIR)/proxyr_ppage_free.s \
    $(SRC_DIR)/proxyr_mpage_free.s \
	


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

proxyr_initial_action_ARGS :=

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
