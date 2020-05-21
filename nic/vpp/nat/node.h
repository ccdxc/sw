//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_NAT_NODE_H__
#define __VPP_NAT_NODE_H__

#include "nat_api.h"

#define foreach_nat_counter                                 \
        _(SUCCESS, "NAT success" )                          \
        _(FAILED, "NAT failed" )                            \
        _(ERR_DEALLOC, "NAT dealloc due to error" )         \
        _(INVALID_EXISTS, "Invalid NAT flow, PB exists" )   \
        _(INVALID, "Invalid NAT flow, PB does not exist" )  \
        _(ICMP_ERR, "ICMP error packets sent to the host" ) \

#define foreach_nat_next                                    \
        _(IP4_FLOW_PROG, "pds-ip4-flow-program" )           \
        _(IP4_LINUX_INJECT, "pds-ip4-linux-inject" )        \
        _(HOST_TX, "pds-vnic-l2-rewrite" )                        \
        _(DROP, "pds-error-drop")                           \

typedef enum
{
#define _(n,s) NAT_COUNTER_##n,
    foreach_nat_counter
#undef _
    NAT_COUNTER_LAST,
} nat_counter_t;

typedef enum
{
#define _(n,s) NAT_NEXT_##n,
    foreach_nat_next
#undef _
    NAT_N_NEXT,
} nat_next_t;

typedef enum nat_node_type_s {
    NAT_NODE_ALLOC,
    NAT_NODE_DEALLOC,
    NAT_NODE_INVALIDATE,
    NAT_NODE_ICMP_ERROR
} nat_node_type_t;

typedef struct nat_trace_s {
    ip4_address_t pvt_ip;
    ip4_address_t dip;
    ip4_address_t public_ip;
    u16 vpc_id;
    u16 pvt_port;
    u16 dport;
    u16 public_port;
    u8 protocol;
    u8 err;
    u8 type;
} nat_trace_t;

typedef struct nat_node_main_s {
    nat_vendor_invalidate_cb invalidate_cb;
} nat_node_main_t;

extern nat_node_main_t nat_node_main;

void pds_nat_cfg_init(void);

#endif    // __VPP_NAT_NODE_H__
