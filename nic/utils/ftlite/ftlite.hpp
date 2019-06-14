//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_HPP__
#define __FTLITE_HPP__

#include <stdint.h>

#include "include/sdk/base.hpp"
#include "ftlite_ipv4_structs.hpp"
#include "ftlite_ipv6_structs.hpp"

namespace ftlite {

struct __attribute__((__packed__)) pkt_meta_t {
    uint16_t flags;
    uint16_t len;
    uint16_t local_vnic_tag;
};

struct flow_table_meta_t {
    struct __attribute__((__packed__)) {
        uint32_t msb : 9;
        uint32_t index : 23;
    } hash;
    uint32_t ipaf : 1;
    uint32_t ptype : 1; // Parent Table Type
    uint32_t pslot : 3; // Parent Hint Slot
    uint32_t nrecircs : 3;
    uint32_t pindex; // Parent Index
};

struct __attribute__((__packed__)) ipv4_flow_meta_t {
    flow_table_meta_t tblmeta;
    struct {
        ftlite_ipv4_entry_t entry;
        uint8_t pad[32];
    } parent;
    struct {
        ftlite_ipv6_entry_t entry;
    } leaf;
};

struct __attribute__((__packed__)) ipv6_flow_meta_t {
    flow_table_meta_t tblmeta;
    struct {
        ftlite_ipv6_entry_t entry;
    } parent;
    struct {
        ftlite_ipv6_entry_t entry;
    } leaf;
};

union __attribute__((__packed__)) flow_meta_t {
    ipv4_flow_meta_t ipv4meta;
    ipv6_flow_meta_t ipv6meta;
};
  
struct __attribute__((__packed__)) session_meta_t {
    struct __attribute__((__packed__)) {
        uint32_t tcp_state : 4;
        uint32_t tcp_seq_num : 32;
        uint32_t tcp_ack_num : 32;
        uint32_t tcp_win_sz : 16;
        uint32_t tcp_win_scale : 4;
    } iflow;
    struct __attribute__((__packed__)) {
        uint32_t tcp_state : 4;
        uint32_t tcp_seq_num : 32;
        uint32_t tcp_ack_num : 32;
        uint32_t tcp_win_sz : 16;
        uint32_t tcp_win_scale : 4;
    } rflow;
    uint32_t tx_dst_ip[4];
    uint32_t tx_dst_l4port : 16;
    uint32_t nexthop_indx : 20;
    uint32_t tx_rewrite_flags : 8;
    uint32_t rx_rewrite_flags : 8;
    uint32_t tx_policer_idx : 12;
    uint32_t rx_policer_idx : 12;
    uint32_t meter_idx : 16;
    uint32_t timestamp1;
    uint32_t timestamp2 : 16;
    uint32_t drop : 1;
    uint32_t pad1 : 32;
    uint32_t pad2 : 32;
    uint32_t pad3 : 3;
};

struct __attribute__((__packed__)) insert_params_t {
    pkt_meta_t pktmeta;
    flow_meta_t iflow;
    flow_meta_t rflow;
    session_meta_t session;
};

struct init_params_t {
    uint32_t ipv4_main_table_id;
    uint32_t ipv4_num_hints;
    uint32_t ipv6_main_table_id;
    uint32_t ipv6_num_hints;
};

sdk_ret_t init(init_params_t *params);
sdk_ret_t insert(insert_params_t *params);

} // namespace ftlite

#endif // __FTLITE_HPP__
