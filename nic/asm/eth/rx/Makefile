
TOPDIR = ../../../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.eth_rx_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = eth_rxdma_actions
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I.. -I$(ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include/ -I$(TOPDIR)/include
SRC_DIR = .


CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
	eth_rx_fetch_desc.s \
	eth_rx_packet.s \
	$(NULL)


BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

local_deps = $(shell find . -name '*.asm' -o -name '*.s')
FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.eth_rx_asm: $(OBJ_DIR)/.dir $(local_deps) $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')
$(OBJ_DIR)/%.bin: %.s $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@


#BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))
#
#$(BIN_FILES): %.bin: %.s
#	@echo $@
#	@mkdir -p $(OBJ_DIR)
#	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)
#
#asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.eth_rx_asm
