SRC_DIR = .
TOPDIR = ../../..
P4_DIR = $(TOPDIR)/p4/smbdc
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen

include $(TOPDIR)/make/Make.inc

CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
INC_DIR = -I ../common/include -I ../../../p4/iris/include -I ./include -I $(GEN_DIR)/smbdc_req_txdma/asm_out -I $(TOPDIR)/include -I$(TOPDIR)/p4/include

src = $(wildcard *.s)
bin = $(src:.s=.bin)

LDFLAGS =
CFLAGS = -g

all: asm

asm: $(bin)

.PHONY: clean
clean:
	@rm -fR $(OBJ_DIR)/*.bin

%.bin: %.s
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIR) $(AS_DEFINES) $< -o $(OBJ_DIR)/$@

TEST_FILES := \
    $(SRC_DIR)/ctl/smbdc_req_tx_mr_select_process.ctl \
    $(SRC_DIR)/ctl/smbdc_req_tx_wqe_process.ctl \
    $(SRC_DIR)/ctl/smbdc_req_tx_cqcb_process.ctl

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

smbdc_req_tx_mr_select_process_PARAMS := -P smbdc_req_tx_post_rdma_req_process=200
smbdc_req_tx_wqe_process_PARAMS := -P smbdc_req_tx_mr_select_process=300


$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename $(notdir $@))_ARGS} ${$(basename $(notdir $@))_PARAMS} $(INC_DIR) $< $(OBJ_DIR)/$(basename $(notdir $@)).bin

gtest: $(TMP_FILES)
