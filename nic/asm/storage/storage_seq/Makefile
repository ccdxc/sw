
TOPDIR = ../../..
SWDIR = $(TOPDIR)/../
GEN_DIR = $(TOPDIR)/gen
CUSTOM = $(GEN_DIR)/.storage_seq_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc


PROJ = storage_seq
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
SYM_DIR = $(TOPDIR)/sym/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/asm_out/
ALT_ASM_HDR_DIR = $(TOPDIR)/gen/$(PROJ)/alt_asm_out/
INC_DIRS = -I. -I.. -I../include -I$(ASM_HDR_DIR) -I$(ALT_ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include 
INC_DIRS += -I$(TOPDIR)/p4/include -I$(TOPDIR)/p4/storage/common -I$(TOPDIR)/include -I$(SWDIR)


SRC_FILES := \
	storage_seq_barco_chain_action.s \
	storage_seq_barco_entry_handler.s \
	storage_seq_barco_ring_pndx_pre_read0.s \
	storage_seq_barco_ring_pndx_read.s \
	storage_seq_comp_aol_pad_handler.s \
	storage_seq_comp_sgl_pad_only_xfer.s \
	storage_seq_comp_sgl_pdma_handler.s \
	storage_seq_comp_status_desc0_handler.s \
	storage_seq_comp_status_desc1_handler.s \
	storage_seq_comp_status_handler.s \
	storage_seq_q_state_pop.s \
	storage_seq_xts_comp_len_update.s \
	storage_seq_xts_sgl_pdma_handler.s \
	storage_seq_xts_status_desc0_handler.s \
	storage_seq_xts_status_desc1_handler.s \
	storage_seq_xts_status_handler.s \
	$(NULL)

BIN_FILES := $(patsubst %.s, %.bin, $(SRC_FILES))
SYM_FILES := $(patsubst %.s, %.sym, $(SRC_FILES))

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)
COMP_SYM_FILES = $(FILES:%=$(SYM_DIR)/%.sym)

$(GEN_DIR)/.storage_seq_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

$(OBJ_DIR)/%.bin: %.s $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) -t $(CAPAS_COV) $(INC_DIRS) $(AS_DEFINES) $< -o $@ > $(SYM_DIR)/$(patsubst %.bin,%.sym,$(shell basename $@))

clean:
	@rm -fR $(COMP_BIN_FILES)
	@rm -fR $(COMP_SYM_FILES)
	@rm -fR $(GEN_DIR)/.storage_seq_asm

.asm_test:
	@echo DEPS_DIRS = $(DEPS_DIRS)
	@echo DEPS = $(DEPS)
	@echo FILES = $(FILES)
	@echo BIN_FILES = $(BIN_FILES)
	@echo SYM_FILES = $(SYM_FILES)
	@echo COMP_BIN_FILES = $(COMP_BIN_FILES)
	@echo COMP_SYM_FILES = $(COMP_SYM_FILES)
