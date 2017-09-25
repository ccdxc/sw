TOPDIR = ../..
CUSTOM = build_asm
CUSTOM_CLEAN = clean_asm

include $(TOPDIR)/make/Makefile.inc

PROJ = ipfix
OBJ_DIR = ../../obj/p4plus_bin
ASM_HDR_DIR = ../../gen/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I$(TOPDIR)/include
SRC_DIR = .
CAPAS_BIN = ../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

build_asm: deps asm

deps:

SRC_FILES := \
    $(SRC_DIR)/ipfix.s \
    $(SRC_DIR)/ipfix_flow_hash.s \
    $(SRC_DIR)/ipfix_flow_info.s \
    $(SRC_DIR)/ipfix_session_state.s \
    $(SRC_DIR)/ipfix_flow_stats.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo "CAPAS $@"
	@mkdir -p $(OBJ_DIR)
	@$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

TEST_FILES := $(patsubst %.s, %.ctl, $(SRC_FILES))
TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

gtest:

clean_asm:
	@rm -f $(OBJ_DIR)/ipfix*.bin
