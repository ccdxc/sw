//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Declarations for NAT Policy config hierarchy
//-----------------------------------------------------------------------------

#ifndef __NAT_POL_HPP__
#define __NAT_POL_HPP__

#include "lib/ht/ht.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/rule_match.hpp"

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
    types::NatAction  src_nat_action;
    hal_handle_t      src_nat_pool;
    types::NatAction  dst_nat_action;
    hal_handle_t      dst_nat_pool;
} nat_cfg_rule_action_t;

typedef struct nat_cfg_rule_key_s {
    rule_id_t    rule_id;
} __PACK__ nat_cfg_rule_key_t;

typedef struct nat_cfg_rule_s {
    nat_cfg_rule_key_t       key;
    rule_match_t             match;
    nat_cfg_rule_action_t    action;

    // operational
    uint32_t                 prio;
    dllist_ctxt_t            list_ctxt;
    acl::ref_t               ref_count;
} nat_cfg_rule_t;

typedef struct nat_cfg_pol_key_s {
    pol_id_t    pol_id;
    vrf_id_t    vrf_id;
} __PACK__ nat_cfg_pol_key_t;

typedef struct nat_cfg_pol_create_app_ctxt_s {
    const acl::acl_ctx_t    *acl_ctx;
} __PACK__ nat_cfg_pol_create_app_ctxt_t;

typedef struct nat_cfg_pol_s {
    nat_cfg_pol_key_t    key;
    dllist_ctxt_t        rule_list;

    // operational
    sdk_spinlock_t       slock;
    hal_handle_t         hal_hdl;
} nat_cfg_pol_t;

//-----------------------------------------------------------------------------
// Inline functions
//-----------------------------------------------------------------------------

inline void *
nat_cfg_pol_key_func_get (void *entry)
{
    nat_cfg_pol_t *pol = NULL;
    hal_handle_id_ht_entry_t *ht_entry;

    SDK_ASSERT(entry != NULL);
    if ((ht_entry = (hal_handle_id_ht_entry_t *)entry) == NULL)
        return NULL;

    pol = (nat_cfg_pol_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(pol->key);
}

inline uint32_t
nat_cfg_pol_key_size ()
{
    return sizeof(nat_cfg_pol_key_t);
}

inline void
nat_cfg_pol_create_rsp_build (nat::NatPolicyResponse *rsp, hal_ret_t ret,
                              hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK)
        rsp->mutable_policy_status()->set_nat_policy_handle(hal_handle);
    rsp->set_api_status(hal_prepare_rsp(ret));
}

inline void
nat_cfg_pol_delete_rsp_build (nat::NatPolicyDeleteResponse *rsp, hal_ret_t ret)
{
    rsp->set_api_status(hal_prepare_rsp(ret));
}

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

// pol.cc
void nat_cfg_pol_dump(nat::NatPolicySpec& spec);
hal_ret_t nat_cfg_pol_create_cfg_handle(
    nat::NatPolicySpec& spec, nat_cfg_pol_t **out_pol);
hal_ret_t nat_cfg_pol_create_oper_handle(nat_cfg_pol_t *pol);
nat_cfg_pol_t * nat_cfg_pol_key_or_handle_lookup(
    const kh::NatPolicyKeyHandle& kh);
hal_ret_t nat_cfg_pol_get_cfg_handle(
    nat::NatPolicyGetRequest& req, nat::NatPolicyGetResponseMsg *rsp);
hal_ret_t nat_cfg_pol_delete_cfg_handle(nat_cfg_pol_t *pol);
hal_ret_t nat_cfg_pol_delete_oper_handle(nat_cfg_pol_t *pol);

// rule.cc
hal_ret_t nat_cfg_rule_spec_handle(
    const nat::NatRuleSpec& spec, dllist_ctxt_t *head);
hal_ret_t nat_cfg_rule_acl_build(
    nat_cfg_rule_t *rule, const acl_ctx_t **acl_ctx);
void nat_cfg_rule_acl_cleanup(nat_cfg_rule_t *rule);
hal_ret_t nat_cfg_rule_spec_build(nat_cfg_rule_t *rule, nat::NatRuleSpec *spec);
void nat_cfg_rule_list_cleanup(dllist_ctxt_t *head);

}  // namespace hal

#endif    // __NAT_POL_HPP__
