//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/test/athena/api/flow_session_rewrite/utils.hpp"

// Data templates
uint8_t smac[ETH_ADDR_LEN] = { 0x22, 0x33, 0x44, 0x0, 0x0, 0x0 };
uint8_t dmac[ETH_ADDR_LEN] = { 0x55, 0x66, 0x77, 0x0, 0x0, 0x0 };
uint16_t sport = 1024;
uint16_t dport = 2048;
uint32_t ip_saddr = 0x14000000;
uint32_t ip_daddr = 0x1E000000;
uint32_t mpls_label = 100;
uint32_t vni = 25;
uint32_t src_slot_id = 30;
uint32_t dst_slot_id = 35;
uint16_t sg_id = 42;
uint8_t ipv6_addr[IP6_ADDR8_LEN] = { 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xEE, 0xFF,
                                     0, 0, 0, 0, 0, 0, 0, 0 , 0};
uint32_t origin_ip_addr = 0x12340000;

void
fill_data (pds_flow_session_rewrite_data_t *data, uint32_t index,
           pds_flow_session_rewrite_nat_type_t nat_type,
           pds_flow_session_encap_t encap_type)
{
    uint8_t ipv6_rw_addr[IP6_ADDR8_LEN] = { 0 };

    data->strip_encap_header = index % 2;
    data->strip_l2_header = index % 2;
    data->strip_vlan_tag = index % 2;

    // Fill rewrite NAT data
    data->nat_info.nat_type = nat_type;
    if (nat_type == REWRITE_NAT_TYPE_IPV4_SNAT) {
        data->nat_info.u.ipv4_addr = ip_saddr + index;
    } else if (nat_type == REWRITE_NAT_TYPE_IPV4_DNAT) {
        data->nat_info.u.ipv4_addr = ip_daddr + index;
    } else if (nat_type == REWRITE_NAT_TYPE_IPV6_SNAT ||
               nat_type == REWRITE_NAT_TYPE_IPV6_DNAT) {
        memcpy(ipv6_rw_addr, ipv6_addr, IP6_ADDR8_LEN);
        ipv6_rw_addr[14] = index & 0xFF;
        ipv6_rw_addr[15] = index & 0xFF;
        memcpy(data->nat_info.u.ipv6_addr, ipv6_rw_addr, IP6_ADDR8_LEN);
    } else if (nat_type == REWRITE_NAT_TYPE_IPV4_SDPAT) {
        data->nat_info.u.pat.ip_saddr = ip_saddr + index;
        data->nat_info.u.pat.ip_daddr = ip_daddr + index;
        data->nat_info.u.pat.l4_sport = sport + index;
        data->nat_info.u.pat.l4_dport = dport + index;
    }

    // Fill rewrite encap data
    data->encap_type = encap_type;
    if (encap_type == ENCAP_TYPE_L2) {
        pds_flow_session_l2_encap_t *l2_encap = &data->u.l2_encap;
        memcpy(l2_encap->smac, smac, ETH_ADDR_LEN);
        memcpy(l2_encap->dmac, dmac, ETH_ADDR_LEN);
        l2_encap->insert_vlan_tag = index % 2;
        l2_encap->vlan_id = index & 0xFFF;
    } else if (encap_type == ENCAP_TYPE_MPLSOUDP) {
        pds_flow_session_mplsoudp_encap_t *mplsoudp_encap =
            &data->u.mplsoudp_encap;
        memcpy(mplsoudp_encap->l2_encap.smac, smac, ETH_ADDR_LEN);
        memcpy(mplsoudp_encap->l2_encap.dmac, dmac, ETH_ADDR_LEN);
        mplsoudp_encap->l2_encap.insert_vlan_tag = index % 2;
        mplsoudp_encap->l2_encap.vlan_id = index & 0xFFF;
        mplsoudp_encap->ip_encap.ip_saddr = ip_saddr + index;
        mplsoudp_encap->ip_encap.ip_daddr = ip_daddr + index;
        mplsoudp_encap->udp_encap.udp_sport = sport + index;
        mplsoudp_encap->udp_encap.udp_dport = dport + index;
        mplsoudp_encap->mpls1_label = mpls_label + 1;
        mplsoudp_encap->mpls2_label = mpls_label + 2;
        mplsoudp_encap->mpls2_label = mpls_label + 3;
    } else if (encap_type == ENCAP_TYPE_GENEVE) {
        pds_flow_session_geneve_encap_t *geneve_encap =
            &data->u.geneve_encap;
        memcpy(geneve_encap->l2_encap.smac, smac, ETH_ADDR_LEN);
        memcpy(geneve_encap->l2_encap.dmac, dmac, ETH_ADDR_LEN);
        geneve_encap->l2_encap.insert_vlan_tag = index % 2;
        geneve_encap->l2_encap.vlan_id = index & 0xFFF;
        geneve_encap->ip_encap.ip_saddr = ip_saddr + index;
        geneve_encap->ip_encap.ip_daddr = ip_daddr + index;
        geneve_encap->vni = vni + index;
        geneve_encap->source_slot_id = src_slot_id + index;
        geneve_encap->destination_slot_id = dst_slot_id + index;
        geneve_encap->sg_id1 = sg_id + index + 1;
        geneve_encap->sg_id2 = sg_id + index + 2;
        geneve_encap->sg_id3 = sg_id + index + 3;
        geneve_encap->sg_id4 = sg_id + index + 4;
        geneve_encap->sg_id5 = sg_id + index + 5;
        geneve_encap->sg_id6 = sg_id + index + 6;
        geneve_encap->originator_physical_ip = origin_ip_addr + index;
    }
    return;
}
