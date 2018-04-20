//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles Config APIs for NAT Rules
//-----------------------------------------------------------------------------

#include "nat.hpp"

namespace hal {

static inline nat_cfg_rule_t *
nat_cfg_rule_alloc (void)
{
    return ((nat_cfg_rule_t *)g_hal_state->nat_cfg_rule_slab()->alloc());
}

static inline void
nat_cfg_rule_init (nat_cfg_rule_t *rule)
{
    return;
}

static hal_ret_t
nat_cfg_rule_action_spec_extract (nat::NatRuleSpec& spec,
                                  nat_cfg_rule_action_t *action)
{
    hal_ret_t ret = HAL_RET_OK;

    action->src_nat_action = spec.src_nat_action();
    action->dst_nat_action = spec.dst_nat_action();
    // TODO - check for pools
    // action->src_nat_pool = spec.src_nat_pool();
    // action->dst_nat_pool = spec.src_nat_pool();

    return ret;
}

static hal_ret_t
nat_cfg_rule_match_dst_sg_spec_extract (nat::NatRuleSpec& spec,
                                        nat_cfg_rule_match_t *match)
{
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_rule_match_src_sg_spec_extract (nat::NatRuleSpec& spec,
                                        nat_cfg_rule_match_t *match)
{
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_rule_match_sg_spec_extract (nat::NatRuleSpec& spec,
                                    nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_rule_match_src_sg_spec_extract(
           spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_dst_sg_spec_extract(
           spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_rule_match_spec_extract (nat::NatRuleSpec& spec,
                                 nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_rule_match_addr_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_port_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_sg_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_rule_data_spec_extract (nat::NatRuleSpec& spec, nat_cfg_rule_t *rule)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_rule_match_spec_extract(
           spec, &rule->match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_action_spec_extract(
           spec, &rule->action)) != HAL_RET_OK)
        return ret;

    return ret;
}

static inline hal_ret_t
nat_cfg_rule_key_spec_extract (nat::NatRuleSpec& spec, nat_cfg_rule_key_t *key)
{
    key->rule_id = spec.rule_id();
    return HAL_RET_OK;
}

hal_ret_t
nat_cfg_rule_spec_extract (nat::NatRuleSpec& spec, nat_cfg_rule_t *rule)
{
    hal_ret_t ret;
    
    if ((ret = nat_cfg_rule_key_spec_extract(spec, &rule->key)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_data_spec_extract(spec, rule)) != HAL_RET_OK)
        return ret;

    return ret;
}

} // namespace hal
