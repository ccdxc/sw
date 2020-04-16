#ifndef __PDS_POLICY_HPP__
#define __PDS_POLICY_HPP__

#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

namespace fte_ath {

#define MAX_VNIC_ID 512
#define MAX_VLAN_ID 0x1000        // 2^12 = 4096
#define MAX_MPLS_LABEL 0x100000   // 2^20 = 1048576
#define MAX_V4_FLOWS 32
#define MAX_NAT_IP 32

#define ENCAP_MPLSOUDP 1
#define ENCAP_GENEVE   2

typedef struct session_info_s {
    uint8_t tcp_flags;
    uint64_t policer_bw1;
    uint8_t host_mac[ETH_ADDR_LEN];
} session_info_t;

typedef struct rewrite_underlay_info_s {
    uint32_t rewrite_id;
    uint8_t encap_type;
    uint8_t substrate_smac[ETH_ADDR_LEN];
    uint8_t substrate_dmac[ETH_ADDR_LEN];
    uint16_t substrate_vlan;
    uint32_t substrate_sip;
    uint32_t substrate_dip;
    uint32_t mpls_label1;
    uint32_t mpls_label2;
} rewrite_underlay_info_t;

typedef struct rewrite_host_info_s {
    uint32_t rewrite_id;
    uint8_t ep_smac[ETH_ADDR_LEN];
    uint8_t ep_dmac[ETH_ADDR_LEN];
} rewrite_host_info_t;

typedef struct nat_info_s {
    uint32_t local_ip_lo;
    uint32_t local_ip_hi;
    uint32_t nat_ip_lo;
    uint32_t nat_ip_hi;
} nat_info_t;

typedef struct nat_map_tbl_s {
    uint32_t local_ip;
    uint32_t nat_ip;
    uint32_t h2s_rewrite_id;
    uint32_t s2h_rewrite_id;
} nat_map_tbl_t;
    
typedef struct v4_flows_info_s {
    uint16_t vnic_lo;
    uint16_t vnic_hi;
    uint32_t sip_lo;
    uint32_t sip_hi;
    uint32_t dip_lo;
    uint32_t dip_hi;
    uint8_t proto;
    uint16_t sport_lo;
    uint16_t sport_hi;
    uint16_t dport_lo;
    uint16_t dport_hi;
} v4_flows_info_t;

typedef struct flow_cache_policy_info_s {
    uint16_t vlan_id;
    uint16_t vnic_id;
    uint32_t src_slot_id;
    bool skip_flow_log;
    uint32_t epoch;
    session_info_t to_host;
    session_info_t to_switch;
    rewrite_underlay_info_t rewrite_underlay;
    rewrite_host_info_t rewrite_host;
    bool nat_enabled;
    nat_info_t nat_info;
    uint8_t num_nat_mappings;
    nat_map_tbl_t nat_map_tbl[MAX_NAT_IP];
} flow_cache_policy_info_t;

sdk_ret_t parse_flow_cache_policy_cfg(const char *cfg_file);

extern uint16_t g_vlan_to_vnic[MAX_VLAN_ID];
extern uint16_t g_mpls_label_to_vnic[MAX_MPLS_LABEL];
extern flow_cache_policy_info_t g_flow_cache_policy[MAX_VNIC_ID];
extern uint16_t g_vnic_id_list[MAX_VNIC_ID];
extern uint32_t g_num_policies;
extern v4_flows_info_t g_v4_flows[MAX_V4_FLOWS];
extern uint8_t g_num_v4_flows;

}

#endif
