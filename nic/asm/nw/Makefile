
TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

PROJ = iris
OBJ_DIR = ../../obj/asm_bin
ASM_HDR_DIR = ../../gen/$(PROJ)/asm_out/
INC_DIRS = -I. -I$(ASM_HDR_DIR)
SRC_DIR = .
CAPAS_BIN = ../../asic/capri/model/capsim-gen/bin/capas
CAPSIM_BIN = ../../asic/capri/model/capsim-gen/bin/capsim

all: deps asm

deps:

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
    $(SRC_DIR)/registered_macs.asm \
    $(SRC_DIR)/registered_macs_otcam.asm \
    $(SRC_DIR)/replica.asm \
    $(SRC_DIR)/rewrite.asm \
    $(SRC_DIR)/tunnel_decap.asm \
    $(SRC_DIR)/tunnel_decap_copy_inner.asm \
    $(SRC_DIR)/tunnel_encap_update_inner.asm \
    $(SRC_DIR)/tunnel_rewrite.asm \
    $(SRC_DIR)/twice_nat.asm \
    $(SRC_DIR)/tx_stats.asm

BIN_FILES := $(patsubst %.asm, %.bin, $(SRC_FILES))

$(BIN_FILES): %.bin: %.asm
	@echo $@
	@mkdir -p $(OBJ_DIR)
	$(CAPAS_BIN) $(INC_DIRS) $< -o $(OBJ_DIR)/$(shell basename $@)

asm: $(BIN_FILES)

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
registered_macs_ARGS := -r registered_macs_hit
registered_macs_otcam_ARGS := -r registered_macs_otcam_hit
rewrite_ARGS := -r l3_rewrite
tunnel_decap_copy_inner_ARGS := -r copy_inner_ipv4_udp
tunnel_encap_update_inner_ARGS := -r encap_inner_ipv4_udp_rewrite
tunnel_rewrite_ARGS := -r encap_vxlan
twice_nat_ARGS := -r twice_nat_rewrite_info

$(TMP_FILES): %.x: %.ctl
	@echo Testing ...
	@echo $<
	$(CAPSIM_BIN) -e ${$(basename ${<})_ARGS} $(INC_DIRS) $< $(OBJ_DIR)/$(patsubst %.x,%.bin,$@)

test: $(TMP_FILES)

clean:
	@rm -fR $(OBJ_DIR)/*.bin

