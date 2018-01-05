
TOPDIR = ../../..
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.virtio_tx_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = virtio_txdma
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I.. -I$(ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include -I$(TOPDIR)/p4/include -I$(TOPDIR)/include
SRC_DIR = .


CAPSIM_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
	virtio_tx_read_qstate.s \
	virtio_tx_read_virtq_avail.s \
	virtio_tx_check_reqs.s \
	virtio_tx_read_head_desc_idx.s \
	virtio_tx_read_head_desc.s \
	virtio_tx_read_frag_desc.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.virtio_tx_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
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
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_read_qstate.ctl $(OBJ_DIR)/virtio_tx_read_qstate.bin > $(SRC_DIR)/run/virtio_tx_read_qstate.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_read_virtq_avail.ctl $(OBJ_DIR)/virtio_tx_read_virtq_avail.bin > $(SRC_DIR)/run/virtio_tx_read_virtq_avail.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_check_reqs.ctl $(OBJ_DIR)/virtio_tx_check_reqs.bin > $(SRC_DIR)/run/virtio_tx_check_reqs.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_check_reqs-1.ctl $(OBJ_DIR)/virtio_tx_check_reqs.bin > $(SRC_DIR)/run/virtio_tx_check_reqs-1.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_read_head_desc_idx.ctl $(OBJ_DIR)/virtio_tx_read_head_desc_idx.bin > $(SRC_DIR)/run/virtio_tx_read_head_desc_idx.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_read_head_desc.ctl $(OBJ_DIR)/virtio_tx_read_head_desc.bin > $(SRC_DIR)/run/virtio_tx_read_head_desc.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_read_head_desc-1.ctl $(OBJ_DIR)/virtio_tx_read_head_desc.bin > $(SRC_DIR)/run/virtio_tx_read_head_desc-1.run
	$(CAPSIM_BIN) -c $(INC_DIRS) $(SRC_DIR)/ctl/virtio_tx_read_frag_desc.ctl $(OBJ_DIR)/virtio_tx_read_frag_desc.bin > $(SRC_DIR)/run/virtio_tx_read_frag_desc.run

clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.virtio_tx_asm
