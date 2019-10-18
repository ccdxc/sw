//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_NODE_H__
#define __VPP_INFRA_NODE_H__

#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <nic/p4/common/defines.h>
#include "pkt.h"
#include "hw_program.h"

#define foreach_p4cpu_hdr_lookup_next                               \
        _(IP4_FLOW_PROG, "pds-ip4-flow-program" )                   \
        _(IP6_FLOW_PROG, "pds-ip6-flow-program" )                   \
        _(IP4_TUN_FLOW_PROG, "pds-tunnel-ip4-flow-program" )        \
        _(IP6_TUN_FLOW_PROG, "pds-tunnel-ip6-flow-program" )        \
        _(ARP_PROXY, "pds-arp-proxy")                                   \
        _(DROP, "error-drop")                                       \

#define foreach_p4cpu_hdr_lookup_counter                            \
        _(IP4_FLOW, "IPv4 flow packets" )                           \
        _(IP6_FLOW, "IPv6 flow packets" )                           \
        _(IP4_TUN_FLOW, "IPv4 tunnel flow packets" )                \
        _(IP6_TUN_FLOW, "IPv6 tunnel flow packets" )                \
        _(ARP_PROXY, "Arp-proxy")                                   \
        _(UNKOWN, "Unknown flow packets")                           \

typedef enum
{
#define _(s,n) P4CPU_HDR_LOOKUP_NEXT_##s,
    foreach_p4cpu_hdr_lookup_next
#undef _
    P4CPU_HDR_LOOKUP_N_NEXT,
} p4cpu_hdr_lookup_next_t;

typedef enum
{
#define _(n,s) P4CPU_HDR_LOOKUP_COUNTER_##n,
    foreach_p4cpu_hdr_lookup_counter
#undef _
    P4CPU_HDR_LOOKUP_COUNTER_LAST,
} p4cpu_hdr_lookup_counter_t;

typedef struct p4cpu_hdr_lookup_trace_s {
    u32 l2_offset;
    u32 l3_offset;
    u32 l4_offset;
    u32 vnic;
    u32 flow_hash;
    u32 flags;
} p4cpu_hdr_lookup_trace_t;

#endif    // __VPP_INFRA_NODE_H__
