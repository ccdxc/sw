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

#define foreach_ip4_linux_inject_counter                \
    _(TX, "Injected IPv4 packet to Linux" )             \
    _(SOCK_ERR, "Socket create error" )                 \
    _(SEND_ERR, "Socket send error" )                   \

#define foreach_p4cpu_hdr_lookup_counter                \
        _(SUCESS, "Lookup success")                     \
        _(FAILURE, "Lookup failed")                     \
        _(CONGESTION, "Thread hand-off congestion")     \

typedef struct ip4_linux_inject_trace_s {
    int error;
    int sys_errno;
} ip4_linux_inject_trace_t;

typedef enum
{
#define _(s,n) P4CPU_HDR_LOOKUP_NEXT_##s,
    foreach_p4cpu_hdr_lookup_next
#undef _
    P4CPU_HDR_LOOKUP_N_NEXT,
} p4cpu_hdr_lookup_next_t;

typedef enum {
#define _(n,s) IP4_LINUX_INJECT_COUNTER_##n,
    foreach_ip4_linux_inject_counter
#undef _
    IP4_LINUX_INJECT_COUNTER_LAST,
} ip4_linnux_out_counter_t;

typedef enum
{
#define _(n,s) P4CPU_HDR_LOOKUP_COUNTER_##n,
    foreach_p4cpu_hdr_lookup_counter
#undef _
    P4CPU_HDR_LOOKUP_COUNTER_LAST,
} p4cpu_hdr_lookup_counter_t;

typedef struct pds_infra_main_s {
    int *ip4_linux_inject_fds;  // raw socket fd pool to inject packets to linux
} pds_infra_main_t;

#endif    // __VPP_INFRA_LOOKUP_NODE_H__
