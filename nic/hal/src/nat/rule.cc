//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for nat::NatRuleSpec object
//-----------------------------------------------------------------------------

#include "nat.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/utils/addr_list.hpp"
#include "nic/hal/src/utils/port_list.hpp"
#include "nic/hal/src/utils/cfg_op_ctxt.hpp"
#include "nic/hal/src/utils/rule_match.hpp"

using acl::acl_ctx_t;
using acl::acl_config_t;
using acl::ref_t;
using namespace acl;
using kh::NatPoolKeyHandle;

namespace hal {

//-----------------------------------------------------------------------------
// Rule action routines
//-----------------------------------------------------------------------------

static hal_ret_t
nat_cfg_rule_action_spec_extract (const nat::NatRuleAction& spec,
                                  nat_cfg_rule_action_t *action)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_pool_t *pool;

    action->src_nat_action = spec.src_nat_action();
    action->dst_nat_action = spec.dst_nat_action();

    if (spec.has_src_nat_pool()) {
        pool = find_nat_pool_by_key_or_handle(spec.src_nat_pool());
        if (pool != NULL) {
            action->src_nat_pool = pool->hal_handle;
        } else {
            HAL_TRACE_ERR("src NAT pool not present in rule");
            ret = HAL_RET_NAT_POOL_NOT_FOUND;
            return ret;
        }
    }

    if (spec.has_dst_nat_pool()) {
        pool = find_nat_pool_by_key_or_handle(spec.dst_nat_pool());
        if (pool != NULL) {
            action->dst_nat_pool = pool->hal_handle;
        } else {
            HAL_TRACE_ERR("dst NAT pool not present in rule");
            ret = HAL_RET_NAT_POOL_NOT_FOUND;
            return ret;
        }
    }

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

void
nat_cfg_rule_free (void *rule)
{
    hal::delay_delete_to_slab(HAL_SLAB_NAT_CFG_RULE, (nat_cfg_rule_t *)rule);
}

static inline void
nat_cfg_rule_init (nat_cfg_rule_t *rule)
{
    rule_match_init(&rule->match);
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

    if ((ret = rule_match_spec_extract(
           spec.match(), &rule->match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_action_spec_extract(
           spec.action(), &rule->action)) != HAL_RET_OK)
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

hal_ret_t
nat_cfg_rule_spec_build (nat_cfg_rule_t *rule, nat::NatRuleSpec *spec)
{
    hal_ret_t   ret;

    spec->set_rule_id(rule->key.rule_id);

    auto action = spec->mutable_action();
    action->set_src_nat_action(rule->action.src_nat_action);
    action->set_dst_nat_action(rule->action.dst_nat_action);
    // TODO Set src and dest NAT Pool

    if ((ret = rule_match_spec_build(
           &rule->match, spec->mutable_match())) != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

acl_config_t nat_ip_acl_config_glbl;

const char *
nat_acl_ctx_name (vrf_id_t vrf_id)
{
    thread_local static char name[ACL_NAMESIZE];

    std::snprintf(name, sizeof(name), "nat-ipv4-rules:%lu", vrf_id);
    return name;
}

const acl::acl_ctx_t *
nat_cfg_pol_create_app_ctxt_init (nat_cfg_pol_t *pol)
{
    return (rule_lib_init(nat_acl_ctx_name(
               pol->key.vrf_id), &nat_ip_acl_config_glbl));
}

hal_ret_t
nat_cfg_rule_create_oper_handle (nat_cfg_rule_t *rule, const acl_ctx_t **acl_ctx)
{
    rule_data_t     *rule_data;
    rule_data = rule_data_alloc_init();
    rule_data->userdata = rule;
    rule_data->data_free = nat_cfg_rule_free;
    return rule_match_rule_add(acl_ctx,
                               &rule->match,
                               rule->prio,
                               rule_data);
}



} // namespace hal
