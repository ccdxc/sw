TOPDIR = ../../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = proxyc_txdma
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

all: asm

SRC_FILES := \
    $(SRC_DIR)/proxyc_tx_start.s \
    $(SRC_DIR)/proxyc_my_txq_entry_consume.s \
    $(SRC_DIR)/proxyc_desc_meta_strip.s \
    $(SRC_DIR)/proxyc_desc_enqueue.s \
    $(SRC_DIR)/proxyc_cpu_flags_post_read.s \
    $(SRC_DIR)/proxyc_cleanup_discard.s \
    $(SRC_DIR)/proxyc_desc_free.s \
    $(SRC_DIR)/proxyc_page0_free.s \
    $(SRC_DIR)/proxyc_page1_free.s \
    $(SRC_DIR)/proxyc_page2_free.s \
    $(SRC_DIR)/proxyc_stats.s \

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

TEST_FILES := \

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

proxyc_initial_action_ARGS :=

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
