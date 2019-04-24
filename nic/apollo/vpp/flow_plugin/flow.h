//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PLUGIN_FLOW_H__
#define __VPP_FLOW_PLUGIN_FLOW_H__

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/udp/udp_packet.h>
#include <nic/p4/common/defines.h>
#include "flow_memhash.h"

#define MAX_FLOWS_PER_FRAME (VLIB_FRAME_SIZE * 2)

#define foreach_p4cpu_hdr_lookup_next                   \
        _(IP4_FLOW_PROG, "pen-ip4-flow-program" )       \
        _(IP6_FLOW_PROG, "pen-ip6-flow-program" )       \
        _(DROP, "error-drop")                           \

#define foreach_p4cpu_hdr_lookup_counter                \
        _(IP4_FLOW, "IPv4 flow packets" )               \
        _(IP6_FLOW, "IPv6 flow packets" )               \
        _(UNKOWN, "Unknown flow packets")               \

#define foreach_flow_prog_next                          \
        _(FWD_FLOW, "pen-fwd-flow" )                    \
        _(DROP, "error-drop")                           \

#define foreach_flow_prog_counter                       \
        _(FLOW_SUCCESS, "Flow programming success" )    \
        _(FLOW_FAILED, "Flow programming failed")       \

#define foreach_fwd_flow_next                           \
        _(INTF_OUT, "interface-tx" )                    \
        _(DROP, "error-drop")                           \

#define foreach_fwd_flow_counter                        \
        _(REWRITE_SUCCESS, "Rewrite success" )          \
        _(REWRITE_FAILED, "Rewrite failed" )            \

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

typedef enum
{
#define _(s,n) FLOW_PROG_NEXT_##s,
    foreach_flow_prog_next
#undef _
    FLOW_PROG_N_NEXT,
} flow_prog_next_t;

typedef enum
{
#define _(n,s) FLOW_PROG_COUNTER_##n,
    foreach_flow_prog_counter
#undef _
    FLOW_PROG_COUNTER_LAST,
} flow_prog_counter_t;

typedef enum
{
#define _(s,n) FWD_FLOW_NEXT_##s,
    foreach_fwd_flow_next
#undef _
    FWD_FLOW_N_NEXT,
} fwd_flow_next_t;

typedef enum
{
#define _(n,s) FWD_FLOW_COUNTER_##n,
    foreach_fwd_flow_counter
#undef _
    FWD_FLOW_COUNTER_LAST,
} fwd_flow_counter_t;

typedef struct fwd_flow_trace_t {
    u32 hw_index;
} fwd_flow_trace_t;

typedef struct flow_prog_trace_t {
    ip46_address_t src, dst;
    u16 src_port, dst_port;
    u8 protocol;
} flow_prog_trace_t;

typedef struct p4cpu_hdr_lookup_trace_t {
    u32 l2_offset;
    u32 l3_offset;
    u32 l4_offset;
    u32 vrf;
    u32 flow_hash;
    u32 flags;
} p4cpu_hdr_lookup_trace_t;

void pen_flow_prog_lock(void);

void pen_flow_prog_unlock(void);

mem_hash * pen_flow_prog_get_table(void);

#endif    // __VPP_FLOW_PLUGIN_FLOW_H__
