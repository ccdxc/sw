//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for NAT feature
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nat/nat.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/fte/acl/acl.hpp"

using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

namespace hal {

#if 0
//------------------------------------------------------------------------------
// ACL rule matching data structure is used to save the NAT rules config
// The ACL rule is presumably faster than a linear search
//------------------------------------------------------------------------------
static hal_ret_t
nat_rule_spec_validate (nat::NatRuleSpec& spec, nat::NatRuleResponse *rsp)
{
    return HAL_RET_OK;
}

static nat_rule_t *
nat_rule_alloc (void)
{
    nat_rule_t *nat_rule;

    nat_rule = (nat_rule_t *) g_hal_state->nat_rule_slab()->alloc();
    return nat_rule;
}

static hal_ret_t
nat_rule_spec_key_extract (nat::NatRuleSpec& spec, nat_rule_key_t *rule_key)
{
    rule_key->rule_id = spec.key_or_handle().rule_id();
    rule_key->vrf_id = spec.key_or_handle().vrf_id();
        nwsec_policy->key.vrf_id = spec.policy_key_or_handle().security_policy_key().vrf_id_or_handle().vrf_id();
}

static hal_ret_t
nat_rule_spec_extract (nat::NatRuleSpec& spec, nat_rule_t *rule)
{
    nat_rule_spec_key_extract(spec, &rule->key);
    return HAL_RET_OK;
}

static hal_ret_t
nat_rule_db_add (nat_rule_t *rule)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_rule_create (nat::NatRuleSpec& spec, nat::NatRuleResponse *rsp)
{
    hal_ret_t     ret;
    nat_rule_t    *rule;

    if ((ret = nat_rule_spec_validate(spec, rsp)) != HAL_RET_OK)
        goto end;

    if ((rule = nat_rule_alloc()) == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    if ((ret = nat_rule_spec_extract(spec, rule)) != HAL_RET_OK)
        goto end;

    if ((ret = nat_rule_db_add(rule)) != HAL_RET_OK)
        goto end;

end:

    return HAL_RET_OK;

}

//------------------------------------------------------------------------------
// process a nat rule update request
//------------------------------------------------------------------------------
hal_ret_t
nat_rule_update (NatRuleSpec& spec, NatRuleResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a nat rule get request
//------------------------------------------------------------------------------
hal_ret_t
nat_rule_get (NatRuleGetRequest& req, NatRuleGetResponseMsg *rsp)
{
    return HAL_RET_OK;
}
#endif

hal_ret_t
nat_policy_create (NatPolicySpec& spec, NatPolicyResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_policy_update (NatPolicySpec& spec, NatPolicyResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_policy_delete (NatPolicyDeleteRequest& req,
                   NatPolicyDeleteResponse *res)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_policy_get (NatPolicyGetRequest& req, NatPolicyGetResponseMsg *res)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_vpn_mapping_create (NatVpnMappingSpec& spec,
                        NatVpnMappingResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_vpn_mapping_get (NatVpnMappingGetRequest& req,
                     NatVpnMappingGetResponseMsg *rsp)
{
    return HAL_RET_OK;
}

}    // namespace hal
