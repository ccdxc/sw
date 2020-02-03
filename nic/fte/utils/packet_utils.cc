//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Misc Packet utils to inject and modify packets.
//-----------------------------------------------------------------------------
#include "nic/include/base.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/fte.hpp"

#include "packet_utils.hpp"

using namespace hal;
using namespace hal::pd;

namespace fte {

namespace utils {

static uint32_t g_cpu_bypass_flowid;

static void
build_arp_packet(uint8_t *pkt,
        uint8_t arp_pkt_type,
        const ip_addr_t *src_ip_addr,
        const mac_addr_t src_mac,
        const uint16_t *vlan_tag = nullptr,
        const ip_addr_t *dst_ip_addr = nullptr,
        const mac_addr_t dst_mac = nullptr) {
    ether_header_t             *eth_hdr;
    struct ether_arp           *arphead;
    ip_addr_t                   ip_addr;
    vlan_header_t              *vlan_hdr;

    eth_hdr = (ether_header_t *)(pkt);

    if (vlan_tag) {
        arphead = (struct ether_arp *)(pkt + sizeof(vlan_header_t));
        vlan_hdr = (vlan_header_t *)(pkt);
        if ((arp_pkt_type == ARPOP_REQUEST) || (arp_pkt_type == ARPOP_REPLY)) {
            vlan_hdr->etype =  htons(ETH_TYPE_ARP);
        } else if (arp_pkt_type == ARPOP_REVREQUEST)  {
            vlan_hdr->etype =  htons(ETH_TYPE_RARP);
        }
        vlan_hdr->tpid = htons(ETH_TYPE_DOT1Q);
        vlan_hdr->vlan_tag = htons(*vlan_tag);
    } else {
        arphead = (struct ether_arp *)(pkt + L2_ETH_HDR_LEN);
        if ((arp_pkt_type == ARPOP_REQUEST) || (arp_pkt_type == ARPOP_REPLY)) {
            eth_hdr->etype = htons(ETH_TYPE_ARP);
        } else if (arp_pkt_type == ARPOP_REVREQUEST)  {
            eth_hdr->etype = htons(ETH_TYPE_RARP);
            memcpy(arphead->arp_tha, src_mac, ETH_ADDR_LEN);
        }
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


static hal_ret_t
hal_inject_arp_packet(const arp_pkt_data_t *pkt_data, uint32_t type) {
    uint8_t                     pkt[ARP_DOT1Q_PKT_SIZE];
    cpu_to_p4plus_header_t      cpu_hdr = {0};
    p4plus_to_p4_header_t       p4plus_hdr = {0};
    pd_func_args_t pd_func_args = {0};
    hal_ret_t                   ret = HAL_RET_OK;
    hal::if_t                   *ep_if;
    hal::if_t                   *uplink_if = NULL;
    hal_handle_t                l2seg_hdl = 0;
    l2seg_t                     *l2seg = NULL;
    pd::pd_if_get_lport_id_args_t args;
    uint16_t                    vlan_id;

    if (pkt_data->ep == nullptr) {
         ret = HAL_RET_INVALID_ARG;
         goto out;
     }

     ep_if = hal::find_if_by_handle(pkt_data->ep->if_handle);
     if (ep_if == nullptr) {
         HAL_TRACE_ERR("EP interface not found..");
         ret = HAL_RET_IF_NOT_FOUND;
         goto out;
     }

    if (type == ARPOP_REVREQUEST) {
        ret = if_enicif_get_pinned_if(ep_if, &uplink_if);
        if ((ret != HAL_RET_OK) || (!uplink_if)) {
            HAL_TRACE_ERR("EP uplink not found..");
            ret = HAL_RET_IF_NOT_FOUND;
            goto out;
        }
        args.pi_if = uplink_if;
    } else {
        args.pi_if = ep_if;
    }
    pd_func_args.pd_if_get_lport_id = &args;
    if (hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_LPORT_ID,
                    &pd_func_args) != HAL_RET_OK) {
            HAL_TRACE_ERR("EP Dest Lport not found..");
            ret = HAL_RET_IF_NOT_FOUND;
            goto out;
    }

    if (!g_cpu_bypass_flowid) {
        pd_get_cpu_bypass_flowid_args_t args;
        pd::pd_func_args_t          pd_func_args = {0};
        args.hw_flowid = 0;
        pd_func_args.pd_get_cpu_bypass_flowid = &args;
        ret = hal_pd_call(PD_FUNC_ID_BYPASS_FLOWID_GET, &pd_func_args);
        if (ret == HAL_RET_OK) {
            g_cpu_bypass_flowid = args.hw_flowid;
        }
    }
    cpu_hdr.src_lif = SERVICE_LIF_CPU_BYPASS;

    p4plus_hdr.p4plus_app_id = P4PLUS_APPTYPE_CPU;
    p4plus_hdr.flow_index_valid = 1;
    p4plus_hdr.flow_index = g_cpu_bypass_flowid;
    p4plus_hdr.dst_lport_valid = 1;
    p4plus_hdr.dst_lport = args.lport_id;

    if (type == ARPOP_REVREQUEST) {
        l2seg_hdl = pkt_data->ep->l2seg_handle;
        l2seg     = l2seg_lookup_by_handle(l2seg_hdl);
        if (!l2seg) {
          HAL_TRACE_ERR("l2seg not found...");
          ret = HAL_RET_IF_NOT_FOUND;
          goto out;
        }
        vlan_id = (uint16_t)(l2seg->wire_encap.val);
        build_arp_packet(pkt, type,
            pkt_data->src_ip_addr, *pkt_data->src_mac,
            &vlan_id, pkt_data->dst_ip_addr, NULL);
    } else {
        vlan_id = (uint16_t)(ep_if->encap_vlan);
        build_arp_packet(pkt, type,
            pkt_data->src_ip_addr, *pkt_data->src_mac,
            &vlan_id, pkt_data->dst_ip_addr, pkt_data->ep->l2_key.mac_addr);
    }

    HAL_TRACE_VERBOSE("Sending ARP of type {} for Mac {} Vlan {} lport {}",
        type, macaddr2str(pkt_data->ep->l2_key.mac_addr), vlan_id, args.lport_id);

    fte::fte_asq_send(&cpu_hdr, &p4plus_hdr, pkt, ARP_DOT1Q_PKT_SIZE);

out:
    return ret;
}

hal_ret_t
hal_inject_arp_request_pkt (const arp_pkt_data_t *pkt_data) {
    return hal_inject_arp_packet(pkt_data, ARPOP_REQUEST);
}

hal_ret_t
hal_inject_arp_response_pkt (const arp_pkt_data_t *pkt_data) {
    return hal_inject_arp_packet(pkt_data, ARPOP_REPLY);
}

hal_ret_t
hal_inject_rarp_request_pkt (const arp_pkt_data_t *pkt_data) {
    return hal_inject_arp_packet(pkt_data, ARPOP_REVREQUEST);
}


} // namespace utils
} // namespace fte
