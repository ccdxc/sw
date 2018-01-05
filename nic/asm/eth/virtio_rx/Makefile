
TOPDIR = ../../../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.virtio_rx_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = virtio_rxdma
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I.. -I$(TOPDIR) -I$(ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include/ -I$(TOPDIR)/include
SRC_DIR = .


CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
	virtio_rx_read_qstate.s \
	virtio_rx_read_virtq_avail.s \
	virtio_rx_check_reqs.s \
	virtio_rx_read_head_desc_idx.s \
	virtio_rx_read_head_desc.s \
	virtio_rx_read_frag_desc.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

local_deps = $(shell find . -name '*.asm' -o -name '*.s')
FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.virtio_rx_asm: $(OBJ_DIR)/.dir $(local_deps) $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')
$(OBJ_DIR)/%.bin: %.s $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@


#BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))
#
#$(BIN_FILES): %.bin: %.s
#	@echo $@
#	@mkdir -p $(OBJ_DIR)
#	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)
#
#asm: $(BIN_FILES)

runsim: $(COMP_BIN_FILES)
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_read_qstate.ctl $(OBJ_DIR)/virtio_rx_read_qstate.bin > $(SRC_DIR)/run/virtio_rx_read_qstate.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_read_virtq_avail.ctl $(OBJ_DIR)/virtio_rx_read_virtq_avail.bin > $(SRC_DIR)/run/virtio_rx_read_virtq_avail.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_check_reqs.ctl $(OBJ_DIR)/virtio_rx_check_reqs.bin > $(SRC_DIR)/run/virtio_rx_check_reqs.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_read_head_desc_idx.ctl $(OBJ_DIR)/virtio_rx_read_head_desc_idx.bin > $(SRC_DIR)/run/virtio_rx_read_head_desc_idx.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_read_head_desc.ctl $(OBJ_DIR)/virtio_rx_read_head_desc.bin > $(SRC_DIR)/run/virtio_rx_read_head_desc.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_read_head_desc-1.ctl $(OBJ_DIR)/virtio_rx_read_head_desc.bin > $(SRC_DIR)/run/virtio_rx_read_head_desc-1.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_rx_read_frag_desc.ctl $(OBJ_DIR)/virtio_rx_read_frag_desc.bin > $(SRC_DIR)/run/virtio_rx_read_frag_desc.run

clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.virtio_rx_asm
