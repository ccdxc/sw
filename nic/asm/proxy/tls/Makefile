
TOPDIR = ../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc


PROJ = tls_txdma_pre_crypto_enc
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
GEN_DIR = $(TOPDIR)/gen
ASM_HDR_DIR = $(GEN_DIR)/$(PROJ)/asm_out/
P4_DIR = $(TOPDIR)/p4/proxy
COMMON_P4_DIR = $(TOPDIR)/p4/common-p4+
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I./include -I../include -I../../common-p4+/include -I../../cpu-p4plus/include -I../tcp/include -I$(TOPDIR)/include
SRC_DIR = .
CAPAS_BIN = ../../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../../asic/capri/model/capsim-gen/bin/capsim
NCC_BIN = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --p4-plus --asm-out --no-ohi

all: deps asm

deps:

SRC_FILES := \
    $(SRC_DIR)/tls-stage0.s \
    $(SRC_DIR)/tls-pre-crypto-stage0.s \
	$(SRC_DIR)/tls-post-crypto-stage0.s


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
    $(SRC_DIR)/ctl/tls-alloc-brq-pi.ctl \
    $(SRC_DIR)/ctl/tls-alloc-rnmdr.ctl \
    $(SRC_DIR)/ctl/tls-alloc-rnmdr-split.ctl \
    $(SRC_DIR)/ctl/tls-alloc-rnmpr.ctl \
    $(SRC_DIR)/ctl/tls-queue-brq.ctl \
    $(SRC_DIR)/ctl/tls-read-desc.ctl \
    $(SRC_DIR)/ctl/tls-read-header.ctl \
    $(SRC_DIR)/ctl/tls-rx-serq.ctl \
    $(SRC_DIR)/ctl/tls-serq.ctl \
    $(SRC_DIR)/ctl/tls-update-page-ctl.ctl

TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))


$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

run: $(BIN_FILES)
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-serq.ctl $(OBJ_DIR)/tls-serq.bin > $(SRC_DIR)/run/tls-serq.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-read-desc.ctl $(OBJ_DIR)/tls-read-desc.bin > $(SRC_DIR)/run/tls-read-desc.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-rx-serq.ctl $(OBJ_DIR)/tls-rx-serq.bin > $(SRC_DIR)/run/tls-rx-serq.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-read-header.ctl $(OBJ_DIR)/tls-read-header.bin > $(SRC_DIR)/run/tls-read-header.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-alloc-rnmdr-split.ctl $(OBJ_DIR)/tls-alloc-rnmdr-split.bin > $(SRC_DIR)/run/tls-alloc-rnmdr-split.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-update-page-ctl.ctl $(OBJ_DIR)/tls-update-page-ctl.bin > $(SRC_DIR)/run/tls-update-page-ctl.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-alloc-rnmdr.ctl $(OBJ_DIR)/tls-alloc-rnmdr.bin > $(SRC_DIR)/run/tls-alloc-rnmdr.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-alloc-rnmpr.ctl $(OBJ_DIR)/tls-alloc-rnmpr.bin > $(SRC_DIR)/run/tls-alloc-rnmpr.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-alloc-brq-pi.ctl $(OBJ_DIR)/tls-alloc-brq-pi.bin > $(SRC_DIR)/run/tls-alloc-brq-pi.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tls-queue-brq.ctl $(OBJ_DIR)/tls-queue-brq.bin > $(SRC_DIR)/run/tls-queue-brq.run

clean:
	@rm -fR $(OBJ_DIR)/*.bin

