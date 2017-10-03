TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.adminq_asm
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = adminq_txdma_actions
OBJ_DIR = ../../obj/p4plus_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I.. -I$(ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include -I$(TOPDIR)/include
SRC_DIR = .
CAPAS_BIN = ../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
    $(SRC_DIR)/adminq_fetch_desc.s \
    $(SRC_DIR)/adminq_operation.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.adminq_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

$(OBJ_DIR)/%.bin: %.s $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $@

clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.adminq_asm

.asm_test:
	@echo DEPS_DIRS = $(DEPS_DIRS)
	@echo DEPS = $(DEPS)
	@echo FILES = $(FILES)
	@echo BIN_FILES = $(BIN_FILES)
	@echo COMP_BIN_FILES = $(COMP_BIN_FILES)
