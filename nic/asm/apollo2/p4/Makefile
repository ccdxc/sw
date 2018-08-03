TOPDIR = ../../..
SWDIR = $(TOPDIR)/../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.apollo_asm
CUSTOM_CLEAN = asm_clean

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = apollo2
OBJ_DIR = $(TOPDIR)/obj/$(PROJ)/asm_bin/
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I$(SWDIR)
SRC_DIR = .

SRC_FILES := \
	local_vnic_by_vlan_tx.asm \
	$(NULL)

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.apollo_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	@echo "CAPAS   " $<
	@$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@

asm_clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.apollo_asm
