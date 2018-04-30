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

using acl::acl_ctx_t;
using acl::acl_config_t;
using acl::ref_t;
using namespace acl;

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

//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

struct ipv4_tuple {
    uint32_t  ip_src;
    uint32_t  ip_dst;
    uint32_t  port_src;
    uint32_t  port_dst;
};

enum {
    IP_SRC = 0, IP_DST, PORT_SRC, PORT_DST, NUM_FIELDS 
};

#define ACL_FLD_DEF(_type, _struct_name, _fld_name) \
    {_type, sizeof(((_struct_name*)0)->_fld_name),  \
            offsetof(_struct_name, _fld_name) }

acl::acl_config_t nat_ip_acl_config_glbl = {
    num_categories: 1,
    num_fields: NUM_FIELDS,
    defs:  {  
        ACL_FLD_DEF(ACL_FIELD_TYPE_RANGE, ipv4_tuple, ip_src),
        ACL_FLD_DEF(ACL_FIELD_TYPE_RANGE, ipv4_tuple, ip_dst),
        ACL_FLD_DEF(ACL_FIELD_TYPE_RANGE, ipv4_tuple, port_src),
        ACL_FLD_DEF(ACL_FIELD_TYPE_RANGE, ipv4_tuple, port_dst),
    }
};

ACL_RULE_DEF(ipv4_rule_t, NUM_FIELDS);

static hal_ret_t
rule_lib_add (const acl_ctx_t **acl_ctx, ipv4_rule_t *rule)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = acl_add_rule((const acl_ctx_t **)acl_ctx,
           (const acl_rule_t *)rule)) != HAL_RET_OK)
        return ret;

    return ret;
}

static ipv4_rule_t *
rule_lib_alloc (void)
{
    ipv4_rule_t *rule = (ipv4_rule_t *)g_hal_state->ipv4_rule_slab()->alloc();
    rule->data.category_mask = 0x01;
    ref_init(&rule->ref_count, [] (const ref_t * ref_count) {
        ipv4_rule_t *rule  = (ipv4_rule_t *)acl_rule_from_ref(ref_count);
        ref_dec(&((nat_cfg_rule_t *)rule->data.userdata)->ref_count);
        g_hal_state->ipv4_rule_slab()->free(
            (void *)acl_rule_from_ref(ref_count)); });
    return rule;
}

hal_ret_t
rule_lib_handle (nat_cfg_rule_match_t *match, const acl_ctx_t **acl_ctx)
{
    int prio = 0;
    ipv4_rule_t *rule;
    hal_ret_t ret = HAL_RET_OK;
    addr_list_elem_t *src_addr, *dst_addr;
    port_list_elem_t *src_port, *dst_port;
    dllist_ctxt_t *sa_entry, *da_entry, *sp_entry, *dp_entry;

    dllist_for_each(sa_entry, &match->src_addr_list) {
        src_addr = dllist_entry(sa_entry, addr_list_elem_t, list_ctxt);

        dllist_for_each(da_entry, &match->dst_addr_list) {
            dst_addr = dllist_entry(da_entry, addr_list_elem_t, list_ctxt);

            dllist_for_each(sp_entry, &match->src_port_list) {
                src_port = dllist_entry(
                    sp_entry, port_list_elem_t, list_ctxt);

                dllist_for_each(dp_entry, &match->dst_port_list) {
                    dst_port = dllist_entry(
                        dp_entry, port_list_elem_t, list_ctxt);

                    rule  = rule_lib_alloc();

                    rule->field[IP_SRC].value.u32 =
                        src_addr->ip_range.vx_range[0].v4_range.ip_lo;
                    rule->field[IP_SRC].mask_range.u32 =
                        src_addr->ip_range.vx_range[0].v4_range.ip_hi;

                    rule->field[IP_DST].value.u32 =
                        dst_addr->ip_range.vx_range[0].v4_range.ip_lo;
                    rule->field[IP_DST].mask_range.u32 =
                        dst_addr->ip_range.vx_range[0].v4_range.ip_hi;

                    rule->field[PORT_SRC].value.u32 =
                        src_port->port_range.port_lo;
                    rule->field[PORT_DST].mask_range.u32 =
                        src_port->port_range.port_hi;

                    rule->field[PORT_DST].value.u32 =
                        dst_port->port_range.port_lo;
                    rule->field[PORT_DST].mask_range.u32 =
                        dst_port->port_range.port_hi;

                    rule->data.priority = prio++;
                    rule->data.userdata = (void *)match;

                    ret = rule_lib_add(acl_ctx, rule);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Unable to create the acl rules");
                        return ret;
                    }
                    //  << push it to the vector of ipv4_rule_t >>>
                }
            }
        }
    }
    return ret;
}

static inline const char *
nat_acl_ctx_name (vrf_id_t vrf_id)
{
    thread_local static char name[ACL_NAMESIZE];

    std::snprintf(name, sizeof(name), "nwsec-ipv4-rules:%lu", vrf_id);
    return name;
}

const acl::acl_ctx_t *
nat_cfg_pol_create_app_ctxt_init (nat_cfg_pol_t *pol)
{
    return (acl::acl_create(nat_acl_ctx_name(
               pol->key.vrf_id), &nat_ip_acl_config_glbl));
}

hal_ret_t
nat_cfg_rule_create_oper_handle (nat_cfg_rule_t *rule, const acl_ctx_t *acl_ctx)
{
    return rule_lib_handle(&rule->match, &acl_ctx);
}

} // namespace hal
