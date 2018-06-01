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
#include "sg_list.hpp"

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

#define RULE_MATCH_GET_SG(sg_entry)  \
    dllist_entry(sg_entry, sg_list_elem_t, list_ctxt)

#define PRINT_RULE_FIELDS(rule)  \
            HAL_TRACE_DEBUG("Added rule with keys: src {}, src_range {}, dst {}, dst_range {}, mac_src {}, mac_src_range {}, mac_dst {}, mac_dst_range {}, port_src {}, port_src_mask {}, port_dst {}, port_dst_mask {}, src_sg {}, src_sg_mask {}, dst_sg {}, dst_sg_mask {} proto {}, proto_mask {}, ethertype {} ethertype_mask {}", \
                        rule->field[IP_SRC].value.u32,rule->field[IP_SRC].mask_range.u32,                                            \
                        rule->field[IP_DST].value.u32,rule->field[IP_DST].mask_range.u32,                                            \
                        rule->field[MAC_SRC].value.u32,rule->field[MAC_SRC].mask_range.u32,                                          \
                        rule->field[MAC_DST].value.u32,rule->field[MAC_DST].mask_range.u32,                                          \
                        rule->field[PORT_SRC].value.u32,rule->field[PORT_SRC].mask_range.u32,                                        \
                        rule->field[PORT_DST].value.u32,rule->field[PORT_DST].mask_range.u32,                                        \
                        rule->field[SRC_SG].value.u32,rule->field[SRC_SG].mask_range.u32,                                            \
                        rule->field[DST_SG].value.u32,rule->field[DST_SG].mask_range.u32,                                            \
                        rule->field[PROTO].value.u8,rule->field[PROTO].mask_range.u8,                                                \
                        rule->field[ETHERTYPE].value.u16, rule->field[ETHERTYPE].value.u16)

static inline rule_data_t *
rule_data_from_ref(const ref_t *ref_count) {
    return container_of(ref_count, rule_data_t, ref_count);
}
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
