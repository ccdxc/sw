//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_LOOKUP_NODE_H__
#define __VPP_INFRA_LOOKUP_NODE_H__

#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <nic/p4/common/defines.h>
#include <pkt.h>

#define foreach_p4cpu_hdr_lookup_next                   \
        _(DROP, "error-drop")                           \

#define foreach_p4cpu_hdr_lookup_counter                \
        _(SUCESS, "Lookup success")                     \
        _(FAILURE, "Lookup failed")                     \
        _(CONGESTION, "Thread handoff congestion")      \

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

#endif    // __VPP_INFRA_LOOKUP_NODE_H__
