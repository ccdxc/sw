//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for nat::NatRuleSpec object
//-----------------------------------------------------------------------------

#include "nat.hpp"
#include "nic/hal/src/utils/addr_list.hpp"
#include "nic/hal/src/utils/port_list.hpp"

namespace hal {

//-----------------------------------------------------------------------------
// Rule action routines
//-----------------------------------------------------------------------------

static hal_ret_t
nat_cfg_rule_action_spec_extract (const nat::NatRuleSpec& spec,
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

//-----------------------------------------------------------------------------
// Rule match routines
//-----------------------------------------------------------------------------

static inline void
nat_cfg_rule_match_init (nat_cfg_rule_match_t *match)
{
    dllist_reset(&match->src_addr_list);
    dllist_reset(&match->dst_addr_list);
    dllist_reset(&match->src_port_list);
    dllist_reset(&match->dst_port_list);
    dllist_reset(&match->src_sg_list);
    dllist_reset(&match->dst_sg_list);
}

static hal_ret_t
nat_cfg_rule_match_dst_port_spec_handle (const nat::NatRuleSpec& spec,
                                         nat_cfg_rule_match_t *match)
{
    hal_ret_t ret;

    for (int i = 0; i < spec.dst_port_range_size(); i++) {
        if ((ret = port_list_elem_l4portrange_spec_handle(
                spec.dst_port_range(i), &match->dst_port_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_rule_match_src_port_spec_handle (const nat::NatRuleSpec& spec,
                                         nat_cfg_rule_match_t *match)
{
    hal_ret_t ret;

    for (int i = 0; i < spec.src_port_range_size(); i++) {
        if ((ret = port_list_elem_l4portrange_spec_handle(
                spec.src_port_range(i), &match->src_port_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_rule_match_port_spec_extract (const nat::NatRuleSpec& spec,
                                       nat_cfg_rule_match_t *match)
{
    hal_ret_t ret;

    if ((ret = nat_cfg_rule_match_src_port_spec_handle(
            spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_dst_port_spec_handle(
            spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_rule_match_dst_addr_spec_handle (const nat::NatRuleSpec& spec,
                                         nat_cfg_rule_match_t *match)
{
    hal_ret_t ret;

    for (int i = 0; i < spec.dst_address_size(); i++) {
        if ((ret = addr_list_elem_ipaddressobj_spec_handle(
                spec.dst_address(i), &match->dst_addr_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_rule_match_src_addr_spec_handle (const nat::NatRuleSpec& spec,
                                         nat_cfg_rule_match_t *match)
{
    hal_ret_t ret;

    for (int i = 0; i < spec.src_address_size(); i++) {
        if ((ret = addr_list_elem_ipaddressobj_spec_handle(
                spec.src_address(i), &match->src_addr_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_rule_match_addr_spec_extract (const nat::NatRuleSpec& spec,
                                      nat_cfg_rule_match_t *match)
{
    hal_ret_t ret;

    if ((ret = nat_cfg_rule_match_src_addr_spec_handle(
            spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_dst_addr_spec_handle(
            spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_rule_match_spec_extract (const nat::NatRuleSpec& spec,
                                 nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_rule_match_addr_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_port_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

//-----------------------------------------------------------------------------
// Rule routines
//-----------------------------------------------------------------------------

static inline nat_cfg_rule_t *
nat_cfg_rule_alloc (void)
{
    return ((nat_cfg_rule_t *)g_hal_state->nat_cfg_rule_slab()->alloc());
}

static inline void
nat_cfg_rule_free (nat_cfg_rule_t *rule)
{
    hal::delay_delete_to_slab(HAL_SLAB_NAT_CFG_RULE, rule);
}

static inline void
nat_cfg_rule_init (nat_cfg_rule_t *rule)
{
    nat_cfg_rule_match_init(&rule->match);
    dllist_reset(&rule->list_ctxt);
}

static inline void
nat_cfg_rule_uninit (nat_cfg_rule_t *rule)
{
    return;
}

static inline nat_cfg_rule_t *
nat_cfg_rule_alloc_init (void)
{
    nat_cfg_rule_t *rule;

    if ((rule = nat_cfg_rule_alloc()) ==  NULL)
        return NULL;

    nat_cfg_rule_init(rule);
    return rule;
}

static inline void
nat_cfg_rule_uninit_free (nat_cfg_rule_t *rule)
{
    if (rule) {
        nat_cfg_rule_uninit(rule);
        nat_cfg_rule_free(rule);
    }
}

static inline void
nat_cfg_rule_db_add (dllist_ctxt_t *head, nat_cfg_rule_t *rule)
{
    dllist_add_tail(head, &rule->list_ctxt);
}

static inline void
nat_cfg_rule_db_del (nat_cfg_rule_t *rule)
{
    dllist_del(&rule->list_ctxt);
}

static hal_ret_t
nat_cfg_rule_data_spec_extract (const nat::NatRuleSpec& spec,
                                nat_cfg_rule_t *rule)
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
nat_cfg_rule_key_spec_extract (const nat::NatRuleSpec& spec,
                               nat_cfg_rule_key_t *key)
{
    key->rule_id = spec.rule_id();
    return HAL_RET_OK;
}

static inline hal_ret_t
nat_cfg_rule_spec_extract (const nat::NatRuleSpec& spec, nat_cfg_rule_t *rule)
{
    hal_ret_t ret;

    if ((ret = nat_cfg_rule_key_spec_extract(
           spec, &rule->key)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_data_spec_extract(
           spec, rule)) != HAL_RET_OK)
        return ret;

   return ret;
}

hal_ret_t
nat_cfg_rule_spec_handle (const nat::NatRuleSpec& spec, dllist_ctxt_t *head)
{
    hal_ret_t ret;
    nat_cfg_rule_t *rule;

    if ((rule = nat_cfg_rule_alloc_init()) == NULL)
        return HAL_RET_OOM;

    if ((ret = nat_cfg_rule_spec_extract(spec, rule)) != HAL_RET_OK)
        return ret;

    nat_cfg_rule_db_add(head, rule);
    return ret;
}

} // namespace hal
