SRC_DIR = .
TOPDIR = ../../..
P4_DIR = $(TOPDIR)/p4/rdma
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
NCC_BIN = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --p4-plus --asm-out --no-ohi
INC_DIR = -I ../common/include -I ./include -I $(GEN_DIR)/rdma_rxdma/asm_out

src = $(wildcard *.s)
bin = $(src:.s=.bin)

LDFLAGS = 
CFLAGS = -g

all: deps asm

deps: $(GEN_DIR)/rdma_rxdma/asm_out/ingress.h

%.h: $(P4_DIR)/rdma_rxdma.p4
	$(NCC_BIN) $(NCC_OPTIONS) $(P4_DIR)/rdma_rxdma.p4 --gen-dir $(GEN_DIR)

asm: $(bin)

.PHONY: clean 
clean:
	@rm -fR $(OBJ_DIR)/*.bin

%.bin: %.s
	$(CAPAS_BIN) $(INC_DIR) $< -o $(OBJ_DIR)/$@

TEST_FILES := \
    $(SRC_DIR)/ctl/common_p4plus_stage0_lif_table1.ctl

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))


$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIR) $< $(OBJ_DIR)/$(basename $(notdir $@)).bin

gtest: $(TMP_FILES)
