//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Declarations for NAT Policy config hierarchy
//-----------------------------------------------------------------------------

#ifndef __NAT_POL_HPP__
#define __NAT_POL_HPP__

#include "sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

namespace hal {

typedef uint64_t rule_id_t;
typedef uint64_t pol_id_t;

//------------------------------------------------------------------------------
// NAT Policy (cfg) data structure layout
//
//    nat_policy <policy_key> {  policy_key = pol_id + vrf_id
//        nat_rule <rule_key> {  rule_key = rule_id
//
//            // match criteria for rule match
//            nat_rule_match {
//                list of src_addr range;
//                list of dst_addr range;
//
//                list of src-ports range;
//                list of dst-ports range;
//
//                list of src-security-group range;
//                list of dst-security-group range;
//            }
//
//            // action for rule match
//            nat_rule_action {
//                src-nat-action;
//                src-nat-pool-hal-handle;
//                dst-nat-action;
//                dst-nat-pool-hal-handle;
//            }
//        }
//    }
//
// NAT Policy (oper) data structure layout
//
//
//------------------------------------------------------------------------------

typedef struct nat_sg_range_s {
    uint32_t    sg_lo;
    uint32_t    sg_hi;
} nat_sg_range_t;

typedef struct nat_cfg_rule_action_s {
    nat::NatAction    src_nat_action;
    hal_handle        src_nat_pool;
    nat::NatAction    dst_nat_action;
    hal_handle        dst_nat_pool;
} nat_cfg_rule_action_t;

typedef struct nat_cfg_rule_match_s {
    dllist_ctxt_t    src_addr_list;
    dllist_ctxt_t    dst_addr_list;
    dllist_ctxt_t    src_port_list;
    dllist_ctxt_t    dst_port_list;
    dllist_ctxt_t    src_sg_list;
    dllist_ctxt_t    dst_sg_list;
} nat_cfg_rule_match_t;

typedef struct nat_cfg_rule_key_s {
    rule_id_t   rule_id;
} __PACK__ nat_cfg_rule_key_t;

typedef struct nat_cfg_rule_s {
    nat_cfg_rule_key_t       key;
    nat_cfg_rule_match_t     match;
    nat_cfg_rule_action_t    action;

    // operational
    ht_ctxt_t                ht_ctxt;
    dllist_ctxt_t            list_ctxt;
} nat_cfg_rule_t;

typedef struct nat_cfg_pol_key_s {
    pol_id_t    pol_id;
    vrf_id_t    vrf_id;
} __PACK__ nat_cfg_pol_key_t;

typedef struct nat_cfg_pol_s {
    nat_cfg_pol_key_t    key;
    dllist_ctxt_t        rule_list;
    dllist_ctxt_t        list_ctxt;
    hal_spinlock_t       slock;
    hal_handle_t         hdl;
    ht_ctxt_t            key_ctx;
    ht_ctxt_t            hdl_ctx;
} nat_cfg_pol_t;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
hal_ret_t nat_cfg_rule_spec_handle(
    const nat::NatRuleSpec& spec, dllist_ctxt_t *head);

}  // namespace hal

#endif    // __NAT_POL_HPP__
