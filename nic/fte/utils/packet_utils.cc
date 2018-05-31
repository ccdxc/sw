//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Misc Packet utils to inject and modify packets.
//-----------------------------------------------------------------------------
#include "nic/include/base.h"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include "nic/p4/iris/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/fte.hpp"

#include "packet_utils.hpp"

using namespace hal;
using namespace hal::pd;

namespace fte {

namespace utils {

static void
build_arp_packet(uint8_t *pkt,
        uint8_t arp_pkt_type,
        const ip_addr_t *src_ip_addr,
        const mac_addr_t src_mac,
        const uint16_t *vlan_tag = nullptr,
        const ip_addr_t *dst_ip_addr = nullptr,
        const mac_addr_t dst_mac = nullptr)
{
    ether_header_t             *eth_hdr;
    struct ether_arp           *arphead;
    ip_addr_t                   ip_addr;
    vlan_header_t              *vlan_hdr;

    eth_hdr = (ether_header_t *)(pkt);

    if (vlan_tag) {
        arphead = (struct ether_arp *)(pkt + sizeof(vlan_header_t));
        vlan_hdr = (vlan_header_t *)(pkt);
        vlan_hdr->etype =  htons(ETH_TYPE_ARP);
        vlan_hdr->tpid = htons(ETH_TYPE_DOT1Q);
        vlan_hdr->vlan_tag = htons(*vlan_tag);
    } else {
        arphead = (struct ether_arp *)(pkt + L2_ETH_HDR_LEN);
        eth_hdr->etype = htons(ETH_TYPE_ARP);
    }

    if (dst_mac) {
        memcpy(eth_hdr->dmac, dst_mac, ETH_ADDR_LEN);
        memcpy(arphead->arp_tha, dst_mac, ETH_ADDR_LEN);
    } else {
        memset(eth_hdr->dmac, 0xff, ETH_ADDR_LEN);
    }
    memcpy(eth_hdr->smac, src_mac, ETH_ADDR_LEN);


    memcpy(arphead->arp_sha, src_mac, ETH_ADDR_LEN);
    ip_addr = *src_ip_addr;
    ip_addr.addr.v4_addr = htonl(ip_addr.addr.v4_addr);
    memcpy(arphead->arp_spa, &(ip_addr.addr.v4_addr),
            sizeof(src_ip_addr->addr.v4_addr));

    if (dst_ip_addr) {
        ip_addr = *dst_ip_addr;
        ip_addr.addr.v4_addr = htonl(ip_addr.addr.v4_addr);
        memcpy(arphead->arp_tpa, &(ip_addr.addr.v4_addr),
                sizeof(dst_ip_addr->addr.v4_addr));
    }

    arphead->ea_hdr.ar_op = htons(arp_pkt_type);
    arphead->ea_hdr.ar_pro = htons(ETHERTYPE_IPV4);
    arphead->ea_hdr.ar_hrd = htons(ARP_HARDWARE_TYPE);
    arphead->ea_hdr.ar_hln = ETH_ADDR_LEN;
    arphead->ea_hdr.ar_pln = IP4_ADDR8_LEN;
}

void
hal_build_arp_request_pkt(const mac_addr_t src_mac,
        const ip_addr_t *src_ip_addr,
        const ip_addr_t *dst_ip_addr, const uint16_t *vlan_tag,
        uint8_t *pkt)
{
    build_arp_packet(pkt, ARPOP_REQUEST,
            src_ip_addr, src_mac, vlan_tag, dst_ip_addr);
}

void
hal_build_arp_response_pkt(const mac_addr_t src_mac,
        const ip_addr_t *src_ip_addr,
        const mac_addr_t dst_mac, const ip_addr_t *dst_ip_addr,
        const uint16_t *vlan_tag, uint8_t *pkt)
{
    build_arp_packet(pkt, ARPOP_REPLY,
            src_ip_addr, src_mac, vlan_tag, dst_ip_addr, dst_mac);
}

hal_ret_t
hal_inject_arp_request_pkt(const l2seg_t *segment,
        const mac_addr_t src_mac, const ip_addr_t *src_ip_addr,
        const ip_addr_t *dst_ip_addr)
{
    uint8_t                     pkt[ARP_PKT_SIZE];
    cpu_to_p4plus_header_t      cpu_hdr = {0};
    p4plus_to_p4_header_t       p4plus_hdr = {0};
    pd_l2seg_get_fromcpu_vlanid_args_t   l2_args;
    hal_ret_t                   ret = HAL_RET_OK;


    cpu_hdr.src_lif = SERVICE_LIF_CPU_BYPASS;

    p4plus_hdr.flags = 0;
    p4plus_hdr.p4plus_app_id = P4PLUS_APPTYPE_CPU;

    l2_args.l2seg = (l2seg_t*)(segment);
    l2_args.vid = &(cpu_hdr.hw_vlan_id);

    if (hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                                      (void*)&l2_args) == HAL_RET_OK) {
        cpu_hdr.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
    }

    hal_build_arp_request_pkt(src_mac, src_ip_addr, dst_ip_addr, nullptr, pkt);

    fte::fte_asq_send(&cpu_hdr, &p4plus_hdr, pkt, ARP_PKT_SIZE);

    return ret;
}

hal_ret_t
hal_inject_arp_response_pkt(const l2seg_t *segment,
        const mac_addr_t src_mac, const ip_addr_t *src_ip_addr,
        const mac_addr_t dst_mac, const ip_addr_t *dst_ip_addr)
{
    uint8_t                     pkt[ARP_PKT_SIZE];
    cpu_to_p4plus_header_t      cpu_hdr = {0};
    p4plus_to_p4_header_t       p4plus_hdr = {0};
    pd_l2seg_get_fromcpu_vlanid_args_t   l2_args;
    hal_ret_t                   ret = HAL_RET_OK;


    cpu_hdr.src_lif = SERVICE_LIF_CPU_BYPASS;

    p4plus_hdr.flags = 0;
    p4plus_hdr.p4plus_app_id = P4PLUS_APPTYPE_CPU;

    l2_args.l2seg = (l2seg_t*)(segment);
    l2_args.vid = &(cpu_hdr.hw_vlan_id);

    if (hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                                      (void*)&l2_args) == HAL_RET_OK) {
        cpu_hdr.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
    }

    hal_build_arp_response_pkt(src_mac, src_ip_addr, dst_mac, dst_ip_addr, nullptr, pkt);

    fte::fte_asq_send(&cpu_hdr, &p4plus_hdr, pkt, ARP_PKT_SIZE);

    return ret;
}

} // namespace utils
} // namespace fte
