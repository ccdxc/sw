//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::RuleMatch
//-----------------------------------------------------------------------------

#include "nic/include/hal_state.hpp"
#include "rule_match.hpp"
#include "utils.hpp"

namespace hal {

//-----------------------------------------------------------------------------
// No alloc or free functions for rule_match as it is user managed. 
// This allows the structure encompassing rule_match_t to be allocated 
// contiguously
//-----------------------------------------------------------------------------

static inline void
rule_match_app_init (rule_match_app_t *app)
{
    dllist_reset(&app->l4srcport_list);
    dllist_reset(&app->l4dstport_list);
    dllist_reset(&app->icmp_list);
    dllist_reset(&app->rpc_list);
    dllist_reset(&app->esp_list);
}

void
rule_match_init (rule_match_t *match)
{
    dllist_reset(&match->src_addr_list);
    dllist_reset(&match->dst_addr_list);
    dllist_reset(&match->src_sg_list);
    dllist_reset(&match->dst_sg_list);
    rule_match_app_init(&match->app);
}

static inline void
rule_match_app_cleanup (rule_match_t *match)
{
   rule_match_app_t *rule_match_app = &match->app;

   port_list_cleanup(&rule_match_app->l4srcport_list);
   port_list_cleanup(&rule_match_app->l4dstport_list);
   //TBD - cleanup icmp, rpc, esp
}

static inline void
rule_match_proto_cleanup (rule_match_t *match)
{
   match->proto = types::IPPROTO_NONE;
}

static inline void
rule_match_sg_cleanup (rule_match_t *match)
{
   return;
}

static inline void
rule_match_addr_cleanup (rule_match_t *match)
{
   addr_list_cleanup(&match->src_addr_list);
   addr_list_cleanup(&match->dst_addr_list);
}

void
rule_match_cleanup (rule_match_t *match)
{
    rule_match_addr_cleanup(match);
    rule_match_sg_cleanup(match);
    rule_match_proto_cleanup(match);
    rule_match_app_cleanup(match);
}

//-----------------------------------------------------------------------------
// Extraction routines
//-----------------------------------------------------------------------------

static inline hal_ret_t
rule_match_dst_port_spec_extract (
    const types::RuleMatch_L4PortAppInfo port_info, rule_match_app_t *app)
{
    hal_ret_t ret;

    for (int i = 0; i < port_info.dst_port_range_size(); i++) {
        if ((ret = port_list_elem_l4portrange_spec_handle(
                port_info.dst_port_range(i),
                &app->l4srcport_list)) != HAL_RET_OK)
            return ret;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_src_port_spec_extract (
    const types::RuleMatch_L4PortAppInfo port_info, rule_match_app_t *app)
{
    hal_ret_t ret;

    for (int i = 0; i < port_info.src_port_range_size(); i++) {
        if ((ret = port_list_elem_l4portrange_spec_handle(
                port_info.src_port_range(i),
                &app->l4srcport_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_port_app_spec_extract (const types::RuleMatch_AppMatchInfo  spec,
                                  rule_match_app_t *app)
{
    hal_ret_t ret;
    const types::RuleMatch_L4PortAppInfo port_info = spec.port_info(); 

    if ((ret = rule_match_src_port_spec_extract(port_info, app)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_port_spec_extract(port_info, app)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_app_spec_extract (const types::RuleMatch& spec,
                             rule_match_t *match)
{
    hal_ret_t   ret;

    // walk the apps
    for (int i = 0; i < spec.app_match_size(); i++) {
        const types::RuleMatch_AppMatchInfo app = spec.app_match(i);
        if (app.App_case() == types::RuleMatch_AppMatchInfo::kPortInfo) {
            ret = rule_match_port_app_spec_extract(app, &match->app);
            if (ret != HAL_RET_OK) {
                return ret;
            }
        } else if (app.App_case() == types::RuleMatch_AppMatchInfo::kIcmpInfo) {

        } else if (app.App_case() == types::RuleMatch_AppMatchInfo::kRpcInfo) {

        } else if (app.App_case() == types::RuleMatch_AppMatchInfo::kMsrpcInfo) {

        } else if (app.App_case() == types::RuleMatch_AppMatchInfo::kEspInfo) {

        } else {
            HAL_TRACE_ERR("App type not Handled {}", app.App_case());
            return HAL_RET_ERR;
        }
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_proto_spec_extract (const types::RuleMatch&  spec,
                               rule_match_t *match)
{
    match->proto = spec.protocol();
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_dst_sg_spec_extract (const types::RuleMatch& spec,
                                rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    for (int i = 0; i < spec.dst_sg_size(); i++) {
        //TBD: lseshan: Handle sg
        return ret;
    }

    return ret;
}

static inline hal_ret_t
rule_match_src_sg_spec_extract (const types::RuleMatch& spec,
                                rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    for (int i = 0; i < spec.src_sg_size(); i++) {
        //TBD: lseshan: Handle sg
        return ret;
    }

    return ret;
}

static inline hal_ret_t
rule_match_sg_spec_extract (const types::RuleMatch& spec,
                            rule_match_t *match)
{
    hal_ret_t ret;

    if ((ret = rule_match_src_sg_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_sg_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

static hal_ret_t
rule_match_dst_addr_spec_extract (const types::RuleMatch& spec,
                                  rule_match_t *match)
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
rule_match_src_addr_spec_extract (const types::RuleMatch& spec,
                                  rule_match_t *match)
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
rule_match_addr_spec_extract (const types::RuleMatch& spec,
                              rule_match_t *match)
{
    hal_ret_t ret;

    if ((ret = rule_match_src_addr_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_addr_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}


hal_ret_t
rule_match_spec_extract (const types::RuleMatch& spec,
                         rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_addr_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_sg_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_proto_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_app_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

end:
    if (ret != HAL_RET_OK)
        rule_match_cleanup(match);

    return ret;
}

} //end namespace hal
