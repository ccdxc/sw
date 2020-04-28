//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_LOOKUP_NODE_H__
#define __VPP_INFRA_LOOKUP_NODE_H__

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/vnet.h>
#include <nic/p4/common/defines.h>
#include <pkt.h>

#define MAX_FILE_PATH                    64
#define DEAFULT_PACKET_DUMP_SIZE        512

#define foreach_p4cpu_hdr_lookup_next                       \
        _(DROP, "pds-error-drop")                           \

#define foreach_vnic_l2_rewrite_next                        \
    _(TX_OUT, "pds-vnic-tx")                                \
    _(UNKNOWN, "pds-error-drop")                            \

#define foreach_vnic_tx_next                                \
    _(INTF_OUT, "interface-tx" )                            \
    _(DROP, "pds-error-drop")                               \

#define foreach_ip4_linux_inject_counter                    \
    _(TX, "Injected IPv4 packet to Linux" )                 \
    _(SOCK_ERR, "Socket create error" )                     \
    _(SEND_ERR, "Socket send error" )                       \

#define foreach_p4cpu_hdr_lookup_counter                    \
    _(SUCESS, "Lookup success")                             \
    _(FAILURE, "Lookup failed")                             \
    _(CONGESTION, "Thread hand-off congestion")             \

#define foreach_vnic_l2_rewrite_counter                     \
    _(TX_OUT, "Added l2 headers")                           \
    _(VNIC_NOT_FOUND, "VNIC not found")                     \
    _(SUBNET_NOT_FOUND, "Subnet not found")                 \

#define foreach_vnic_tx_counter                             \
    _(SUCCESS, "Sent to tx interface")                      \
    _(FAILED, "Failed to send to tx interface")             \

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
    bool packet_dump_en; // packet dump to file enabled or not
    u16 packet_dump_len;
    char packet_dump_path[MAX_FILE_PATH];
    FILE *packet_dump_fd;
    clib_spinlock_t packet_dump_lock;
} pds_infra_main_t;

typedef enum
{
#define _(n,s) VNIC_L2_REWRITE_NEXT_##n,
    foreach_vnic_l2_rewrite_next
#undef _
    VNIC_L2_REWRITE_N_NEXT,
} vnic_l2_rewrite_next_t;

typedef enum
{
#define _(n,s) VNIC_L2_REWRITE_COUNTER_##n,
    foreach_vnic_l2_rewrite_counter
#undef _
    VNIC_L2_REWRITE_COUNTER_LAST,
} vnic_l2_rewrite_counter_t;

typedef struct vnic_l2_rewrite_trace_s {
    u8 packet_data[64];
} vnic_l2_rewrite_trace_t;

typedef enum
{
#define _(n,s) VNIC_TX_NEXT_##n,
    foreach_vnic_tx_next
#undef _
    VNIC_TX_N_NEXT,
} vnic_tx_next_t;

typedef enum
{
#define _(n,s) VNIC_TX_COUNTER_##n,
    foreach_vnic_tx_counter
#undef _
    VNIC_TX_COUNTER_LAST,
} vnic_tx_counter_t;

typedef struct vnic_tx_trace_s {
    u16 vnic_nh_hw_id;
} vnic_tx_trace_t;

void pds_packet_dump_en_dis(bool enable, char *file, u16 size);
void pds_packet_dump_show(vlib_main_t *vm);

#endif    // __VPP_INFRA_LOOKUP_NODE_H__
