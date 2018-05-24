TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.tyche_asm
CUSTOM_CLEAN = asm_clean

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = tyche
OBJ_DIR = ../../obj/tyche/asm_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR)
SRC_DIR = .

SRC_FILES := \
	rx_child_policer.asm \
	rx_flow_hash.asm \
	rx_flow_hash_overflow.asm \
	rx_flow_result.asm \
	rx_flow_tcam1.asm \
	rx_flow_tcam2.asm \
	rx_key1.asm \
	rx_key2.asm \
	rx_key3.asm \
	rx_parent_policer.asm \
	rx_rewrite0.asm \
	rx_rewrite1.asm \
	rx_rewrite2.asm \
	tx_child_policer.asm \
	tx_flow_hash.asm \
	tx_flow_hash_overflow.asm \
	tx_flow_result.asm \
	tx_flow_tcam.asm \
	tx_key.asm \
	tx_parent_policer.asm \
	tx_rewrite0.asm \
	tx_rewrite1.asm \
	tx_rewrite2.asm \
	$(NULL)

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

asm: $(BIN_FILES)

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.tyche_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	@echo "CAPAS   " $<
	@$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@

asm_clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.tyche_asm
