TOPDIR = ../../..
SWDIR = $(TOPDIR)/../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.apollo_txdma_asm
CUSTOM_CLEAN = apollo_txdma_clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = apollo_txdma
OBJ_DIR = $(TOPDIR)/obj/$(PROJ)/asm_bin/
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I$(SWDIR) -I$(TOPDIR)/asm/common-p4+/include -I$(TOPDIR)/include
SRC_DIR = .

CAPSIM_BIN = ../../../asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := $(wildcard $(SRC_DIR)/*.asm)

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.apollo_txdma_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')
$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	@echo "CAPAS   " $<
	@$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $(AS_DEFINES) $< -o $@

apollo_txdma_clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.apollo_txdma_asm

TEST_FILES := $(wildcard $(SRC_DIR)/*.ctl)
TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

gtest: $(TMP_FILES)
