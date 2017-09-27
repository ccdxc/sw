
TOPDIR = ../../../..
CUSTOM = my_all
CUSTOM_CLEAN = my_clean

include $(TOPDIR)/make/Makefile.inc

PROJ = tcp_proxy_rxdma
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
P4_DIR = $(TOPDIR)/p4/proxy
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../include -I../../../common-p4+/include -I../../../cpu-p4plus/include -I$(TOPDIR)/include -I../../include
SRC_DIR = .
CAPAS_BIN = ../../../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../../../asic/capri/model/capsim-gen/bin/capsim
NCC_BIN = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --p4-plus --asm-out --no-ohi

my_all: deps asm

deps: $(ASM_HDR_DIR)/ingress.h


SRC_FILES := \
    $(SRC_DIR)/tcp-cc.s \
    $(SRC_DIR)/tcp-fastretx-alert.s \
    $(SRC_DIR)/tcp-rdesc-alloc.s \
    $(SRC_DIR)/tcp-read-idx.s \
    $(SRC_DIR)/tcp-read-rnmdr-alloc-idx.s \
    $(SRC_DIR)/tcp-read-rnmpr-alloc-idx.s \
    $(SRC_DIR)/tcp-read-tx2rx-shared.s \
    $(SRC_DIR)/tcp-rpage-alloc.s \
    $(SRC_DIR)/tcp-rtt.s \
    $(SRC_DIR)/tcp-rx.s \
    $(SRC_DIR)/tcp-sack2.s \
    $(SRC_DIR)/tcp-sack4.s \
    $(SRC_DIR)/tcp-fc.s \
    $(SRC_DIR)/tcp-rx-stats.s \
    $(SRC_DIR)/tcp-write-serq.s \
    $(SRC_DIR)/tcp-write-arq.s

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
    $(SRC_DIR)/ctl/tcp-rx.ctl \
    $(SRC_DIR)/ctl/tcp-write-serq.ctl \
    $(SRC_DIR)/ctl/tcp-cc.ctl \
    $(SRC_DIR)/ctl/tcp-rtt-first.ctl \
    $(SRC_DIR)/ctl/tcp-rtt-non-first.ctl \
    $(SRC_DIR)/ctl/tcp-fastretx-alert-cwr-end.ctl

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

tcp-rx_ARGS :=
tcp-write-serq_ARGS :=
tcp-cc_ARGS :=
tcp-rtt_ARGS :=
tcp-rtt-non-first_ARGS :=
tcp-fastretx-alert_cwr_end_ARGS :=

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

run: $(BIN_FILES)
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-rx.ctl $(OBJ_DIR)/tcp-rx.bin \
		-P tcp_rx_rtt_stage2_start=0x100000 \
		-P tcp_rx_read_rnmdr_stage2_start=0x200000 \
		-P tcp_rx_read_rnpdr_stage2_start=0x300000 \
		-P tcp_rx_read_serq_stage2_start=0x400000 \
		-P tcp_rx_sack_stage2_start=0x500000 \
		-P tcp_rx_read_rnmpr_stage2_start=0x600000 \
		> $(SRC_DIR)/run/tcp-rx.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-write-serq.ctl $(OBJ_DIR)/tcp-write-serq.bin > $(SRC_DIR)/run/tcp-write-serq.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-cc.ctl $(OBJ_DIR)/tcp-cc.bin \
	    	-P tcp_rx_fc_stage5_start=0x100000 > $(SRC_DIR)/run/tcp-cc.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-rtt-first.ctl $(OBJ_DIR)/tcp-rtt.bin \
	    	-P tcp_rx_fra_stage3_start=0x100000 > $(SRC_DIR)/run/tcp-rtt-first.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-rtt-non-first.ctl $(OBJ_DIR)/tcp-rtt.bin \
	  	-P tcp_rx_fra_stage3_start=0x100000 > $(SRC_DIR)/run/tcp-rtt-non-first.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-fastretx-alert-cwr-end.ctl $(OBJ_DIR)/tcp-fastretx-alert.bin \
	    	-P tcp_rx_cc_stage4_start=0x100000 > $(SRC_DIR)/run/tcp-fastretx-alert-cwr-end.run

my_clean:
	@rm -fR $(OBJ_DIR)/*.bin
