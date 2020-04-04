#ifndef __ATHENA_TEST_HPP__
#define __ATHENA_TEST_HPP__

namespace fte_ath {

extern uint32_t g_session_index;
extern uint32_t g_session_rewrite_index;
sdk_ret_t
fte_vlan_to_vnic_map (uint16_t vlan_id, uint16_t vnic_id);

sdk_ret_t
fte_mpls_label_to_vnic_map (uint32_t mpls_label, uint16_t vnic_id);

sdk_ret_t
fte_h2s_v4_session_rewrite_mplsoudp (uint32_t session_rewrite_id,
                                     mac_addr_t *substrate_dmac,
                                     mac_addr_t *substrate_smac,
                                     uint16_t substrate_vlan,
                                     uint32_t substrate_sip,
                                     uint32_t substrate_dip,
                                     uint32_t mpls1_label,
                                     uint32_t mpls2_label);

sdk_ret_t
fte_h2s_nat_v4_session_rewrite_mplsoudp (uint32_t session_rewrite_id,
                                     mac_addr_t *substrate_dmac,
                                     mac_addr_t *substrate_smac,
                                     uint16_t substrate_vlan,
                                     uint32_t substrate_sip,
                                     uint32_t substrate_dip,
                                     uint32_t mpls1_label,
                                     uint32_t mpls2_label,
                                     pds_flow_session_rewrite_nat_type_t nat_type,
                                     ipv4_addr_t ipv4_addr);

sdk_ret_t
fte_s2h_nat_v4_session_rewrite (uint32_t session_rewrite_id,
                            mac_addr_t *ep_dmac, mac_addr_t *ep_smac,
                            uint16_t vnic_vlan,
                            pds_flow_session_rewrite_nat_type_t nat_type,
                            ipv4_addr_t ipv4_addr);

sdk_ret_t
fte_create_dnat_map_ipv4(uint16_t vnic_id, ipv4_addr_t v4_nat_dip, 
        ipv4_addr_t v4_orig_dip, uint16_t dnat_epoch);

sdk_ret_t
fte_s2h_v4_session_rewrite (uint32_t session_rewrite_id,
                            mac_addr_t *ep_dmac, mac_addr_t *ep_smac,
                            uint16_t vnic_vlan);

sdk_ret_t
fte_session_info_create_all(uint32_t session_id, uint32_t conntrack_id,
                uint8_t skip_flow_log, mac_addr_t *host_mac,
                uint16_t h2s_epoch_vnic, uint32_t h2s_epoch_vnic_id,
                uint16_t h2s_epoch_mapping, uint32_t h2s_epoch_mapping_id,
                uint16_t h2s_policer_bw1_id, uint16_t h2s_policer_bw2_id,
                uint16_t h2s_vnic_stats_id, uint8_t *h2s_vnic_stats_mask,
                uint16_t h2s_vnic_histogram_latency_id, uint16_t h2s_vnic_histogram_packet_len_id,
                uint8_t h2s_tcp_flags_bitmap,
                uint32_t h2s_session_rewrite_id,
                uint16_t h2s_allowed_flow_state_bitmask,
                pds_egress_action_t h2s_egress_action,

                uint16_t s2h_epoch_vnic, uint32_t s2h_epoch_vnic_id,
                uint16_t s2h_epoch_mapping, uint32_t s2h_epoch_mapping_id,
                uint16_t s2h_policer_bw1_id, uint16_t s2h_policer_bw2_id,
                uint16_t s2h_vnic_stats_id, uint8_t *s2h_vnic_stats_mask,
                uint16_t s2h_vnic_histogram_latency_id, uint16_t s2h_vnic_histogram_packet_len_id,
                uint8_t s2h_tcp_flags_bitmap,
                uint32_t s2h_session_rewrite_id,
                uint16_t s2h_allowed_flow_state_bitmask,
                pds_egress_action_t s2h_egress_action);

sdk_ret_t
fte_flow_create(uint16_t vnic_id, ipv4_addr_t v4_addr_sip, ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
fte_flow_create_icmp(uint16_t vnic_id,
        ipv4_addr_t v4_addr_sip, ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint8_t type, uint8_t code, uint16_t identifier,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
fte_flow_create_v6(uint16_t vnic_id, ipv6_addr_t *v6_addr_sip,
        ipv6_addr_t *v6_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
fte_flow_create_v6_icmp(uint16_t vnic_id, ipv6_addr_t *v6_addr_sip,
        ipv6_addr_t *v6_addr_dip,
        uint8_t proto, uint8_t type, uint8_t code, uint16_t identifier,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
fte_setup_static_flows (void);

sdk_ret_t
fte_setup_static_dnat_flows(void);

}

#endif /* __ATHENA_TEST_HPP__ */
