
TOPDIR = ../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = cpu_rxdma
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
P4_DIR = $(TOPDIR)/p4/cpu-p4+
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../include -I../../common-p4+/include -I../../../include
SRC_DIR = .
CAPAS_BIN = ../../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../../asic/capri/model/capsim-gen/bin/capsim
NCC_BIN = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --p4-plus --asm-out --no-ohi

all: deps asm

deps: 

SRC_FILES := \
    $(SRC_DIR)/cpu_initial_action.s \
    $(SRC_DIR)/cpu_read_desc_pindex.s \
    $(SRC_DIR)/cpu_read_page_pindex.s \
    $(SRC_DIR)/cpu_read_arqrx_pindex.s \
    $(SRC_DIR)/cpu_desc_alloc.s \
    $(SRC_DIR)/cpu_page_alloc.s \
    $(SRC_DIR)/cpu_write_arq.s \
	


BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

%.h: $(P4_DIR)/$(PROJ).p4 $(COMMON_P4_DIR)/common_headers.p4
	$(NCC_BIN) $(NCC_OPTIONS) $(P4_DIR)/$(PROJ).p4 --gen-dir $(GEN_DIR)
	

asm: $(BIN_FILES)

#TEST_FILES := $(patsubst %.s, %.ctl, $(SRC_FILES))
TEST_FILES := \

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

cpu_initial_action_ARGS :=

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
