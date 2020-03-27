//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_ARP_PROXY_NODE_H__
#define __VPP_ARP_PROXY_NODE_H__

#include <nic/vpp/infra/utils.h>
#include "arp_proxy.h"

// ARP Proxy node
#define foreach_arp_proxy_counter                           \
    _(REPLY_SUCCESS, "Reply success" )                      \
    _(SUBNET_CHECK_FAIL, "Subnet check failed")             \
    _(VNIC_MISSING, "VNIC not present")                     \
    _(NO_MAC, "MAC not found for ARP request")              \
    _(NOT_ARP_REQUEST, "Not an ARP request" )               \
    _(DAD_DROP, "Duplicate address detection drops")        \

#define foreach_arp_proxy_next                              \
    _(EXIT, "pds-vnic-tx")                                  \
    _(DROP, "error-drop")

typedef enum
{
#define _(n,s) ARP_PROXY_COUNTER_##n,
    foreach_arp_proxy_counter
#undef _
    ARP_PROXY_COUNTER_LAST,
} arp_proxy_counter_t;

typedef enum
{
#define _(n,s) ARP_PROXY_NEXT_##n,
    foreach_arp_proxy_next
#undef _
    ARP_PROXY_N_NEXT,
} arp_proxy_next_t;

typedef struct arp_proxy_trace_s {
    ip4_address_t src, dst;
    mac_addr_t smac, vr_mac;
    u16 bd;
} arp_proxy_trace_t;

typedef struct arp_proxy_main_s {
    arp_proxy_vendor_dst_mac_get_cb dst_mac_get_cb;
} arp_proxy_main_t;

extern arp_proxy_main_t arp_proxy_main;

#endif    // __VPP_ARP_PROXY_NODE_H__
