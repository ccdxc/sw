// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __P4PD_DEFAULTS_HPP__
#define __P4PD_DEFAULTS_HPP__

// NWSEC profile table
#define L4_PROF_DEFAULT_ENTRY       0

// Flow info table
#define FLOW_INFO_MISS_ENTRY        0
#define FLOW_INFO_DROP_ENTRY        1

// Flow Stats table
#define FLOW_STATS_NOP_ENTRY        0
#define FLOW_STATS_RSVD_ENTRY       1

// Session State table
#define SESSION_STATE_NOP_ENTRY     0

// Twice NAT table
#define TWICE_NAT_NOP_ENTRY         0

// NACL Stats table
#define NACL_STATS_NOP_ENTRY     0

// Rewrite table
#define REWRITE_NOP_ENTRY           0
#define REWRITE_DECAP_VLAN_ENTRY    1

// Tunnel Rewrite table
#define TUNNEL_REWRITE_NOP_ENTRY            0
#define TUNNEL_REWRITE_ENCP_VLAN_ENTRY      1

// MET(Replication List) table (From p4/iris/include/defins.h)
#define REPL_LIST_INDEX_FIN_COPY        P4_NW_MCAST_INDEX_FIN_COPY
#define REPL_LIST_INDEX_RST_COPY        P4_NW_MCAST_INDEX_RST_COPY
#define REPL_LIST_INDEX_FLOW_REL_COPY   P4_NW_MCAST_INDEX_FLOW_REL_COPY

// Mirror table
#define MIRROR_RESERVED_INDICES 5
#define MIRROR_RDMA_SNIFFER_BEGIN 5
#define MIRROR_RDMA_SNIFFER_END   6
#define MIRROR_FTE_SPAN_IDX 7


#endif    // __P4PD_DEFAULTS_HPP__

