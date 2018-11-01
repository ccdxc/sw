//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::RuleMatch
//-----------------------------------------------------------------------------

#include "nic/include/hal_state.hpp"
#include "rule_match.hpp"
#include "utils.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"

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
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_EXACT, ipv4_tuple, icmp_type),
                RULE_FLD_DEF(acl::ACL_FIELD_TYPE_EXACT, ipv4_tuple, icmp_code),
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
   icmp_list_cleanup(&rule_match_app->icmp_list);
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
   sg_list_cleanup(&match->src_sg_list);
   sg_list_cleanup(&match->dst_sg_list);
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
// Spec Extraction routines
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
rule_match_port_app_spec_extract (const types::RuleMatch_AppMatch spec,
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
rule_match_icmp_app_spec_extract (const types::RuleMatch_AppMatch spec,
                                  rule_match_app_t *app)
{
    hal_ret_t ret;
    const types::RuleMatch_ICMPAppInfo icmp_info = spec.icmp_info(); 

    ret = icmp_list_elem_icmp_spec_handle(icmp_info.icmp_type(), 
                                           icmp_info.icmp_code(),
                                           &app->icmp_list);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
rule_match_app_spec_extract (const types::RuleMatch& spec, rule_match_t *match)
{
    hal_ret_t   ret;

    // walk the apps
    const types::RuleMatch_AppMatch app = spec.app_match();
    if (app.App_case() == types::RuleMatch_AppMatch::kPortInfo) {
        ret = rule_match_port_app_spec_extract(app, &match->app);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    } else if (app.App_case() == types::RuleMatch_AppMatch::kIcmpInfo) {
        ret = rule_match_icmp_app_spec_extract(app, &match->app);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    } else if (app.App_case() == types::RuleMatch_AppMatch::kRpcInfo) {

    } else if (app.App_case() == types::RuleMatch_AppMatch::kMsrpcInfo) {

    } else if (app.App_case() == types::RuleMatch_AppMatch::kEspInfo) {

    } else {
        HAL_TRACE_ERR("App type not Handled {}", app.App_case());
        return HAL_RET_OK;
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

//-----------------------------------------------------------------------------
// Rule ctr related routines
//-----------------------------------------------------------------------------
//

hal_ret_t
init_rule_ctr(rule_ctr_t *ctr)
{
    memset(ctr, 0, sizeof(rule_ctr_t));
    ctr->ht_ctxt.reset();
    ref_init(&ctr->ref_count, [] (const ref_t *ref) {
        rule_ctr_t *ctr = container_of(ref, rule_ctr_t, ref_count);

        HAL_TRACE_DEBUG("Free rule ctr with pointer {:#x} key: {}", (uint64_t)ctr, ctr->rule_key);
        g_hal_state->rule_ctr_slab()->free(ctr);
    });
    return HAL_RET_OK;
}

rule_ctr_t *
alloc_init_rule_ctr(rule_cfg_t *cfg, rule_key_t rule_key)
{
    rule_ctr_t *ctr;

    ctr = (rule_ctr_t *)cfg->rule_ctr_ht->lookup((void *) &rule_key);
    if (ctr != NULL) {
        HAL_TRACE_DEBUG("found rule ctr for rule key: {}", rule_key);
        ref_inc(&ctr->ref_count);
        return ctr;
    }

    ctr = (rule_ctr_t *) g_hal_state->rule_ctr_slab()->alloc();
    if (ctr == NULL) {
        return NULL;
    }
    init_rule_ctr(ctr);
    ctr->rule_key = rule_key;
    cfg->rule_ctr_ht->insert(ctr, &ctr->ht_ctxt);
    return ctr;
}

void
free_rule_ctr(rule_ctr_t *ctr)
{
    ref_dec(&((rule_ctr_t *) ctr)->ref_count);
}

hal_ret_t
init_rule_data(rule_data_t *rule_data, void *usr_data, void *ctr)
{
    memset(rule_data, 0, sizeof(rule_data_t));
    ref_init(&rule_data->ref_cnt, [] (const ref_t *ref) {
        rule_data_t *data = container_of(ref, rule_data_t, ref_cnt);
        nwsec_rule_t *rule = container_of((acl::ref_t *)data->user_data, nwsec_rule_t, ref_count);
        
        HAL_TRACE_DEBUG("Free rule data key: {}", rule->rule_id);
        ref_dec((acl::ref_t *)data->ctr);
        ref_dec((acl::ref_t *)data->user_data);
        g_hal_state->rule_data_slab()->free(data);
    });

    ref_inc((const acl::ref_t *)usr_data);

    rule_data->user_data = usr_data;
    rule_data->ctr = ctr;
    return HAL_RET_OK;
}

rule_data_t *
alloc_init_rule_data(void *usr_data, void *ctr)
{
    rule_data_t *data;
    data = (rule_data_t *) g_hal_state->rule_data_slab()->alloc();
    if (data == NULL) {
        return NULL;
    }

    init_rule_data(data, usr_data, ctr);
    data->user_data = usr_data;
    data->ctr = ctr;
    return data;
}

void
free_rule_data(rule_data_t *data)
{
    ref_dec(&((rule_data_t *) data)->ref_cnt);
}
    
//-----------------------------------------------------------------------------
// Rule Manipulation routines (rule add/del)
//-----------------------------------------------------------------------------


void *rule_ctr_get_key_func(void *entry)
{
    return (void *)&((rule_ctr_t *)entry)->rule_key;
}

uint32_t rule_ctr_compute_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(rule_key_t)) % ht_size;
}

bool  rule_ctr_compare_key_func(void *key1, void *key2) 
{
    return memcmp(key1, key2, sizeof(rule_key_t)) == 0;
}
void *rule_cfg_get_key_func(void *entry)
{
    return (void *)((rule_cfg_t *)entry)->name;
}

uint32_t rule_cfg_compute_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, strlen((char *)key)) % ht_size;
}

bool  rule_cfg_compare_key_func(void *key1, void *key2) 
{
    return strcmp((char *)key1, (char *)key2) == 0;
}

static ht *g_rule_cfg_ht = ht::factory(256, rule_cfg_get_key_func,
									   rule_cfg_compute_hash_func,
									   rule_cfg_compare_key_func,
									   false /* not thread_safe */ );	
void 
rule_lib_delete(const char *name)
{
	rule_cfg_t 	*rcfg = NULL;
	const acl_ctx_t  *acl_ctx = acl::acl_get(name);
	if (acl_ctx) {
        HAL_TRACE_DEBUG("deleted acl");
    	acl::acl_delete(acl_ctx);	
	}
    // walk rule_data_ht and free them
	rcfg = (rule_cfg_t *)g_rule_cfg_ht->remove((void *)name);
	if (rcfg) {
    	g_hal_state->rule_cfg_slab()->free(rcfg);
	} else {
        HAL_TRACE_ERR("Rule cfg not found");
    }
    return;
}

const acl_ctx_t  *
rule_lib_init(const char *name, acl_config_t *cfg)
{
    
    rule_cfg_t  *rcfg = NULL;
    rcfg = (rule_cfg_t *) g_hal_state->rule_cfg_slab()->alloc();
    if (rcfg == NULL) {
        return NULL;
    }
    memcpy(&rcfg->acl_cfg, &ip_acl_config_glbl, sizeof(acl_config_t));
    memcpy(&rcfg->name, name, 64);
    rcfg->acl_ctx = acl_create(name, (const acl_config_t *)&rcfg->acl_cfg);

    rcfg->rule_ctr_ht = ht::factory(8192,
                               rule_ctr_get_key_func,
                               rule_ctr_compute_hash_func,
                               rule_ctr_compare_key_func);
    HAL_ASSERT_RETURN((rcfg->rule_ctr_ht != NULL), NULL);

	// Rule cfg is not maintained as part of hal_state. rule lib data is not expected to persistent
    // Each plugin has to recreate the rules.
	g_rule_cfg_ht->insert((void *)rcfg, &rcfg->ht_ctxt);	

    return rcfg->acl_ctx;
}
rule_cfg_t *
rule_cfg_get(const char *name)
{
    rule_cfg_t *rule_cfg = (rule_cfg_t *)g_rule_cfg_ht->lookup((void *)name);
    if (rule_cfg == NULL) {
        return NULL;
    }
    return rule_cfg;
}

rule_ctr_t *
rule_ctr_get(rule_cfg_t *rcfg, rule_key_t key)
{
    rule_ctr_t *ctr = (rule_ctr_t *) rcfg->rule_ctr_ht->lookup((void *) &key);
    return ctr;
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
        HAL_TRACE_DEBUG(" decrement userdata");
        ref_dec((acl::ref_t *)rule->data.userdata);
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
                       IPProtocol proto, uint32_t icmp_type, uint32_t icmp_code, 
                       uint16_t ethertype)
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
        if (proto == types::IPPROTO_TCP || 
            proto == types::IPPROTO_UDP) {
            if (sp_entry->port_range.port_lo || sp_entry->port_range.port_hi) {
                rule->field[PORT_SRC].value.u32 = sp_entry->port_range.port_lo;
                rule->field[PORT_SRC].mask_range.u32 = sp_entry->port_range.port_hi;
            }
            if (dp_entry->port_range.port_lo || dp_entry->port_range.port_hi) {
                rule->field[PORT_DST].value.u32 = dp_entry->port_range.port_lo;
                rule->field[PORT_DST].mask_range.u32 = dp_entry->port_range.port_hi;
            }
        } else if (proto == types::IPPROTO_ICMP ||
                   proto == types::IPPROTO_ICMPV6) {
            rule->field[ICMP_TYPE].value.u32 = icmp_type;
            rule->field[ICMP_CODE].value.u32 = icmp_code;
        }
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

static inline hal_ret_t
rule_match_process_rule (const acl_ctx_t **acl_ctx,
                         rule_match_t     *match,
                         int               rule_prio,
                         void             *ref_count,
                         bool             add)
{
    ipv4_rule_t          *rule;
    rule_match_app_t     *app_match = &match->app;
    hal_ret_t            ret = HAL_RET_OK;
    mac_addr_list_elem_t *mac_src_addr, *mac_dst_addr;
    addr_list_elem_t     *src_addr, *dst_addr;
    port_list_elem_t     *dst_port, *src_port;
    sg_list_elem_t       *src_sg, *dst_sg;
    dllist_ctxt_t        *sa_entry, *da_entry, *sp_entry, *dp_entry, *icmp_entry;
    dllist_ctxt_t        *mac_sa_entry, *mac_da_entry, *dst_sg_entry, *src_sg_entry;
    icmp_list_elem_t     *icmp;

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
                            if (match->proto == types::IPPROTO_TCP || types::IPPROTO_UDP) { 
                                /* L4 Src-Port loop */
                                dllist_for_each(sp_entry, &app_match->l4srcport_list) {
                                    src_port = RULE_MATCH_GET_PORT(sp_entry);
                                     /* L4 Dst-Port loop */
                                    dllist_for_each(dp_entry, &app_match->l4dstport_list) {
                                        dst_port = RULE_MATCH_GET_PORT(dp_entry);
                                        rule = construct_rule_fields(src_addr, dst_addr,  mac_src_addr,
                                                                mac_dst_addr, src_port,
                                                                dst_port, src_sg, dst_sg, match->proto,0, 0,
                                                                match->ethertype);
                                        if (!rule) {
                                            HAL_TRACE_DEBUG("rule allocation failed");
                                            continue;
                                        } else {
                                            PRINT_RULE_FIELDS(rule);
                                        }
                                        rule->data.priority = rule_prio;
                                        rule->data.userdata = ref_count;
                                        if (add) {
                                            /* Rule add */
                                            ret = acl_add_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
                                            if (ret != HAL_RET_OK) {
                                                HAL_TRACE_ERR("Unable to create the acl rules");
                                                return ret;
                                            }
                                            ref_inc((acl::ref_t *)ref_count);
                                        } else {
                                            /* Rule delete */
                                            ret = acl_del_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
                                            if (ret != HAL_RET_OK) {
                                                HAL_TRACE_ERR("Unable to delete the acl rules");
                                                return ret;
                                            }
                                            ref_dec((acl::ref_t *)ref_count);
                                            acl_rule_deref((const acl_rule_t *)rule);
                                        }
                                    } //  < push it to the vector of ipv4_rule_t >
                                }
                            } else if (match->proto == types::IPPROTO_ICMP || types::IPPROTO_ICMPV6) {
                                /* ICMP loop */
                                dllist_for_each(icmp_entry, &app_match->icmp_list) {
                                    icmp = RULE_MATCH_GET_ICMP(icmp_entry);
                                    rule = construct_rule_fields(src_addr, dst_addr,  mac_src_addr,
                                                        mac_dst_addr, src_port,
                                                        dst_port, src_sg, dst_sg, match->proto,
                                                        icmp->icmp_type, icmp->icmp_code,
                                                        match->ethertype);
                                    if (!rule) {
                                        HAL_TRACE_DEBUG("rule allocation failed");
                                        continue;
                                    } else {
                                        PRINT_RULE_FIELDS(rule);
                                    }
                                    rule->data.priority = rule_prio;
                                    rule->data.userdata = ref_count;
                                    if (add) {
                                        /* Rule add */
                                        ret = acl_add_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
                                        if (ret != HAL_RET_OK) {
                                            HAL_TRACE_ERR("Unable to create the acl rules");
                                            return ret;
                                        }
                                        ref_inc((acl::ref_t *)ref_count);
                                    } else {
                                        /* Rule delete */
                                        ret = acl_del_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
                                        if (ret != HAL_RET_OK) {
                                            HAL_TRACE_ERR("Unable to delete the acl rules");
                                            return ret;
                                        }
                                        ref_dec((acl::ref_t *)ref_count);
                                        acl_rule_deref((const acl_rule_t *)rule);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// rule_match_rule_add api adds the rules to the acl library.
// As of today type of fields that are instantiated in acl_lib
// (pkt_classify_lib) will be of type ipv4_rule_t across plugins that use
// types.RuleMatch 
// Todo: Take only the ctx name from user. (lseshan)
hal_ret_t
rule_match_rule_add (const acl_ctx_t  **acl_ctx,
                     rule_match_t     *match,
					 rule_key_t 	   rule_key,
                     int               rule_prio,
                     void             *ref_count)
{
    hal_ret_t            ret = HAL_RET_OK;
    port_list_elem_t     dst_port_new = {0}, src_port_new = {0};
    addr_list_elem_t     src_addr_new = {0}, dst_addr_new = {0};
    sg_list_elem_t       src_sg_new = {0}, dst_sg_new = {0};
    mac_addr_list_elem_t mac_src_addr_new = {0}, mac_dst_addr_new = {0};
    rule_data_t          *rule_data = NULL;
	rule_cfg_t			 *rule_cfg = NULL;
    rule_ctr_t           *rule_ctr = NULL;

	rule_cfg = (rule_cfg_t *)g_rule_cfg_ht->lookup((void *) (*acl_ctx)->name());
	if (rule_cfg == NULL) {
		return HAL_RET_ERR;
	}
    rule_ctr = alloc_init_rule_ctr(rule_cfg, rule_key);

    HAL_TRACE_DEBUG("alloc rule data with key : {}", rule_key);
    rule_data = alloc_init_rule_data(ref_count, &rule_ctr->ref_count);

    /* Add dummy node at the head of each list if the list is empty. If the
       list is not empty then we shouldn't insert a wildcard match for that
       field, so no dummy node if the list is not empty */
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->src_mac_addr_list, &mac_src_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->dst_mac_addr_list, &mac_dst_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->src_addr_list, &src_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->dst_addr_list, &dst_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->app.l4dstport_list, &dst_port_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->app.l4srcport_list, &src_port_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->src_sg_list, &src_sg_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_ADD(&match->dst_sg_list, &dst_sg_new.list_ctxt);
    
    /* Add the rule */
    ret = rule_match_process_rule(acl_ctx, match, rule_prio, &rule_data->ref_cnt, true);

    /* Delete dummy nodes at the head of each list */
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&mac_src_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&mac_dst_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&src_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&dst_addr_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&dst_port_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&src_port_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&src_sg_new.list_ctxt);
    RULE_MATCH_DLLIST_CHECK_EMPTY_DEL(&dst_sg_new.list_ctxt);
    ref_dec((acl::ref_t *)&rule_data->ref_cnt);
    return ret;
}

hal_ret_t
rule_match_rule_del (const acl_ctx_t   **acl_ctx, 
					 rule_match_t 	    *match,
                     rule_key_t          key,
                     int                 rule_prio,
                     void               *ref_count)
{
    hal_ret_t            ret = HAL_RET_OK;
    rule_data_t          *data = NULL;
	rule_cfg_t			 *rule_cfg = NULL;


	rule_cfg = (rule_cfg_t *)g_rule_cfg_ht->lookup((void *) (*acl_ctx)->name());
    if (rule_cfg == NULL) {
        return HAL_RET_ERR;
    }

    ret = rule_match_process_rule(acl_ctx, match, rule_prio, ref_count, false);
    rule_match_cleanup(match);

    data = (rule_data_t *) rule_cfg->rule_ctr_ht->remove(&key);
    if (data) {
        free_rule_data(data);
    }
    return ret;
}

void *
rule_classify(const acl_ctx_t *ctx, const uint8_t *key,
              const acl_rule_t *rules[], uint32_t categories)
{
    hal_ret_t ret = acl_classify(ctx, key, rules, categories);
    if (ret != HAL_RET_OK) {
        return NULL;
    }

    if (*rules == NULL) {
        return NULL;
    }
    return NULL;
}

acl::ref_t *
get_rule_data(acl_rule_t * rule)
{
    acl::ref_t *rc;
    rc = (acl::ref_t *) rule->data.userdata;
    rule_data_t *rule_data = (hal::rule_data_t *) container_of(rc, rule_data_t, ref_cnt);

    acl::ref_t *user_ref;

    user_ref = (acl::ref_t *) rule_data->user_data;
    return user_ref;
}

rule_ctr_t *
get_rule_ctr(acl_rule_t *rule)
{
    acl::ref_t *rc;
    rc = (acl::ref_t *) rule->data.userdata;
    rule_data_t *rule_data = (hal::rule_data_t *) container_of(rc, rule_data_t, ref_cnt);

    return (hal::rule_ctr_t *) RULE_MATCH_USER_DATA((acl::ref_t *)rule_data->ctr,
                                    rule_ctr_t, ref_count);
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
        port_info = spec->mutable_app_match()->mutable_port_info();

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
