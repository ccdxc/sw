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
    u16 vpc_id;
    ip4_address_t pvt_ip;
    u16 pvt_port;
    ip4_address_t dip;
    u16 dport;
    u8 protocol;
    ip4_address_t public_ip;
    u16 public_port;
    // TODO
} nat_trace_t;

void pds_nat_cfg_init(void);

#endif    // __VPP_NAT_NODE_H__
