SRC_DIR = .
TOPDIR = ../../..
P4_DIR = $(TOPDIR)/p4/rdma
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen

include $(TOPDIR)/make/Make.inc

CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
INC_DIR = -I ../common/include -I ./include -I $(GEN_DIR) -I $(TOPDIR)/include -I ../req_rx/include -I ../resp_rx/include

src = $(wildcard *.s)
bin = $(src:.s=.bin)

LDFLAGS =
CFLAGS = -g

all: deps asm

deps: $(GEN_DIR)/rdma_rxdma/asm_out/ingress.h

%.h: $(P4_DIR)/rdma_rxdma.p4
	$(MAKE) -C $(P4_DIR) ../../gen/rdma_rxdma/asm_out/ingress.h

asm: $(bin)

.PHONY: clean
clean:
	@rm -fR $(OBJ_DIR)/*.bin

%.bin: %.s
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIR) $< -o $(OBJ_DIR)/$@

TEST_FILES := \
    $(SRC_DIR)/ctl/rx_stage0_load_rdma_params.ctl \
    $(SRC_DIR)/ctl/tx_stage0_load_rdma_params.ctl

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIR) $< $(OBJ_DIR)/$(basename $(notdir $@)).bin

gtest: $(TMP_FILES)
