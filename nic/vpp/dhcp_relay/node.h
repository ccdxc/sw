//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_DHCP_RELAY_NODE_H__
#define __VPP_DHCP_RELAY_NODE_H__

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <nic/vpp/infra/pkt.h>
#include <nic/vpp/infra/utils.h>

#define PDS_DHCP_OPT_82_CIRC_ID             1
#define PDS_DHCP_OPT_82_CIRC_ID_LEN         5
#define PDS_DHCP_OPT_82_LINK_SEL            5
#define PDS_DHCP_OPT_82_LINK_SEL_LEN        4
#define PDS_DHCP_OPT_82_SVR_IDENT           11
#define PDS_DHCP_OPT_82_SVR_IDENT_LEN       4
#define PDS_DHCP_OPT_82_VSS                 151
#define PDS_DHCP_OPT_82_VSS_LEN             8
#define PDS_DHCP_OPT_82_VSS_CONTROL         152
#define PDS_DHCP_OPT_82_VSS_CONTROL_LEN     0
#define PDS_IP4_PREFIX_LEN                  4
#define PDS_VNIC_ID_LEN                     2

#define PDS_LOCAL_DHCP_SERVER_ADDR          0xa9fe0002
#define PDS_LOCAL_DHCP_AGENT_ADDR           0xa9fe0003

// clfy node related defines
#define foreach_dhcp_relay_clfy_counter                     \
    _(TO_SERVER, "Sent to Server " )                        \
    _(TO_CLIENT, "Sent to Client" )                         \

#define foreach_dhcp_relay_clfy_next                        \
        _(TO_SERVER, "dhcp-proxy-to-server" )               \
        _(TO_CLIENT, "dhcp-proxy-to-client")                \

// server header node related defines
#define foreach_dhcp_relay_svr_hdr_counter                  \
    _(TX, "Sent on Network interface " )                    \

#define foreach_dhcp_relay_svr_hdr_next                     \
    _(INTF_OUT, "interface-tx" )                            \
    _(DROP, "error-drop")                                   \
// client header node related defines
#define foreach_dhcp_relay_client_hdr_counter               \
    _(TX, "Sent on host interface " )                       \

#define foreach_dhcp_relay_client_hdr_next                  \
    _(INTF_OUT, "interface-tx" )                            \
    _(DROP, "error-drop")                                   \

#define foreach_dhcp_relay_linux_inject_counter                \
    _(TX, "Reinjected to Linux" )                           \
    _(SOCK_ERR, "Socket create error" )                     \
    _(SEND_ERR, "Socket send error" )                       \

// clfy node related defines
typedef enum
{
#define _(n,s) DHCP_RELAY_CLFY_COUNTER_##n,
    foreach_dhcp_relay_clfy_counter
#undef _
    DHCP_RELAY_CLFY_COUNTER_LAST,
} dhcp_relay_clfy_counter_t;

typedef enum
{
#define _(n,s) PDS_DHCP_RELAY_CLFY_NEXT_##n,
    foreach_dhcp_relay_clfy_next
#undef _
    PDS_DHCP_RELAY_CLFY_N_NEXT,
} dhcp_relay_clfy_next_t;

typedef struct dhcp_relay_clfy_trace_s {
    uint16_t lif;    
} dhcp_relay_clfy_trace_t;

// server header node related defines
typedef enum
{
#define _(n,s) DHCP_RELAY_SVR_HDR_COUNTER_##n,
    foreach_dhcp_relay_svr_hdr_counter
#undef _
    DHCP_RELAY_SVR_HDR_COUNTER_LAST,
} dhcp_relay_svr_hdr_counter_t;

typedef enum
{
#define _(n,s) PDS_DHCP_RELAY_SVR_HDR_NEXT_##n,
    foreach_dhcp_relay_svr_hdr_next
#undef _
    PDS_DHCP_RELAY_SVR_HDR_N_NEXT,
} dhcp_relay_svr_hdr_next_t;

typedef struct dhcp_relay_svr_hdr_trace_s {
    uint16_t next_hop;
} dhcp_relay_svr_hdr_trace_t;

// client header node related defines
typedef enum
{
#define _(n,s) DHCP_RELAY_CLIENT_HDR_COUNTER_##n,
    foreach_dhcp_relay_client_hdr_counter
#undef _
    DHCP_RELAY_CLIENT_HDR_COUNTER_LAST,
} dhcp_relay_client_hdr_counter_t;

typedef enum
{
#define _(n,s) PDS_DHCP_RELAY_CLIENT_HDR_NEXT_##n,
    foreach_dhcp_relay_client_hdr_next
#undef _
    PDS_DHCP_RELAY_CLIENT_HDR_N_NEXT,
} dhcp_relay_client_hdr_next_t;

typedef struct dhcp_relay_client_hdr_trace_s {
    mac_addr_t client_mac;
} dhcp_relay_client_hdr_trace_t;

typedef enum
{
#define _(n,s) DHCP_RELAY_LINUX_INJECT_COUNTER_##n,
    foreach_dhcp_relay_linux_inject_counter
#undef _
    DHCP_RELAY_LINUX_INJECT_COUNTER_LAST,
} dhcp_relay_linnux_out_counter_t;

typedef struct dhcp_relay_linux_inject_trace_s {
    int error;
    int sys_errno;
} dhcp_relay_linux_inject_trace_t;

typedef struct dhcp_relay_main_s {
    int *inject_fds;
} dhcp_relay_main_t;

#endif    // __VPP_DHCP_RELAY_NODE_H__
