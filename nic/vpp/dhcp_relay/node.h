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

// classify node related defines
#define foreach_dhcp_relay_clfy_counter                     \
    _(TO_PROXY_SERVER, "Sent to proxy server")              \
    _(TO_RELAY_SERVER, "Sent to relay server")              \
    _(TO_RELAY_CLIENT, "Sent to relay client")              \
    _(NO_VNIC, "VNIC not found")                            \
    _(NO_DHCP, "DHCP server not configured")                \

#define foreach_dhcp_relay_clfy_next                        \
    _(TO_PROXY_SERVER, "dhcp-proxy-to-server")              \
    _(TO_RELAY_SERVER, "dhcp-relay-to-server")              \
    _(TO_RELAY_CLIENT, "dhcp-relay-to-client")              \
    _(DROP, "error-drop")                                   \

// client tx node related defines
#define foreach_dhcp_relay_client_tx_counter                \
    _(TX, "Sent on host interface")                         \

#define foreach_dhcp_relay_client_tx_next                   \
    _(INTF_OUT, "interface-tx")                             \
    _(DROP, "error-drop")                                   \

// to server node related defines
#define foreach_dhcp_relay_to_server_counter                \
    _(TX, "Sent to DHCP server")                            \
    _(NO_VNIC, "VNIC not found")                            \
    _(NO_DHCP, "DHCP server not configured")                \
    _(LINEARIZE_FAILED, "Buffer linearization failed")       \
    _(PACKET_TOO_BIG, "Packet too big")                     \

#define foreach_dhcp_relay_to_server_next                   \
    _(LINUX_INJECT, "pds-ip4-linux-inject")                 \
    _(DROP, "error-drop")                                   \

// to client node related defines
#define foreach_dhcp_relay_to_client_counter                \
    _(TX, "Sent to DHCP client")                            \
    _(NO_VNIC, "VNIC not found")                            \
    _(NO_SUBNET, "Subnet not found")                        \
    _(LINEARIZE_FAILED, "Buffer linerization failed")       \
    _(INVALID_SERVER, "Matching DHCP server Not found")     \

#define foreach_dhcp_relay_to_client_next                   \
    _(CLIENT_TX, "pds-dhcp-relay-client-tx")                \
    _(DROP, "error-drop")                                   \

// classify node related defines
typedef enum {
#define _(n,s) DHCP_RELAY_CLFY_COUNTER_##n,
    foreach_dhcp_relay_clfy_counter
#undef _
    DHCP_RELAY_CLFY_COUNTER_LAST,
} dhcp_relay_clfy_counter_t;

typedef enum {
#define _(n,s) PDS_DHCP_RELAY_CLFY_NEXT_##n,
    foreach_dhcp_relay_clfy_next
#undef _
    PDS_DHCP_RELAY_CLFY_N_NEXT,
} dhcp_relay_clfy_next_t;

typedef struct dhcp_relay_clfy_trace_s {
    uint16_t lif;
} dhcp_relay_clfy_trace_t;

// client tx node related defines
typedef enum {
#define _(n,s) DHCP_RELAY_CLIENT_TX_COUNTER_##n,
    foreach_dhcp_relay_client_tx_counter
#undef _
    DHCP_RELAY_CLIENT_TX_COUNTER_LAST,
} dhcp_relay_client_tx_counter_t;

typedef enum {
#define _(n,s) PDS_DHCP_RELAY_CLIENT_TX_NEXT_##n,
    foreach_dhcp_relay_client_tx_next
#undef _
    PDS_DHCP_RELAY_CLIENT_TX_N_NEXT,
} dhcp_relay_client_tx_next_t;

typedef struct dhcp_relay_client_tx_trace_s {
    mac_addr_t client_mac;
} dhcp_relay_client_tx_trace_t;

// relay to server node related defines
typedef enum {
#define _(n,s) DHCP_RELAY_TO_SVR_COUNTER_##n,
    foreach_dhcp_relay_to_server_counter
#undef _
    DHCP_RELAY_TO_SVR_COUNTER_LAST,
} dhcp_relay_to_svr_counter_t;

typedef enum {
#define _(n,s) PDS_DHCP_RELAY_TO_SVR_NEXT_##n,
    foreach_dhcp_relay_to_server_next
#undef _
    PDS_DHCP_RELAY_TO_SVR_N_NEXT,
} dhcp_relay_to_svr_next_t;

// relay to client node related defines
typedef enum {
#define _(n,s) DHCP_RELAY_TO_CLIENT_COUNTER_##n,
    foreach_dhcp_relay_to_client_counter
#undef _
    DHCP_RELAY_TO_CLIENT_COUNTER_LAST,
} dhcp_relay_to_client_counter_t;

typedef enum {
#define _(n,s) PDS_DHCP_RELAY_TO_CLIENT_NEXT_##n,
    foreach_dhcp_relay_to_client_next
#undef _
    PDS_DHCP_RELAY_TO_CLIENT_N_NEXT,
} dhcp_relay_to_client_next_t;

typedef struct dhcp_relay_to_client_trace_s {
    ip46_address_t svr_addr;
    ip46_address_t relay_addr;
    u8 server_found;
} dhcp_relay_to_svr_client_trace_t;

#define PDS_OBJ_ID_LEN 16
typedef struct dhcp_relay_server_s {
    u8 obj_id[PDS_OBJ_ID_LEN + 1];
    ip46_address_t server_addr;
    ip46_address_t relay_addr;
    u16 server_vpc;
} dhcp_relay_server_t;

typedef struct dhcp_relay_policy_s {
    u16 subnet_hw_id;                   // subnet id
    u16 local_server;                   // local or remote server flag
    u16 *servers;   // pool pf servers for the subnet
} dhcp_relay_policy_t;

typedef struct dhcp_relay_main_s {
    u16 *policy_pool_idx;     // vector storing policy pool index for a subnet
    dhcp_relay_policy_t *policy_pool;   // policy configs
    dhcp_relay_server_t *server_pool;
} dhcp_relay_main_t;

extern dhcp_relay_main_t dhcp_relay_main;

#endif    // __VPP_DHCP_RELAY_NODE_H__
