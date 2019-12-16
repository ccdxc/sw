TOPDIR = ../../..
SWDIR = $(TOPDIR)/../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.apollo_rxdma_asm
CUSTOM_CLEAN = apollo_rxdma_clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = apollo
OBJ_DIR = $(TOPDIR)/obj/$(PROJ)_rxdma/asm_bin/
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)_rxdma/asm_out/
ALT_ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)_rxdma/alt_asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I$(ALT_ASM_HDR_DIR) -I$(SWDIR) -I$(TOPDIR)/asm/common-p4+/include -I$(TOPDIR)/include
SRC_DIR = .

SRC_FILES := $(wildcard $(SRC_DIR)/*.asm)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.apollo_rxdma_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	@echo "CAPAS   " $<
	@$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $(AS_DEFINES) $< -o $@

apollo_rxdma_clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.apollo_rxdma_asm

