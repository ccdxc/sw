TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.apollo_asm
CUSTOM_CLEAN = asm_clean

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = apollo
OBJ_DIR = ../../obj/apollo/asm_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR)
SRC_DIR = .

SRC_FILES := \
	egress_vnic_stats.asm \
	encap.asm \
	ep_mapping.asm \
	ep_mapping_ohash.asm \
	ingress_vnic_stats.asm \
	init_config.asm \
	key_native.asm \
	key_tunneled.asm \
	lpm_s0.asm \
	lpm_s1.asm \
	lpm_s2.asm \
	mirror.asm \
	policy.asm \
	policy_ohash.asm \
	policy_stats.asm \
	rewrite.asm \
	slacl_class0.asm \
	slacl_class1.asm \
	slacl_class2.asm \
	slacl_dport.asm \
	slacl_ip_15_00.asm \
	slacl_ip_31_16.asm \
	slacl_proto.asm \
	slacl_sport.asm \
	slacl_stats.asm \
	source_guard.asm \
	source_guard_ohash.asm \
	tunnel_decap.asm \
	tunnel_rewrite.asm \
	vnic.asm \
	$(NULL)

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

$(GEN_DIR)/.apollo_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	@echo "CAPAS   " $<
	@$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@

asm_clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.apollo_asm
