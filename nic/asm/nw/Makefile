TOPDIR = ../..
GEN_DIR = ../../gen
CUSTOM = $(GEN_DIR)/.nw_asm
CUSTOM_CLEAN = clean


include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

PROJ = iris
OBJ_DIR = ../../obj/asm_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR)
SRC_DIR = .


CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

SRC_FILES := \
    $(SRC_DIR)/copp_action.asm \
    $(SRC_DIR)/copp.asm \
    $(SRC_DIR)/ddos_service.asm \
    $(SRC_DIR)/ddos_service_policer.asm \
    $(SRC_DIR)/ddos_service_policer_action.asm \
    $(SRC_DIR)/ddos_src_dst.asm \
    $(SRC_DIR)/ddos_src_dst_policer.asm \
    $(SRC_DIR)/ddos_src_dst_policer_action.asm \
    $(SRC_DIR)/ddos_src_vf.asm \
    $(SRC_DIR)/ddos_src_vf_policer.asm \
    $(SRC_DIR)/ddos_src_vf_policer_action.asm \
    $(SRC_DIR)/decode_roce_opcode.asm \
    $(SRC_DIR)/drop_stats.asm \
    $(SRC_DIR)/egress_policer_action.asm \
    $(SRC_DIR)/egress_policer.asm \
    $(SRC_DIR)/flow_hash.asm \
    $(SRC_DIR)/flow_hash_overflow.asm \
    $(SRC_DIR)/flow_info.asm \
    $(SRC_DIR)/session_state.asm \
    $(SRC_DIR)/flow_stats.asm \
    $(SRC_DIR)/ingress_policer_action.asm \
    $(SRC_DIR)/ingress_policer.asm \
    $(SRC_DIR)/ingress_tx_stats.asm \
    $(SRC_DIR)/input_mapping_native.asm \
    $(SRC_DIR)/input_mapping_tunneled.asm \
    $(SRC_DIR)/input_properties.asm \
    $(SRC_DIR)/input_properties_mac_vlan.asm \
    $(SRC_DIR)/ipsg.asm \
    $(SRC_DIR)/l4_profile.asm \
    $(SRC_DIR)/mirror.asm \
    $(SRC_DIR)/nacl.asm \
    $(SRC_DIR)/output_mapping.asm \
    $(SRC_DIR)/p4plus_app.asm \
    $(SRC_DIR)/replica.asm \
    $(SRC_DIR)/rewrite.asm \
    $(SRC_DIR)/tunnel_decap.asm \
    $(SRC_DIR)/tunnel_decap_copy_inner.asm \
    $(SRC_DIR)/tunnel_encap_update_inner.asm \
    $(SRC_DIR)/tunnel_rewrite.asm \
    $(SRC_DIR)/twice_nat.asm \
    $(SRC_DIR)/tx_stats.asm

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

asm: $(BIN_FILES)

#local_deps = $(shell find . -name '*.asm' -o -name '*.s')
DEPS_DIRS = $(subst -I,, $(INC_DIRS))
DEPS = $(shell find $(DEPS_DIRS) -name '*.h')

#asm_all: $(GEN_DIR)/.nw_asm asm
#asm_all: $(GEN_DIR)/.nw_asm $(BIN_FILES)
FILES = $(basename $(notdir $(BIN_FILES)))
COMP_BIN_FILES = $(FILES:%=$(OBJ_DIR)/%.bin)

#$(GEN_DIR)/.nw_asm: $(OBJ_DIR)/.dir $(local_deps) $(COMP_BIN_FILES)
$(GEN_DIR)/.nw_asm: $(OBJ_DIR)/.dir $(COMP_BIN_FILES)
	touch $@

$(OBJ_DIR)/%.bin: %.asm $(DEPS) $(CAPAS_BIN)
	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $@
	@#$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)



#$(BIN_FILES): %.bin: %.asm
#	@echo $@
#	@mkdir -p $(OBJ_DIR)
#	$(CAPAS_BIN) $(CAPAS_COV) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)




TEST_FILES := $(patsubst %.asm, %.ctl, $(SRC_FILES))
TMP_FILES  := $(patsubst %.ctl, %.x, $(TEST_FILES))

ddos_service_ARGS := -r ddos_service_hit
ddos_service_policer_action_ARGS := -r ddos_service_policer_action
ddos_src_dst_ARGS := -r ddos_src_dst_hit
ddos_src_dst_policer_action_ARGS := -r ddos_src_dst_policer_action
ddos_src_vf_ARGS := -r ddos_src_vf_hit
ddos_src_vf_policer_action_ARGS := -r ddos_src_vf_policer_action
decode_roce_opcode_ARGS := -r decode_roce_opcode
drop_stats_ARGS := -r drop_stats
flow_info_ARGS := -r flow_info
session_state_ARGS := -r tcp_session_state_info
input_mapping_native_ARGS := -r native_ipv4_packet
input_mapping_tunneled_ARGS := -r tunneled_ipv4_packet
input_properties_ARGS := -r input_properties
input_properties_mac_vlan_ARGS := -r input_properties_mac_vlan
ipsg_ARGS := -r ipsg_hit
mirror_ARGS := -r erspan_mirror
output_mapping_ARGS := -r set_tm_oport
p4plus_app_ARGS := -r p4plus_app_cpu
rewrite_ARGS := -r rewrite
tunnel_decap_copy_inner_ARGS := -r copy_inner_eth_ipv6_udp
tunnel_encap_update_inner_ARGS := -r encap_inner_ipv6_udp_rewrite
tunnel_rewrite_ARGS := -r encap_vxlan
twice_nat_ARGS := -r twice_nat_rewrite_info

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

gtest: $(TMP_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin
	@rm -fR $(GEN_DIR)/.nw_asm

.asm_test:
	@echo DEPS_DIRS = $(DEPS_DIRS)
	@echo DEPS = $(DEPS)
	@echo FILES = $(FILES)
	@echo BIN_FILES = $(BIN_FILES)
	@echo COMP_BIN_FILES = $(COMP_BIN_FILES)
