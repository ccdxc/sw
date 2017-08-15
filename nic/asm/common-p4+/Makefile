
TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = common-p4+
OBJ_DIR = ../../obj/p4plus_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR)
SRC_DIR = .
CAPAS_BIN = ../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

SRC_FILES := \
    $(SRC_DIR)/rxdma_stage0.s \
    $(SRC_DIR)/txdma_stage0.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin

