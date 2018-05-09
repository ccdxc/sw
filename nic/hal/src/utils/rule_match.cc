//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::RuleMatch
//-----------------------------------------------------------------------------

#include "nic/include/hal_state.hpp"
#include "rule_match.hpp"
#include "utils.hpp"

namespace hal {

acl::acl_config_t ip_acl_config_glbl = {
    num_categories: 1,
    num_fields: NUM_FIELDS,
    defs:  {    RULE_FLD_DEF(acl::ACL_FIELD_TYPE_EXACT, ipv4_tuple, proto),

                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_RANGE, ipv4_tuple, ip_src),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_RANGE, ipv4_tuple, ip_dst),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_RANGE, ipv4_tuple, port_src),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_RANGE, ipv4_tuple, port_dst),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_RANGE, ipv4_tuple, src_sg),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_RANGE, ipv4_tuple, dst_sg),
           }
};

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
                &app->l4dstport_list)) != HAL_RET_OK)
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

// rule_lib_alloc function allocates data type ipv4_rule_t to be added to
// acl_lib
rule_data_t *
rule_data_alloc_init()
{
    rule_data_t *data = (rule_data_t *)g_hal_state->rule_data_slab()->alloc();
    ref_init(&data->ref_count, [] (const ref_t * ref_count) {
        rule_data_t *rule  = (rule_data_t *)acl_rule_from_ref(ref_count);
        rule->data_free(rule->userdata);
        g_hal_state->rule_data_slab()->free((void *)acl_rule_from_ref(ref_count));
    });
    return data;
}

void
rule_lib_init_config(acl_config_t *cfg)
{
    memcpy(cfg, &ip_acl_config_glbl, sizeof(acl_config_t));
    return;
}

const acl_ctx_t *
rule_lib_init(const char *name, acl_config_t *cfg)
{
    rule_lib_init_config(cfg);
    return acl_create(name, (const acl_config_t *)cfg);
}


// rule_lib_alloc function allocates data type ipv4_rule_t to be added to
// acl_lib
ipv4_rule_t *
rule_lib_alloc()
{
    ipv4_rule_t *rule = (ipv4_rule_t *)g_hal_state->ipv4_rule_slab()->alloc();
    rule->data.category_mask = 0x01;
    ref_init(&rule->ref_count, [] (const ref_t * ref_count) {

        ipv4_rule_t *rule  = (ipv4_rule_t *)acl_rule_from_ref(ref_count);
        ref_dec(&((ipv4_rule_t *)rule->data.userdata)->ref_count);
        g_hal_state->ipv4_rule_slab()->free((void *)acl_rule_from_ref(ref_count));
    });
    return rule;
}

// rule_match_rule_add api adds the rules to the acl library.
// As of today type of fields that are instantiated in acl_lib
// (pkt_classify_lib) will be of type ipv4_rule_t across plugins that use
// types.RuleMatch 
// 
hal_ret_t
rule_match_rule_add (const acl_ctx_t **acl_ctx,
                     rule_match_t     *match,
                     int               rule_prio,
                     rule_data_t      *data)
{
    ipv4_rule_t       *rule;
    rule_match_app_t  *app_match = &match->app;
    hal_ret_t         ret = HAL_RET_OK;
    addr_list_elem_t  *src_addr, *dst_addr;
    port_list_elem_t  *dst_port, *src_port;
    dllist_ctxt_t     *sa_entry, *da_entry, *dp_entry, *sp_entry;
    int               src_addr_len = 1, dst_addr_len = 1, dst_port_len = 1, src_port_len = 1;


    if (!dllist_empty(&match->src_addr_list)) {
        src_addr_len = dllist_count(&match->src_addr_list);
    }

    if (!dllist_empty(&match->dst_addr_list)) {
        dst_addr_len = dllist_count(&match->dst_addr_list);
    }

    if (!dllist_empty(&app_match->l4dstport_list)) {
        dst_port_len = dllist_count(&app_match->l4dstport_list);
    }

    if (!dllist_empty(&app_match->l4srcport_list)) {
        src_port_len = dllist_count(&app_match->l4srcport_list);
    }

    sa_entry = match->src_addr_list.next;
    da_entry = match->dst_addr_list.next;
    dp_entry = app_match->l4dstport_list.next;
    sp_entry = app_match->l4srcport_list.next;


    // ToDo: lseshan: Explore and Make below loop efficient
    for (int src_addr_idx =0; src_addr_idx < src_addr_len; src_addr_idx++) {
        src_addr = RULE_MATCH_GET_ADDR(sa_entry);
        sa_entry = sa_entry->next;
        for (int dst_addr_idx = 0; dst_addr_idx < dst_addr_len; dst_addr_idx++) {
            dst_addr = RULE_MATCH_GET_ADDR(da_entry);
            da_entry = da_entry->next;
            for (int src_port_idx = 0; src_port_idx < src_port_len; src_port_idx++) {
                src_port = RULE_MATCH_GET_PORT(sp_entry);
                sp_entry = sp_entry->next;
                for (int dst_port_idx = 0; dst_port_idx < dst_port_len; dst_port_idx++) {
                    dst_port = RULE_MATCH_GET_PORT(dp_entry);
                    dp_entry = dp_entry->next;

                    rule  = rule_lib_alloc();
                    rule->field[PROTO].value.u32 = 0;
                    if (!dllist_empty(&match->src_addr_list)) {
                        rule->field[IP_SRC].value.u32 = src_addr->ip_range.vx_range[0].v4_range.ip_lo;
                        rule->field[IP_SRC].mask_range.u32 = src_addr->ip_range.vx_range[0].v4_range.ip_hi;
                    }
                    if (!dllist_empty(&match->dst_addr_list)) {
                        rule->field[IP_DST].value.u32 = dst_addr->ip_range.vx_range[0].v4_range.ip_lo;
                        rule->field[IP_DST].mask_range.u32 = dst_addr->ip_range.vx_range[0].v4_range.ip_hi;
                    }
                    if (!dllist_empty(&app_match->l4srcport_list)) {
                        rule->field[PORT_SRC].value.u32 = src_port->port_range.port_lo;
                        rule->field[PORT_SRC].mask_range.u32 = src_port->port_range.port_hi;
                    }
                    if (!dllist_empty(&app_match->l4dstport_list)) {
                        rule->field[PORT_DST].value.u32 = dst_port->port_range.port_lo;
                        rule->field[PORT_DST].mask_range.u32 = dst_port->port_range.port_hi;
                    }
                    rule->data.priority = rule_prio;
                    rule->data.userdata = (void *)data;
                    ret = acl_add_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Unable to create the acl rules");
                        return ret;
                    }
                }//  << push it to the vector of ipv4_rule_t >>>
            }
        }
    }
    //Added rule - lets increment the ref
    ref_inc(&data->ref_count);
    return ret;
}

