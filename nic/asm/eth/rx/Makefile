
TOPDIR = ../../../
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = eth_rxdma_actions
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I.. -I$(ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include/ -I$(TOPDIR)/include
SRC_DIR = .
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

SRC_FILES := \
	eth_rx_fetch_desc.s \
	eth_rx_packet.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
