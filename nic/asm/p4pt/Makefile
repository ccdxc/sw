TOPDIR = ../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.p4pt_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = p4pt
OBJ_DIR = ../../obj/p4plus_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I$(TOPDIR)/include -I../common-p4+/include
SRC_DIR = .

CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
    $(SRC_DIR)/p4pt.s \
    $(SRC_DIR)/p4pt_update_tcb.s \
    $(SRC_DIR)/p4pt_update_tcb_rec.s \
    $(SRC_DIR)/p4pt_update_tcb_stats.s \
    $(SRC_DIR)/p4pt_update_read_latency_distribution.s \
    $(SRC_DIR)/p4pt_update_write_latency_distribution.s \
    $(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.p4pt_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')
$(OBJ_DIR)/%.bin: %.s $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@

TEST_FILES := $(patsubst %.s, %.ctl, $(SRC_FILES))
TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

gtest:

clean:
	@rm -f $(OBJ_DIR)/p4pt*.bin
	@rm -fR $(GEN_DIR)/.p4pt_asm
