
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
    $(SRC_DIR)/tcp-fast-timer.s \
    $(SRC_DIR)/tcp-queue-sched-tx.s \
    $(SRC_DIR)/tcp-read-rx2tx-shared-extra.s \
    $(SRC_DIR)/tcp-read-rx2tx-shared.s \
    $(SRC_DIR)/tcp-read-sesq-ci.s \
    $(SRC_DIR)/tcp-read-sesq.s \
    $(SRC_DIR)/tcp-read-xmit-cursor.s \
    $(SRC_DIR)/tcp-sesq-consume.s \
    $(SRC_DIR)/tcp-slow-timer.s \
    $(SRC_DIR)/tcp-tdesc-alloc.s \
    $(SRC_DIR)/tcp-tso.s \
    $(SRC_DIR)/tcp-tx.s

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.s
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)


#TEST_FILES := $(patsubst %.s, %.ctl, $(SRC_FILES))
TEST_FILES := \
    $(SRC_DIR)/ctl/tcp-fast-timer.ctl \
    $(SRC_DIR)/ctl/tcp-slow-timer-keepalive.ctl \
    $(SRC_DIR)/ctl/tcp-slow-timer-pmtu.ctl \
    $(SRC_DIR)/ctl/tcp-tso.ctl \
    $(SRC_DIR)/ctl/tcp-tx.ctl


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
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-fast-timer.ctl $(OBJ_DIR)/tcp-fast-timer.bin > $(SRC_DIR)/run/tcp-fast-timer.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-slow-timer-keepalive.ctl $(OBJ_DIR)/tcp-slow-timer.bin > $(SRC_DIR)/run/tcp-slow-timer-keepalive.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-slow-timer-pmtu.ctl $(OBJ_DIR)/tcp-slow-timer.bin > $(SRC_DIR)/run/tcp-slow-timer-pmtu.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-tso.ctl $(OBJ_DIR)/tcp-tso.bin > $(SRC_DIR)/run/tcp-tso.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/tcp-tx.ctl $(OBJ_DIR)/tcp-tx.bin > $(SRC_DIR)/run/tcp-tx.run

clean:
	@rm -fR $(OBJ_DIR)/*.bin

