//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// strcuture and definitions for rule_match_t data type.
// Use this library if you are dealing with types::RuleMatch
//-----------------------------------------------------------------------------
#ifndef __RULES_HPP__
#define __RULES_HPP__
#include "nic/include/base.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "addr_list.hpp"
#include "port_list.hpp"

using types::IPProtocol;
namespace hal {

typedef struct rule_match_app_s {
    dllist_ctxt_t    l4srcport_list; // list elems of type port_list_elem_t;
    dllist_ctxt_t    l4dstport_list; // list elems of type port_list_elem_t;
    dllist_ctxt_t    icmp_list;      // list elems of type icmp - TBD
    dllist_ctxt_t    rpc_list;       // list elems of type rpc/msrpc - TBD
    dllist_ctxt_t    esp_list;       // list elems of type Esp - TBD
} __PACK__ rule_match_app_t;

typedef struct rule_match_s {
    dllist_ctxt_t         src_addr_list;
    dllist_ctxt_t         dst_addr_list;
    dllist_ctxt_t         src_sg_list;
    dllist_ctxt_t         dst_sg_list;
    IPProtocol            proto;
    rule_match_app_t      app;
} __PACK__ rule_match_t;

#define RULE_MATCH_GET_ADDR(addr_entry)  \
    dllist_entry(addr_entry, addr_list_elem_t, list_ctxt)

#define RULE_MATCH_GET_PORT(port_entry)  \
    dllist_entry(port_entry, port_list_elem_t, list_ctxt)

// ----------------------------------------------------------------------------
// Function prototype
// ----------------------------------------------------------------------------
void rule_match_init(rule_match_t *match);
hal_ret_t rule_match_spec_extract(
    const types::RuleMatch& spec, rule_match_t *match);
void rule_match_cleanup(rule_match_t *match);



} // namespace hal

#endif //__RULES_HPP__



