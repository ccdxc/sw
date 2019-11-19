//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// strcuture and definitions for rule_match_t data type.
// Use this library if you are dealing with types::RuleMatch
//-----------------------------------------------------------------------------
#ifndef __RULES_HPP__
#define __RULES_HPP__
#include "nic/include/base.hpp"
#include "nic/fte/acl/acl.hpp"
#include "gen/proto/types.pb.h"
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
enum AppType {
    APP_NONE,
    APP_TCP_UDP,
    APP_ESP,
    APP_ICMP,
    APP_RPC
};

typedef struct rule_match_icmp_s {
    uint32_t    icmp_type;
    uint32_t    icmp_code;
} __PACK__ rule_match_icmp_t;

typedef struct rule_match_app_s {
    AppType            app_type;
    dllist_ctxt_t      l4srcport_list; // list elems of type port_list_elem_t;
    dllist_ctxt_t      l4dstport_list; // list elems of type port_list_elem_t;
    dllist_ctxt_t      rpc_list;       // list elems of type rpc/msrpc - TBD
    uint32_t           esp_spi;       // list elems of type Esp - TBD
    rule_match_icmp_t  icmp;
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

// ipv4_rules_t definition 
// into the library using acl_* calls
struct ipv4_tuple {
    uint8_t   proto;
    uint32_t  ip_src;
    uint32_t  ip_dst;
    uint32_t  mac_src;
    uint32_t  mac_dst;
    uint16_t  ethertype;
    uint32_t  port_src;
    uint32_t  port_dst;
    uint32_t  src_sg;
    uint32_t  dst_sg;
    uint32_t  icmp_type;
    uint32_t  icmp_code;
};

#define PRINT_RULE_KEY(key) \
    HAL_TRACE_DEBUG("key to acl: key->proto: {}, key->ip_src: {}, key->ip_dst: {}, key->mac_src: {}, key->mac_dst: {}, key->ethertype: {}," \
    "key->port_src: {}, key->port_dst: {}, key->icmp_type: {}, key->icmp_code: {}", key->proto, key->ip_src, key->ip_dst, key->mac_src, key->mac_dst, key->key_ethertype, key->port_src, key->port_dst, key->icmp_type, key->icmp_code);
    
#define INVALID_TCP_UDP_PORT        65536
#define IS_ANY_PORT(lo, hi)                 ((lo == INVALID_TCP_UDP_PORT) && (hi == INVALID_TCP_UDP_PORT))

enum {
    PROTO = 0, IP_SRC, IP_DST, MAC_SRC, MAC_DST, ETHERTYPE, PORT_SRC, PORT_DST, SRC_SG, DST_SG, ICMP_TYPE, ICMP_CODE,
    NUM_FIELDS
};

#define RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(head, nnode)     \
    if (dllist_empty(head)) {  \
        dllist_add(head, nnode); \
    }

#define RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(nnode)     \
    if (!dllist_empty(nnode)) {  \
        dllist_del(nnode); \
    }

ACL_RULE_DEF(ipv4_rule_t, NUM_FIELDS);

typedef uint64_t rule_key_t;
struct rule_ctr_t;

typedef hal_ret_t (*rule_ctr_cb_t)(rule_ctr_t *ctr, bool add);

typedef struct rule_lib_cb_s {
    rule_ctr_cb_t     rule_ctr_cb;
} __PACK__ rule_lib_cb_t;

typedef struct rule_cfg_s {
    char                name[64];
    const acl_ctx_t     *acl_ctx;
    acl_config_t        acl_cfg;
    ht_ctxt_t           ht_ctxt;
    ht                 *rule_ctr_ht; // Hash table for counters keyed by: rule key
    rule_ctr_cb_t       rule_ctr_cb;
    acl::ref_t          ref_count;
} rule_cfg_t;

typedef struct rule_data_s {
    void       *user_data;
    void       *ctr;
    acl::ref_t ref_cnt;
} rule_data_t;

typedef struct rule_ctr_data_s {
    uint64_t    tcp_hits;
    uint64_t    udp_hits;
    uint64_t    icmp_hits;
    uint64_t    esp_hits;
    uint64_t    other_hits;
    uint64_t    total_hits;
} __PACK__ rule_ctr_data_t;

struct rule_ctr_t {
    rule_ctr_data_t *rule_stats; 
    uint32_t         stats_idx;
    rule_key_t       rule_key;
    ht_ctxt_t        ht_ctxt;
    rule_cfg_t      *rule_cfg;
    rule_data_t     *rule_data;
    acl::ref_t       ref_count;
};

//------------------------------------------------------------------------------
// The user data struct is defined as below. The ref_count MUST be the last
// field in the user data struct: 
// typedef struct user_data_s {
//    < user_data_struct field1>
//    < user_data_struct field2>
//    ....
//    acl::ref_t    ref_count;
// } user_data_t;
//
// To get the user data back the following macro can be used:
// acl::ref_t *rc = rule->data.userdata; <-- ref_count ptr returned after acl classify
// user_data_ptr = RULE_MATCH_USER_DATA(rc, user_data_t, ref_count);
//------------------------------------------------------------------------------
#define RULE_MATCH_USER_DATA(ptr, type, member)                 \
    ((type *)((char *)(ptr) - offsetof(type, member)))

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
            HAL_TRACE_VERBOSE("rule with keys: src {}, src_range {}, dst {}, dst_range {}, mac_src {}, mac_src_range {}, mac_dst {}, mac_dst_range {}, port_src {}, port_src_mask {}, port_dst {}, port_dst_mask {}, src_sg {}, src_sg_mask {}, dst_sg {}, dst_sg_mask {} proto {}, proto_mask {}, ethertype {} ethertype_mask {} icmp_type {} icmp_code {}", \
                        rule->field[IP_SRC].value.u32,rule->field[IP_SRC].mask_range.u32,                                            \
                        rule->field[IP_DST].value.u32,rule->field[IP_DST].mask_range.u32,                                            \
                        rule->field[MAC_SRC].value.u32,rule->field[MAC_SRC].mask_range.u32,                                          \
                        rule->field[MAC_DST].value.u32,rule->field[MAC_DST].mask_range.u32,                                          \
                        rule->field[PORT_SRC].value.u32,rule->field[PORT_SRC].mask_range.u32,                                        \
                        rule->field[PORT_DST].value.u32,rule->field[PORT_DST].mask_range.u32,                                        \
                        rule->field[SRC_SG].value.u32,rule->field[SRC_SG].mask_range.u32,                                            \
                        rule->field[DST_SG].value.u32,rule->field[DST_SG].mask_range.u32,                                            \
                        rule->field[PROTO].value.u8,rule->field[PROTO].mask_range.u8,                                                \
                        rule->field[ETHERTYPE].value.u16, rule->field[ETHERTYPE].value.u16,                                         \
                        rule->field[ICMP_TYPE].value.u32, rule->field[ICMP_CODE].value.u32)

// ----------------------------------------------------------------------------
// Function prototype
// ----------------------------------------------------------------------------
void rule_match_init(rule_match_t *match);
hal_ret_t rule_match_spec_extract(
    const types::RuleMatch& spec, rule_match_t *match);
void rule_match_cleanup(rule_match_t *match);

hal_ret_t rule_match_rule_add (const acl_ctx_t **acl_ctx,
                               rule_match_t     *match,
                               rule_key_t       rule_key,
                               int              rule_prio,
                               void             *ref_count);
hal_ret_t rule_match_rule_del (const acl_ctx_t **acl_ctx,
                               rule_match_t     *match,
                               rule_key_t       rule_key,
                               int              rule_prio,
                               void             *ref_count);
const acl_ctx_t *rule_lib_init(const char *name, acl_config_t *cfg, rule_lib_cb_t *rule_cb=NULL);
hal_ret_t rule_match_spec_build(rule_match_t *match,
                                types::RuleMatch *spec);
void *rule_cfg_get_key_func(void *entry);
uint32_t rule_cfg_key_size(void);
void rule_lib_delete(const char *name);
rule_cfg_t *
rule_cfg_get(const char *name);
rule_ctr_t *
rule_ctr_get(rule_cfg_t *cfg, rule_key_t key);

acl::ref_t *
get_rule_data(acl_rule_t *rule);
rule_ctr_t *
get_rule_ctr(acl_rule_t *rule);



} // namespace hal

#endif //__RULES_HPP__
