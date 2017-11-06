TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.gft_asm
CUSTOM_CLEAN = asm_clean

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = gft
OBJ_DIR = ../../obj/gft/asm_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR)
SRC_DIR = .

CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
	gft_hash.asm \
	gft_hash_overflow.asm \
	hdr_transpositions0.asm \
	hdr_transpositions1.asm \
	hdr_transpositions2.asm \
	hdr_transpositions3.asm \
	ingress_key1.asm \
	ingress_key2.asm \
	ingress_key3.asm \
	ingress_key4.asm \
	ingress_vport.asm \
	ingress_vport_stats.asm \
	ipolicer.asm \
	$(NULL)

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

asm: $(BIN_FILES)

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.gft_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@

asm_clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.gft_asm
