SRC_DIR = .
TOPDIR = ../../..
P4_DIR = $(TOPDIR)/p4/rdma
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
PROJ=req_rx
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
SYM_DIR = $(TOPDIR)/sym/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ALT_ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/alt_asm_out/

include $(TOPDIR)/make/Make.inc

CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
INC_DIR = -I ../common/include -I $(TOPDIR) -I ./include -I ../../common-p4+/include -I$(TOPDIR)/include -I $(GEN_DIR)/rdma_req_rxdma/asm_out  -I$(ALT_ASM_HDR_DIR)

src = $(wildcard *.s)
bin = $(src:.s=.bin)

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)
COMP_SYM_FILES = $(FILES:%=$(SYM_DIR)/%.sym)

LDFLAGS =
CFLAGS = -g

all: asm

asm: $(bin)

.PHONY: clean
clean:
	@rm -fR $(COMP_BIN_FILES)
	@rm -fR $(COMP_SYM_FILES)

%.bin: %.s
	$(CAPAS_BIN) -t $(CAPAS_COV) $(INC_DIR) $< -o $(OBJ_DIR)/$@ > $(SYM_DIR)/$(patsubst %.bin,%.sym,$(shell basename $(OBJ_DIR)/$@))

TEST_FILES := \
    $(SRC_DIR)/ctl/req_rx_sqcb_process.ctl

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

#this is to make capsim happy and avoid undefined symbols
req_rx_sqcb_process_PARAMS := -P req_rx_sqcb1_process=100

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename $(notdir $@))_ARGS} ${$(basename $(notdir $@))_PARAMS} $(INC_DIR) $< $(OBJ_DIR)/$(basename $(notdir $@)).bin

gtest: $(TMP_FILES)
