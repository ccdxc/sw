//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_ARP_PROXY_NODE_H__
#define __VPP_ARP_PROXY_NODE_H__

#include <nic/vpp/infra/utils.h>

// ARP Proxy node
#define foreach_arp_proxy_counter                           \
    _(REPLY_SUCCESS, "Reply success" )                      \
    _(REPLY_FAILED, "Reply failed, drop" )                  \

#define foreach_arp_proxy_next                              \
    _(EXIT, "pds-arp-proxy-exit")                           \
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

// ARP proxy exit node
#define foreach_arp_proxy_exit_counter                      \
    _(BUILD_P4_HDR, "Build P4 header" )                     \
    _(VNIC_MISSING_DROP, "vnic non-existing in vpp" )       \

#define foreach_arp_proxy_exit_next                         \
    _(INTF_OUT, "interface-tx" )                            \
    _(DROP, "error-drop")                                   \

typedef enum
{
#define _(n,s) ARP_PROXY_EXIT_COUNTER_##n,
    foreach_arp_proxy_exit_counter
#undef _
    ARP_PROXY_EXIT_COUNTER_LAST,
} arp_proxy_exit_counter_t;

typedef enum
{
#define _(n,s) ARP_PROXY_EXIT_NEXT_##n,
    foreach_arp_proxy_exit_next
#undef _
    ARP_PROXY_EXIT_N_NEXT,
} arp_proxy_exit_next_t;

typedef struct arp_proxy_trace_s {
    ip4_address_t src, dst;
    mac_addr_t smac, vr_mac;
    u16 bd;
} arp_proxy_trace_t;

#endif    // __VPP_ARP_PROXY_NODE_H__
