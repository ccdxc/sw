SRC_DIR = .
TOPDIR = ../../..
P4_DIR = $(TOPDIR)/p4/rdma
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen

include $(TOPDIR)/make/Make.inc

CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
NCC_BIN = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --p4-plus --asm-out --no-ohi
INC_DIR = -I ../common/include -I ../../../p4/nw/include -I ./include -I ../../common-p4+/include -I$(TOPDIR)/include -I $(GEN_DIR)/rdma_rxdma/asm_out

src = $(wildcard *.s)
bin = $(src:.s=.bin)

LDFLAGS = 
CFLAGS = -g

#deps
all: asm

#deps: $(GEN_DIR)/rdma_trdma/asm_out/ingress.h

#$(P4_DIR)/rdma_rxdma.p4
%.h: $(COMMON_P4_DIR)/common_headers.p4 $(COMMON_P4_DIR)/common_rxdma_actions.p4
	$(NCC_BIN) $(NCC_OPTIONS) $(P4_DIR)/rdma_rxdma.p4 --gen-dir $(GEN_DIR)
	

asm: $(bin)

.PHONY: clean 
clean:
	@rm -fR $(OBJ_DIR)/*.bin

%.bin: %.s
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIR) $< -o $(OBJ_DIR)/$@

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
