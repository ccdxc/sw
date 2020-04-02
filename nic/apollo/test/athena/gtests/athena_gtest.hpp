#ifndef __ATHENA_GTEST_HPP__
#define __ATHENA_GTEST_HPP__

#define VNIC_ID_UDP         0x0001
#define VNIC_ID_TCP         0x0002
#define VNIC_ID_SLOW_PATH   0x0003
#define VNIC_ID_ICMP        0x0004
#define VNIC_ID_NAT         0x0005

#define VLAN_ID_UDP         0x0001
#define VLAN_ID_TCP         0x0002
#define VLAN_ID_SLOW_PATH   0x0003
#define VLAN_ID_ICMP        0x0004
#define VLAN_ID_NAT         0x0005

#define MPLS_LABEL_UDP      0x6789a
#define MPLS_LABEL_TCP      0x6789b
#define MPLS_LABEL_SLOW_PATH    0x6789c
#define MPLS_LABEL_ICMP     0x6789d
#define MPLS_LABEL_NAT      0x6789e

extern uint8_t     g_h_port;
extern uint8_t     g_s_port;
extern uint32_t    g_session_index;
extern uint32_t    g_session_rewrite_index;
extern uint32_t    g_epoch_index;

void
dump_pkt(std::vector<uint8_t> &pkt);

sdk_ret_t
send_packet(const char *out_pkt_descr, uint8_t *out_pkt, uint16_t out_pkt_len, uint32_t out_port,
        uint8_t *in_pkt, uint16_t in_pkt_len, uint32_t in_port);

sdk_ret_t
create_flow_v4_tcp_udp (uint16_t vnic_id, ipv4_addr_t v4_addr_sip, ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
create_flow_v6_tcp_udp (uint16_t vnic_id, ipv6_addr_t *v6_addr_sip,
        ipv6_addr_t *v6_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index);


sdk_ret_t
create_s2h_session_rewrite(uint32_t session_rewrite_id,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan);

sdk_ret_t
create_s2h_session_rewrite_nat_ipv4(uint32_t session_rewrite_id,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv4_addr_t ipv4_addr);

sdk_ret_t
create_s2h_session_rewrite_nat_ipv6(uint32_t session_rewrite_id,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv6_addr_t *ipv6_addr
        );

sdk_ret_t
create_h2s_session_rewrite_mplsoudp(uint32_t session_rewrite_id,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan,
        uint32_t substrate_sip, uint32_t substrate_dip,
        uint32_t mpls1_label, uint32_t mpls2_label);

sdk_ret_t
create_h2s_session_rewrite_mplsoudp_nat_ipv4(uint32_t session_rewrite_id,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan,
        uint32_t substrate_sip, uint32_t substrate_dip,
        uint32_t mpls1_label, uint32_t mpls2_label,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv4_addr_t ipv4_addr);

sdk_ret_t
create_h2s_session_rewrite_mplsoudp_nat_ipv6(uint32_t session_rewrite_id,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan,
        uint32_t substrate_sip, uint32_t substrate_dip,
        uint32_t mpls1_label, uint32_t mpls2_label,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv6_addr_t *ipv6_addr);

sdk_ret_t
vlan_to_vnic_map(uint16_t vlan_id, uint16_t vnic_id);

sdk_ret_t
mpls_label_to_vnic_map(uint32_t mpls_label, uint16_t vnic_id);

sdk_ret_t
create_session_info_all(uint32_t session_id, uint32_t conntrack_id,
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
create_flow_v4_icmp(uint16_t vnic_id, ipv4_addr_t v4_addr_sip,
        ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint8_t type, uint8_t code, uint16_t identifier,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
create_flow_v6_icmp(uint16_t vnic_id, ipv6_addr_t *v6_addr_sip,
        ipv6_addr_t *v6_addr_dip,
        uint8_t proto, uint8_t type, uint8_t code, uint16_t identifier,
        pds_flow_spec_index_type_t index_type, uint32_t index);

sdk_ret_t
create_dnat_map_ipv4(uint16_t vnic_id, ipv4_addr_t v4_nat_dip, 
        ipv4_addr_t v4_orig_dip, uint16_t dnat_epoch);

sdk_ret_t
create_dnat_map_ipv6(uint16_t vnic_id, ipv6_addr_t *v6_nat_dip, 
        ipv6_addr_t *v6_orig_dip, uint16_t dnat_epoch);

/* UDP Flows */
sdk_ret_t
athena_gtest_setup_flows_udp(void);

sdk_ret_t
athena_gtest_test_flows_udp(void);

/* TCP Flows */
sdk_ret_t
athena_gtest_setup_flows_tcp(void);

sdk_ret_t
athena_gtest_test_flows_tcp(void);

/* Slow Path Flows */
sdk_ret_t
athena_gtest_setup_flows_slowpath(void);

sdk_ret_t
athena_gtest_test_flows_slowpath(void);

/* ICMP Flows */
sdk_ret_t
athena_gtest_setup_flows_icmp(void);

sdk_ret_t
athena_gtest_test_flows_icmp(void);

/* NAT Flows */
sdk_ret_t
athena_gtest_setup_flows_nat(void);

sdk_ret_t
athena_gtest_test_flows_nat(void);

#endif /* __ATHENA_GTEST_HPP__ */
