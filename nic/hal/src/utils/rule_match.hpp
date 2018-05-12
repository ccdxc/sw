//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// strcuture and definitions for rule_match_t data type.
// Use this library if you are dealing with types::RuleMatch
//-----------------------------------------------------------------------------
#ifndef __RULES_HPP__
#define __RULES_HPP__
#include "nic/include/base.h"
#include "nic/fte/acl/acl.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "addr_list.hpp"
#include "port_list.hpp"

using types::IPProtocol;
using acl::acl_rule_data_t;
using acl::acl_field_t;
using acl::acl_rule_t;
using acl::acl_field_type_t;
using acl::acl_ctx_t;
using acl::ref_t;
using acl::acl_config_t;
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
    dllist_ctxt_t         src_mac_addr_list;
    dllist_ctxt_t         dst_mac_addr_list;
    uint16_t              ethertype;
    IPProtocol            proto;
    rule_match_app_t      app;
} __PACK__ rule_match_t;

typedef void (*userdata_free)(void *);
typedef struct rule_data_s {
    void          *userdata;
    userdata_free data_free;
    acl::ref_t    ref_count;
} rule_data_t;

// ipv4_rules_t definition 
// into the library using acl_* calls
struct ipv4_tuple {
    uint8_t   proto;
    uint32_t  ip_src;
    uint32_t  ip_dst;
    uint64_t  mac_src;
    uint64_t  mac_dst;
    uint16_t  ethertype;
    uint32_t  port_src;
    uint32_t  port_dst;
    uint32_t  src_sg;
    uint32_t  dst_sg;
};

enum {
    PROTO = 0, IP_SRC, IP_DST, MAC_SRC, MAC_DST, ETHERTYPE, PORT_SRC, PORT_DST, SRC_SG, DST_SG,
    NUM_FIELDS
};

ACL_RULE_DEF(ipv4_rule_t, NUM_FIELDS);

#define RULE_FLD_DEF(typ, struct_name, fld_name)      \
    {typ, sizeof(((struct_name*)0)->fld_name),   \
            offsetof(struct_name, fld_name) }

#define RULE_MATCH_GET_MAC_ADDR(addr_entry)  \
    dllist_entry(addr_entry, mac_addr_list_elem_t, list_ctxt)

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

hal_ret_t rule_match_rule_add (const acl_ctx_t **acl_ctx,
                               rule_match_t     *match,
                               int              rule_prio,
                               rule_data_t      *data);
const acl_ctx_t *rule_lib_init(const char *name, acl_config_t *cfg);
hal_ret_t rule_match_spec_build(rule_match_t *match,
                                types::RuleMatch *spec);

rule_data_t *
rule_data_alloc_init();

} // namespace hal

#endif //__RULES_HPP__
