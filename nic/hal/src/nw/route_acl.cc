//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles Route ACL
//-----------------------------------------------------------------------------

#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nw/route_acl.hpp"

namespace hal {

// field defs for route "ACL"
acl::acl_config_t route_acl_config = {
    num_categories: 1,
    num_fields: ROUTE_TUPLE_MAX,
    defs:  {
        ROUTE_ACL_FLD_DEF(ACL_FIELD_TYPE_EXACT, route_tuple_t, vrf_id),
        ROUTE_ACL_FLD_DEF(ACL_FIELD_TYPE_PREFIX, route_tuple_t, ip_pfx),
    }
};


//-----------------------------------------------------------------------------
// add route acl rule
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_rule_add(const acl_ctx_t **acl_ctx, route_acl_rule_t *rule)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = acl_add_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to add route acl rule. err: {}", ret);
        return ret;
    }
    return ret;
}

//-----------------------------------------------------------------------------
// del route acl rule
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_rule_del(const acl_ctx_t **acl_ctx, route_acl_rule_t *rule)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = acl_del_rule((const acl_ctx_t **)acl_ctx, (const acl_rule_t *)rule);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to delete route acl rule. ret: {}", ret);
        return ret;
    }
    // TODO: What is this?
    //acl_rule_deref((const acl_rule_t *) rule);
    return ret;
}

//-----------------------------------------------------------------------------
// allocates route acl rule.
//  - allocates memory
//  - fills in priority, userdata. Make sure userdata is on heap.
//  - provide free function
//-----------------------------------------------------------------------------
route_acl_rule_t *
route_acl_rule_alloc(hal_handle_t hal_handle)
{
    route_acl_rule_t *rule = NULL;
    hal_handle_t *handle = NULL;

    rule = (route_acl_rule_t *)g_hal_state->route_acl_rule_slab()->alloc();
    handle = (hal_handle_t *)g_hal_state->hal_handle_id_slab()->alloc();

    *handle = hal_handle;

    // set rule properties
    rule->data.priority = 0;            // all prefixes are of same priority
    rule->data.category_mask = 0x01;    // Why 1 ??
    rule->data.userdata = (void *)handle;

    // set free func. callback
    ref_init(&rule->ref_count, [] (const ref_t * ref_count) {
        route_acl_rule_t *rule  = (route_acl_rule_t *)acl_rule_from_ref(ref_count);
        ref_dec(&((route_acl_rule_t *)rule->data.userdata)->ref_count);
        // free rule
        g_hal_state->route_acl_rule_slab()->free((void *)acl_rule_from_ref(ref_count));
        // free user data (hal_handle_t)
        g_hal_state->hal_handle_id_slab()->free((route_acl_rule_t *)rule->data.userdata);
    });

    return rule;
}

//-----------------------------------------------------------------------------
// route acl rules commit. Call after rules are added
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_rule_commit(const acl_ctx_t *acl_ctx)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = acl_commit(acl_ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to commit the route ACL rules");
        return ret;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// add route to the route "ACL"
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_add_route(route_t *route)
{
    hal_ret_t ret = HAL_RET_OK;
    route_acl_rule_t *route_rule = NULL;
    const acl_ctx_t *acl_ctx = g_hal_state->route_acl();

    if (!route) {
        HAL_TRACE_ERR("Route is NULL");
        goto end;
    }

    route_rule = route_acl_rule_alloc(route->hal_handle);
    route_rule->field[ROUTE_TUPLE_VRF_ID].value.u32 = route->key.vrf_id;
    // TODO: Extend for v6
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].value.u32 = route->key.pfx.addr.addr.v4_addr;
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].mask_range.u32 = route->key.pfx.len;
    route_rule->data.priority = 32 - route->key.pfx.len;

    route_acl_rule_add(&acl_ctx, route_rule);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// del route from the route "ACL"
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_del_route(route_t *route)
{
    hal_ret_t ret = HAL_RET_OK;
    route_acl_rule_t *route_rule = NULL;
    const acl_ctx_t *acl_ctx = g_hal_state->route_acl();

    if (!route) {
        HAL_TRACE_ERR("Route is NULL");
        goto end;
    }

    route_rule = route_acl_rule_alloc(route->hal_handle);
    route_rule->field[ROUTE_TUPLE_VRF_ID].value.u32 = route->key.vrf_id;
    // TODO: Extend for v6
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].value.u32 = route->key.pfx.addr.addr.v4_addr;
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].mask_range.u32 = route->key.pfx.len;

    route_acl_rule_del(&acl_ctx, route_rule);

    // TODO: Check if rules are being freed properly
end:
    return ret;
}

//-----------------------------------------------------------------------------
// route lookup. Should do LPM.
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_lookup(route_key_t *key, hal_handle_t *handle)
{
    hal_ret_t ret = HAL_RET_OK;
    const acl_ctx_t *acl_ctx = g_hal_state->route_acl();
    route_tuple_t tuple = {0};
    route_acl_rule_t *rule;

    if (!key) {
        HAL_TRACE_ERR("Route key is NULL");
        goto end;
    }

    tuple.vrf_id = key->vrf_id;
    tuple.ip_pfx = key->pfx.addr.addr.v4_addr;
    acl_classify(acl_ctx, (const uint8_t*)&tuple, (const acl_rule_t **)&rule, 0x01);

    *handle = *(hal_handle_t *)rule->data.userdata;
end:
    return ret;
}

}    // namespace hal
