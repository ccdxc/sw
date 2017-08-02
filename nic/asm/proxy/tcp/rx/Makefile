
TOPDIR = ../../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = proxy
OBJ_DIR = $(TOPDIR)/obj/asm_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR) -I../include -I../../../include
SRC_DIR = .
CAPAS_BIN = $(TOPDIR)/capsim/gen/bin/capas
CAPSIM_BIN = $(TOPDIR)/capsim/gen/bin/capsim

all: deps asm

deps:


SRC_FILES := \
    $(SRC_DIR)/tcp-aol-page-alloc.s \
    $(SRC_DIR)/tcp-cc.s \
    $(SRC_DIR)/tcp-fastretx-alert.s \
    $(SRC_DIR)/tcp-get-obj.s \
    $(SRC_DIR)/tcp-get-tkt.s \
    $(SRC_DIR)/tcp-rdesc-alloc.s \
    $(SRC_DIR)/tcp-read-aol-entry.s \
    $(SRC_DIR)/tcp-read-idx.s \
    $(SRC_DIR)/tcp-read-rnmdr-alloc-idx.s \
    $(SRC_DIR)/tcp-read-rnmpr-alloc-idx.s \
    $(SRC_DIR)/tcp-read-se-rcv-aol-entry.s \
    $(SRC_DIR)/tcp-read-serq-pidx.s \
    $(SRC_DIR)/tcp-read-tx2rx-shared.s \
    $(SRC_DIR)/tcp-rpage-alloc.s \
    $(SRC_DIR)/tcp-rtt.s \
    $(SRC_DIR)/tcp-rx.s \
    $(SRC_DIR)/tcp-sack1.s \
    $(SRC_DIR)/tcp-sack2.s \
    $(SRC_DIR)/tcp-sack3.s \
    $(SRC_DIR)/tcp-sack4.s \
    $(SRC_DIR)/tcp-write-serq.s

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

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
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-rx.ctl $(OBJ_DIR)/tcp-rx.bin > $(SRC_DIR)/run/tcp-rx.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-write-serq.ctl $(OBJ_DIR)/tcp-write-serq.bin > $(SRC_DIR)/run/tcp-write-serq.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-cc.ctl $(OBJ_DIR)/tcp-cc.bin > $(SRC_DIR)/run/tcp-cc.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-rtt-first.ctl $(OBJ_DIR)/tcp-rtt.bin > $(SRC_DIR)/run/tcp-rtt-first.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-rtt-non-first.ctl $(OBJ_DIR)/tcp-rtt.bin > $(SRC_DIR)/run/tcp-rtt-non-first.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-fastretx-alert-cwr-end.ctl $(OBJ_DIR)/tcp-fastretx-alert.bin > $(SRC_DIR)/run/tcp-fastretx-alert-cwr-end.run

clean:
	@rm -fR $(OBJ_DIR)/*.bin

