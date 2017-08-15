
TOPDIR = ../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = proxy
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../include -I../../include -I../tcp/include -I./include -I../..//common-p4+/include
SRC_DIR = .
CAPAS_BIN = ../../../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../../../asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

SRC_FILES := \
    $(SRC_DIR)/tls-alloc-brq-pi.s \
    $(SRC_DIR)/tls-alloc-rnmdr.s \
    $(SRC_DIR)/tls-alloc-rnmdr-split.s \
    $(SRC_DIR)/tls-alloc-rnmpr.s \
    $(SRC_DIR)/tls-alloc-sesq-pi.s \
    $(SRC_DIR)/tls-brq.s \
    $(SRC_DIR)/tls-queue-brq.s \
    $(SRC_DIR)/tls-queue-sesq.s \
    $(SRC_DIR)/tls-read-desc.s \
    $(SRC_DIR)/tls-read-header.s \
    $(SRC_DIR)/tls-rx-brq.s \
    $(SRC_DIR)/tls-rx-serq.s \
    $(SRC_DIR)/tls-s0.s \
    $(SRC_DIR)/tls-serq-consume.s \
    $(SRC_DIR)/tls-serq.s \
    $(SRC_DIR)/tls-update-page-ctl.s


BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

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