//-----------------------------------------------------------------------------
// Rule Match Spec Build routines
//-----------------------------------------------------------------------------

static inline hal_ret_t
rule_match_dst_port_spec_build (
    rule_match_app_t *app, types::RuleMatch_L4PortAppInfo *port_info)
{
    hal_ret_t       ret;

    if (!dllist_empty(&app->l4dstport_list)) {
        ret = port_list_elem_dst_port_spec_build(&app->l4dstport_list, port_info);
    }

    return ret;
}

static inline hal_ret_t
rule_match_src_port_spec_build (
    rule_match_app_t *app, types::RuleMatch_L4PortAppInfo *port_info)
{
    hal_ret_t       ret;

    if (!dllist_empty(&app->l4srcport_list)) {
        ret = port_list_elem_src_port_spec_build(&app->l4srcport_list, port_info);
    }

    return ret;
}

static inline hal_ret_t
rule_match_port_app_spec_build (rule_match_app_t *app,
                                  types::RuleMatch *spec)
{
    hal_ret_t                       ret;
    types::RuleMatch_L4PortAppInfo  *port_info;

    if (!dllist_empty(&app->l4srcport_list) ||
        !dllist_empty(&app->l4dstport_list)) {
        port_info = spec->add_app_match()->mutable_port_info();

        if ((ret = rule_match_src_port_spec_build(app, port_info)) != HAL_RET_OK)
            return ret;

        if ((ret = rule_match_dst_port_spec_build(app, port_info)) != HAL_RET_OK)
            return ret;
    }

    return ret;
}

static inline hal_ret_t
rule_match_app_spec_build (rule_match_t *match,
                           types::RuleMatch *spec)
{
    hal_ret_t   ret;

    ret = rule_match_port_app_spec_build(&match->app, spec);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // TODO Other app types
    return ret;
}

static inline hal_ret_t
rule_match_proto_spec_build (rule_match_t *match,
                             types::RuleMatch *spec)
{
    spec->set_protocol(match->proto);
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_dst_sg_spec_build (rule_match_t *match,
                              const types::RuleMatch *spec)
{
    hal_ret_t ret = HAL_RET_OK;

    //TBD: lseshan: Handle sg

    return ret;
}

static inline hal_ret_t
rule_match_src_sg_spec_build (rule_match_t *match,
                              const types::RuleMatch *spec)
{
    hal_ret_t ret = HAL_RET_OK;
    
    //TBD: lseshan: Handle sg

    return ret;
}

static inline hal_ret_t
rule_match_sg_spec_build (rule_match_t *match,
                          types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = rule_match_src_sg_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_sg_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

static hal_ret_t
rule_match_src_addr_spec_build (rule_match_t *match,
                                types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = addr_list_elem_spec_src_addr_build(
                &match->src_addr_list, spec)) != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

static hal_ret_t
rule_match_dst_addr_spec_build (rule_match_t *match,
                                types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = addr_list_elem_spec_dst_addr_build(
                &match->dst_addr_list, spec)) != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

static hal_ret_t
rule_match_addr_spec_build (rule_match_t *match,
                            types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = rule_match_src_addr_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_addr_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

hal_ret_t
rule_match_spec_build (rule_match_t *match,
                       types::RuleMatch *spec)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_addr_spec_build(match, spec)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_sg_spec_build(match, spec)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_proto_spec_build(match, spec)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_app_spec_build(match, spec)) != HAL_RET_OK)
        goto end;

end:
    return ret;
}

} //end namespace hal
