//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_ARP_PROXY_ARP_PROXY_H__
#define __VPP_ARP_PROXY_ARP_PROXY_H__

#define foreach_arp_proxy_counter                           \
    _(REPLY_SUCCESS, "Reply success" )                      \
    _(REPLY_FAILED, "Reply failed" )                        \

#define foreach_arp_proxy_next                              \
        _(INTF_OUT, "interface-tx" )                        \
        _(DROP, "error-drop")                               \

#define ETH_ADDR_LEN 6

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
    u8 smac[6], vr_mac[6];
    u16 vnic;
} arp_proxy_trace_t;

#endif    // __VPP_ARP_PROXY_ARP_PROXY_H__
