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
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_EXACT, ipv4_tuple, mac_src),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_EXACT, ipv4_tuple, mac_dst),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_EXACT, ipv4_tuple, ethertype),
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
    dllist_reset(&match->src_mac_addr_list);
    dllist_reset(&match->dst_mac_addr_list);
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
rule_match_ethertype_cleanup (rule_match_t *match)
{
   match->ethertype = 0;
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
   mac_addr_list_cleanup(&match->src_mac_addr_list);
   mac_addr_list_cleanup(&match->dst_mac_addr_list);
}

void
rule_match_cleanup (rule_match_t *match)
{
    rule_match_addr_cleanup(match);
    rule_match_sg_cleanup(match);
    rule_match_proto_cleanup(match);
    rule_match_ethertype_cleanup(match);
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
rule_match_port_app_spec_extract (const types::RuleMatch_AppMatchInfo spec,
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
rule_match_app_spec_extract (const types::RuleMatch& spec, rule_match_t *match)
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
rule_match_proto_spec_extract (const types::RuleMatch& spec,
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
        ret = sg_list_elem_sg_spec_handle(spec.dst_sg(i), &match->dst_sg_list);
        if (ret != HAL_RET_OK)
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
        ret = sg_list_elem_sg_spec_handle(spec.src_sg(i), &match->src_sg_list);
        if (ret != HAL_RET_OK)
            return ret;
    }

    return ret;
}

static inline hal_ret_t
rule_match_sg_spec_extract (const types::RuleMatch& spec, rule_match_t *match)
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
rule_match_dst_mac_addr_spec_extract (const types::RuleMatch& spec,
                                      rule_match_t *match)
{
    hal_ret_t ret;

    for (int i = 0; i < spec.dst_mac_address_size(); i++) {
        if ((ret = mac_addr_elem_add(spec.dst_mac_address(i), 
                                     &match->dst_addr_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
rule_match_src_mac_addr_spec_extract (const types::RuleMatch& spec,
                                      rule_match_t *match)
{
    hal_ret_t ret;

    for (int i = 0; i < spec.src_mac_address_size(); i++) {
        if ((ret = mac_addr_elem_add(spec.src_mac_address(i), 
                                     &match->src_addr_list)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_ethertype_spec_extract (const types::RuleMatch&  spec,
                                   rule_match_t *match)
{
    match->ethertype = spec.ether_type();
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

    if ((ret = rule_match_src_mac_addr_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_mac_addr_spec_extract(spec, match)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

hal_ret_t
rule_match_spec_extract (const types::RuleMatch& spec, rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_addr_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_sg_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_proto_spec_extract(spec, match)) != HAL_RET_OK)
        goto end;

    if ((ret = rule_match_ethertype_spec_extract(spec, match)) != HAL_RET_OK)
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
        rule_data_t *rule  = rule_data_from_ref(ref_count);
        rule->data_free(rule->userdata);
        g_hal_state->rule_data_slab()->free((void *)rule);
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

void
rule_lib_deref(ipv4_rule_t *rule)
{
    acl_rule_deref((acl_rule_t *) rule);
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
        ref_dec(&((rule_data_t *)rule->data.userdata)->ref_count);
        g_hal_state->ipv4_rule_slab()->free((void *)acl_rule_from_ref(ref_count));
    });
    return rule;
}

static ipv4_rule_t *
construct_rule_fields (addr_list_elem_t *sa_entry, addr_list_elem_t *da_entry,
                       mac_addr_list_elem_t *mac_sa_entry,
                       mac_addr_list_elem_t *mac_da_entry,
                       port_list_elem_t *sp_entry, port_list_elem_t *dp_entry,
                       sg_list_elem_t *src_sg_entry, sg_list_elem_t *dst_sg_entry,
                       IPProtocol proto, uint16_t ethertype)
{
    ipv4_rule_t     *rule = NULL;

    rule  = rule_lib_alloc();
    if (sa_entry->num_addrs) {
        rule->field[IP_SRC].value.u32 = sa_entry->ip_range.vx_range[0].v4_range.ip_lo;
        rule->field[IP_SRC].mask_range.u32 = sa_entry->ip_range.vx_range[0].v4_range.ip_hi;
    }
    if (da_entry->num_addrs) {
        rule->field[IP_DST].value.u32 = da_entry->ip_range.vx_range[0].v4_range.ip_lo;
        rule->field[IP_DST].mask_range.u32 = da_entry->ip_range.vx_range[0].v4_range.ip_hi;
    }
    if (sp_entry->port_range.port_lo || sp_entry->port_range.port_hi) {
        rule->field[PORT_SRC].value.u32 = sp_entry->port_range.port_lo;
        rule->field[PORT_SRC].mask_range.u32 = sp_entry->port_range.port_hi;
    }
    if (dp_entry->port_range.port_lo || dp_entry->port_range.port_hi) {
        rule->field[PORT_DST].value.u32 = dp_entry->port_range.port_lo;
        rule->field[PORT_DST].mask_range.u32 = dp_entry->port_range.port_hi;
    }
    if (proto != types::IPPROTO_NONE) {
        rule->field[PROTO].value.u8 = proto;
        rule->field[PROTO].mask_range.u8 = 0xFF;
    }
    if (mac_sa_entry->addr != 0) {
        rule->field[MAC_SRC].value.u32 = mac_sa_entry->addr;
        rule->field[MAC_SRC].mask_range.u32 = 0xFFFFFFFF;
    }
    if (mac_da_entry->addr != 0) {
        rule->field[MAC_DST].value.u32 = mac_da_entry->addr;
        rule->field[MAC_DST].mask_range.u32 = 0xFFFFFFFF;
    }
    if (src_sg_entry->sg_id != 0) {
        rule->field[SRC_SG].value.u32 = src_sg_entry->sg_id;
        rule->field[SRC_SG].mask_range.u32 = 0xFFFFFFFF;
    }
    if (dst_sg_entry->sg_id != 0) {
        rule->field[DST_SG].value.u32 = dst_sg_entry->sg_id;
        rule->field[DST_SG].mask_range.u32 = 0xFFFFFFFF;
    }
    if (ethertype != 0) {
        rule->field[ETHERTYPE].value.u16 = ethertype;
        rule->field[ETHERTYPE].mask_range.u16 = 0xFFFF;
    }
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
    ipv4_rule_t          *rule;
    rule_match_app_t     *app_match = &match->app;
    hal_ret_t            ret = HAL_RET_OK;
    mac_addr_list_elem_t *mac_src_addr, *mac_dst_addr;
    addr_list_elem_t     *src_addr, *dst_addr;
    port_list_elem_t     *dst_port, *src_port;
    sg_list_elem_t       *src_sg, *dst_sg;
    dllist_ctxt_t        *sa_entry, *da_entry, *sp_entry, *dp_entry;
    dllist_ctxt_t        *mac_sa_entry, *mac_da_entry, *dst_sg_entry, *src_sg_entry;
    port_list_elem_t     dst_port_new = {0}, src_port_new = {0};
    addr_list_elem_t     src_addr_new = {0}, dst_addr_new = {0};
    sg_list_elem_t       src_sg_new = {0}, dst_sg_new = {0};
    mac_addr_list_elem_t mac_src_addr_new = {0}, mac_dst_addr_new = {0};

    /* Add dummy node at the head of the list if the list is empty. If the
       list is not empty then we shouldn't insert a wildcard match for that
       field, so no dummy node if the list is not empty */
    if (dllist_empty(&match->src_mac_addr_list)) {
        dllist_add(&match->src_mac_addr_list, &mac_src_addr_new.list_ctxt);
    }
    if (dllist_empty(&match->dst_mac_addr_list)) {
        dllist_add(&match->dst_mac_addr_list, &mac_dst_addr_new.list_ctxt);
    }
    if (dllist_empty(&match->src_addr_list)) {
        dllist_add(&match->src_addr_list, &src_addr_new.list_ctxt);
    }
    if (dllist_empty(&match->dst_addr_list)) {
        dllist_add(&match->dst_addr_list, &dst_addr_new.list_ctxt);
    }
    if (dllist_empty(&app_match->l4dstport_list)) {
        dllist_add(&app_match->l4dstport_list, &dst_port_new.list_ctxt);
    }
    if (dllist_empty(&app_match->l4srcport_list)) {
        dllist_add(&app_match->l4srcport_list, &src_port_new.list_ctxt);
    }
    if (dllist_empty(&match->src_sg_list)) {
        dllist_add(&match->src_sg_list, &src_sg_new.list_ctxt);
    }
    if (dllist_empty(&match->dst_sg_list)) {
        dllist_add(&match->dst_sg_list, &dst_sg_new.list_ctxt);
    }

    /* SRC-SG loop */
    dllist_for_each(src_sg_entry, &match->src_sg_list) {
        src_sg = RULE_MATCH_GET_SG(src_sg_entry);
        /* DST-SG loop */
        dllist_for_each(dst_sg_entry, &match->dst_sg_list) {
            dst_sg = RULE_MATCH_GET_SG(dst_sg_entry);
            /* MAC-SA loop */
            dllist_for_each(mac_sa_entry, &match->src_mac_addr_list) {
                mac_src_addr = RULE_MATCH_GET_MAC_ADDR(mac_sa_entry);
                /* MAC-DA loop */
                dllist_for_each(mac_da_entry, &match->dst_mac_addr_list) {
                    mac_dst_addr = RULE_MATCH_GET_MAC_ADDR(mac_da_entry);
                    /* IP-SA loop */
                    dllist_for_each(sa_entry, &match->src_addr_list) {
                        src_addr = RULE_MATCH_GET_ADDR(sa_entry);
                        /* IP-DA loop */
                        dllist_for_each(da_entry, &match->dst_addr_list) {
                            dst_addr = RULE_MATCH_GET_ADDR(da_entry);
                            /* L4 Src-Port loop */
                            dllist_for_each(sp_entry, &app_match->l4srcport_list) {
                                src_port = RULE_MATCH_GET_PORT(sp_entry);
                                 /* L4 Dst-Port loop */
                                dllist_for_each(dp_entry, &app_match->l4dstport_list) {
                                    dst_port = RULE_MATCH_GET_PORT(dp_entry);
                                     rule = construct_rule_fields(src_addr, dst_addr,  mac_src_addr,
                                                            mac_dst_addr, src_port,
                                                            dst_port, src_sg, dst_sg, match->proto,
                                                            match->ethertype);
                                    if (!rule) {
                                        HAL_TRACE_DEBUG("rule allocation failed");
                                        continue;
                                    } else {
                                        PRINT_RULE_FIELDS(rule);
                                    }
                                    rule->data.priority = rule_prio;
                                    rule->data.userdata = (void *)data;
                                    ret = acl_add_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
                                    if (ret != HAL_RET_OK) {
                                        HAL_TRACE_ERR("Unable to create the acl rules");
                                        return ret;
                                    }
                                    ref_inc(&data->ref_count);
                                }//  < push it to the vector of ipv4_rule_t >
                            }
                        }
                    }
                }
            }
        }
    }

    /* Delete dummy node at the head of the list */
    if (!dllist_empty(&mac_src_addr_new.list_ctxt)) {
        dllist_del(&mac_src_addr_new.list_ctxt);
    }
    if (!dllist_empty(&mac_dst_addr_new.list_ctxt)) {
        dllist_del(&mac_dst_addr_new.list_ctxt);
    }
    if (!dllist_empty(&src_addr_new.list_ctxt)) {
        dllist_del(&src_addr_new.list_ctxt);
    }
    if (!dllist_empty(&dst_addr_new.list_ctxt)) {
        dllist_del(&dst_addr_new.list_ctxt);
    }
    if (!dllist_empty(&dst_port_new.list_ctxt)) {
        dllist_del(&dst_port_new.list_ctxt);
    }
    if (!dllist_empty(&src_port_new.list_ctxt)) {
        dllist_del(&src_port_new.list_ctxt);
    }
    if (!dllist_empty(&src_sg_new.list_ctxt)) {
        dllist_del(&src_sg_new.list_ctxt);
    }
    if (!dllist_empty(&dst_sg_new.list_ctxt)) {
        dllist_del(&dst_sg_new.list_ctxt);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// Build routines
//-----------------------------------------------------------------------------
static inline hal_ret_t
rule_match_dst_port_spec_build (
    rule_match_app_t *app, types::RuleMatch_L4PortAppInfo *port_info)
{
    dllist_ctxt_t *entry;
    port_list_elem_t *port_lelem;
    types::L4PortRange *port_range;

    dllist_for_each(entry, &app->l4dstport_list) {
        port_lelem = dllist_entry(entry, port_list_elem_t, list_ctxt);
        port_range = port_info->add_dst_port_range();
        port_list_elem_l4portrange_spec_build(port_lelem, port_range);
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_src_port_spec_build (
    rule_match_app_t *app, types::RuleMatch_L4PortAppInfo *port_info)
{
    dllist_ctxt_t *entry;
    port_list_elem_t *port_lelem;
    types::L4PortRange *port_range;

    dllist_for_each(entry, &app->l4srcport_list) {
        port_lelem = dllist_entry(entry, port_list_elem_t, list_ctxt);
        port_range = port_info->add_src_port_range();
        port_list_elem_l4portrange_spec_build(port_lelem, port_range);
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_port_app_spec_build (rule_match_app_t *app, types::RuleMatch *spec)
{
    hal_ret_t ret;
    types::RuleMatch_L4PortAppInfo *port_info;

    if (!dllist_empty(&app->l4srcport_list) ||
        !dllist_empty(&app->l4dstport_list)) {
        port_info = spec->add_app_match()->mutable_port_info();

        if ((ret = rule_match_src_port_spec_build(
                app, port_info)) != HAL_RET_OK)
            return ret;

        if ((ret = rule_match_dst_port_spec_build(
                app, port_info)) != HAL_RET_OK)
            return ret;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_app_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = rule_match_port_app_spec_build(&match->app, spec)) != HAL_RET_OK)
        return ret;

    // TODO Other app types
    return ret;
}

static inline hal_ret_t
rule_match_proto_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    spec->set_protocol(match->proto);
    return HAL_RET_OK;
}

static inline hal_ret_t 
rule_match_dst_sg_spec_build (rule_match_t *match, const types::RuleMatch *spec)
{
    //TBD: lseshan: Handle sg
    return HAL_RET_OK;
}

static inline hal_ret_t 
rule_match_src_sg_spec_build (rule_match_t *match, const types::RuleMatch *spec)
{
    //TBD: lseshan: Handle sg
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_sg_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = rule_match_src_sg_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_sg_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
rule_match_dst_addr_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    hal_ret_t ret;
    dllist_ctxt_t *entry;
    addr_list_elem_t *addr_lelem;

    dllist_for_each(entry, &match->dst_addr_list) {
        auto addr = spec->add_dst_address();
        addr_lelem = dllist_entry(entry, addr_list_elem_t, list_ctxt);
        if ((ret = addr_list_elem_ipaddressobj_spec_build(
                addr_lelem, addr)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_src_addr_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    hal_ret_t ret;
    dllist_ctxt_t *entry;
    addr_list_elem_t *addr_lelem;

    dllist_for_each(entry, &match->src_addr_list) {
        auto addr = spec->add_src_address();
        addr_lelem = dllist_entry(entry, addr_list_elem_t, list_ctxt);
        if ((ret = addr_list_elem_ipaddressobj_spec_build(
                addr_lelem, addr)) != HAL_RET_OK)
            return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
rule_match_addr_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    hal_ret_t ret;

    if ((ret = rule_match_src_addr_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_dst_addr_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    return ret;
}

hal_ret_t
rule_match_spec_build (rule_match_t *match, types::RuleMatch *spec)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_addr_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_sg_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_proto_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    if ((ret = rule_match_app_spec_build(match, spec)) != HAL_RET_OK)
        return ret;

    return ret;
}

} //end namespace hal
