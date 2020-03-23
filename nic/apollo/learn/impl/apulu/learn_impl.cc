//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apulu pipeline specific abstractions needed by learn module
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/l2.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/api/impl/apulu/nacl_data.h"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "nic/apollo/packet/apulu/p4_cpu_hdr.h"

namespace learn {
namespace impl {

using namespace sdk::types;
using api::impl::subnet_impl_db;
using api::impl::lif_impl_db;
using api::impl::vnic_impl;

uint16_t
arm_to_p4_hdr_sz (void)
{
    return APULU_ARM_TO_P4_HDR_SZ;
}

uint16_t
p4_to_arm_hdr_sz (void)
{
    return APULU_P4_TO_ARM_HDR_SZ;
}

const char *
learn_lif_name (void)
{
    return "net_ionic1";
}

// reverse lookup subnet from hw id
static pds_obj_key_t
bdid_to_subnet (uint16_t bd_hw_id)
{

    api::impl::subnet_impl *impl;
    pds_obj_key_t *subnet;

    impl = subnet_impl_db()->find(bd_hw_id);
    if (!impl) {
        return k_pds_obj_key_invalid;
    }
    subnet = impl->key();
    return *subnet;
}

// extract vnic encap type from the incoming packet, we need encap details to
// create vnic
static void
fill_encap (char *eth_hdr, pds_encap_t *encap, bool vlan_valid)
{
    uint16_t tpid;
    uint8_t offset;

    if (vlan_valid) {
        offset = 2 * ETH_ADDR_LEN;
        tpid = (eth_hdr[offset] << 8) | eth_hdr[offset + 1];
        if (tpid == ETH_TYPE_DOT1Q) {
            offset += 2;
            encap->type = PDS_ENCAP_TYPE_DOT1Q;
            encap->val.vlan_tag = ((eth_hdr[offset] << 4) & 0xf) |
                                  eth_hdr[offset + 1];
            return;
        }
    }
    encap->type = PDS_ENCAP_TYPE_NONE;
}

sdk_ret_t
extract_info_from_p4_hdr (char *pkt, learn_info_t *info)
{
    bool vlan_valid = false;
    p4_rx_cpu_hdr_t *p4_rx_hdr = (p4_rx_cpu_hdr_t *)pkt;

    info->hints = 0;
    info->lif = p4_rx_hdr->lif;
    info->subnet = bdid_to_subnet(p4_rx_hdr->ingress_bd_id);
    if (info->subnet == k_pds_obj_key_invalid) {
        return SDK_RET_ERR;
    }
    info->l2_offset = APULU_P4_TO_ARM_HDR_SZ;
    if (p4_rx_hdr->flags & APULU_CPU_FLAGS_VLAN_VALID) {
        // assuming only single dot1q header
        info->l3_offset = info->l2_offset + L2_DOT1Q_HDR_LEN;
        vlan_valid = true;
    } else {
        info->l3_offset = info->l2_offset + L2_ETH_HDR_LEN;
    }

    fill_encap(pkt + info->l2_offset, &info->encap, vlan_valid);

    info->pkt_type = PKT_TYPE_NONE;

    // infer pkt type from from nacl data field
    switch (p4_rx_hdr->nacl_data) {
    case NACL_DATA_ID_L2_MISS_ARP:
        info->pkt_type = PKT_TYPE_ARP;
        break;
    case NACL_DATA_ID_ARP_REPLY:
        info->pkt_type = PKT_TYPE_ARP;
        info->hints |= LEARN_HINT_ARP_REPLY;
        break;
    case NACL_DATA_ID_L2_MISS_DHCP:
        info->pkt_type = PKT_TYPE_DHCP;
        break;
    default:
        // currently no other packets are sent to learn lif, we can however
        // infer from flags if we have an IPv4 or IPv6 packet
        if (p4_rx_hdr->flags & APULU_CPU_FLAGS_IPV4_1_VALID) {
            info->pkt_type = PKT_TYPE_IPV4;
        } else if (p4_rx_hdr->flags & APULU_CPU_FLAGS_IPV6_1_VALID) {
            info->pkt_type = PKT_TYPE_IPV6;
        }
        // TODO: NDP
    }
    return SDK_RET_OK;
}

void
arm_to_p4_tx_hdr_fill (char *tx_hdr, p4_tx_info_t *tx_info)
{
    uint16_t nh_idx;
    p4_tx_cpu_hdr_t *p4_tx_hdr = (p4_tx_cpu_hdr_t *)tx_hdr;

    memset(p4_tx_hdr, 0, sizeof(*p4_tx_hdr));
    p4_tx_hdr->lif_sbit0_ebit7  = tx_info->slif & 0xff;
    p4_tx_hdr->lif_sbit8_ebit10  = (tx_info->slif >> 0x8) & 0x7;

    switch (tx_info->nh_type) {
    case LEARN_NH_TYPE_DATAPATH_MNIC:
        nh_idx = lif_impl_db()->find(sdk::platform::LIF_TYPE_MNIC_CPU)->nh_idx();
        p4_tx_hdr->nexthop_valid = 1;
        break;
    case LEARN_NH_TYPE_VNIC:
        vnic_impl *vnic_impl_obj;
        vnic_impl_obj = (vnic_impl *)vnic_db()->find(&tx_info->vnic_key)->impl();
        nh_idx = vnic_impl_obj->nh_idx();
        p4_tx_hdr->nexthop_valid = 1;
        break;
    default:
        // nexthop_valid is intialized to zero
        p4_tx_hdr->local_mapping_override = 1;
        break;
    }

    if (p4_tx_hdr->nexthop_valid) {
        p4_tx_hdr->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
        p4_tx_hdr->nexthop_id = htons(nh_idx);
    }
    p4_tx_hdr->lif_flags = htons(p4_tx_hdr->lif_flags);
}

sdk_ret_t
remote_mapping_find (pds_mapping_key_t *key)
{
    // TODO: replace this with more efficient lookup as we are not interested in
    // remote mapping spec but just the presence of mapping
    pds_remote_mapping_info_t info;

    return api::pds_remote_mapping_read(key, &info);
}

}    // namespace impl
}    // namespace learn
