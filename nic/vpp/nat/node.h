//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_NAT_NODE_H__
#define __VPP_NAT_NODE_H__

#define foreach_nat_counter                             \
        _(SUCCESS, "NAT success" )                      \
        _(FAILED, "NAT failed" )                        \

#define foreach_nat_next                                \
        _(IP4_FLOW_PROG, "pds-ip4-flow-program" )       \
        _(DROP, "error-drop")                           \

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

typedef struct nat_trace_s {
    // TODO
} nat_trace_t;

#endif    // __VPP_NAT_NODE_H__
