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
// route "ACL" user data alloc
//-----------------------------------------------------------------------------
route_acl_user_data_t *
route_acl_userdata_alloc (void)
{
    route_acl_user_data_t *udata = NULL;

    udata = (route_acl_user_data_t *)g_hal_state->
        route_acl_userdata_slab()->alloc();

    return udata;
}

//-----------------------------------------------------------------------------
// route "ACL" user data init
//-----------------------------------------------------------------------------
route_acl_user_data_t *
route_acl_userdata_init (route_acl_user_data_t *udata)
{
    if (!udata) {
        return NULL;
    }

    udata->route_handle = HAL_HANDLE_INVALID;
    ref_init(&udata->ref_count, [] (const ref_t * ref) {
        route_acl_user_data_t * udata = container_of(ref, route_acl_user_data_t, ref_count);
        g_hal_state->route_acl_userdata_slab()->free(udata);

        // cleanup oper state of route
        // Remove hal_handle -> route
        // Free up route structure
        route_clean_handle_mapping(udata->route_handle);
    });
    ref_inc(&udata->ref_count);

    return udata;
}

//-----------------------------------------------------------------------------
// route "ACL" user data alloc and init
//-----------------------------------------------------------------------------
route_acl_user_data_t *
route_acl_userdata_alloc_init ()
{
    return route_acl_userdata_init(route_acl_userdata_alloc());
}


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
    route_acl_user_data_t *udata = NULL;

    rule = (route_acl_rule_t *)g_hal_state->route_acl_rule_slab()->alloc();
    udata = route_acl_userdata_alloc_init();
    udata->route_handle = hal_handle;

    // set rule properties
    rule->data.priority = 0;            // all prefixes are of same priority
    rule->data.category_mask = 0x01;    // Why 1 ??
    rule->data.userdata = (void *)udata;

    // set free func. callback
    ref_init(&rule->ref_count, [] (const ref_t * ref_count) {
        route_acl_rule_t *rule  = (route_acl_rule_t *)acl_rule_from_ref(ref_count);
        ref_dec(&((route_acl_rule_t *)rule->data.userdata)->ref_count);
        // free rule
        g_hal_state->route_acl_rule_slab()->free((void *)acl_rule_from_ref(ref_count));
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
// create route "ACL"
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_create()
{
    const acl_ctx_t *acl_ctx = acl_create("route_acl", &route_acl_config);
    acl_commit(acl_ctx);
    acl_deref(acl_ctx);

    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// add route to the route "ACL"
//-----------------------------------------------------------------------------
hal_ret_t
route_acl_add_route(route_t *route)
{
    hal_ret_t ret = HAL_RET_OK;
    route_acl_rule_t *route_rule = NULL;
    const acl_ctx_t *acl_ctx = acl_get("route_acl");

    if (!route) {
        HAL_TRACE_ERR("Route is NULL");
        goto end;
    }

    route_rule = route_acl_rule_alloc(route->hal_handle);
    route_rule->field[ROUTE_TUPLE_VRF_ID].value.u32 = route->key.vrf_id;
    route_rule->field[ROUTE_TUPLE_VRF_ID].mask_range.u32 = 0xFFFFFFFF;
    // TODO: Extend for v6
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].value.u32 = route->key.pfx.addr.addr.v4_addr;
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].mask_range.u32 = route->key.pfx.len;
    route_rule->data.priority = 32 - route->key.pfx.len;

    route_acl_rule_add(&acl_ctx, route_rule);
    acl_commit(acl_ctx);

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
    const acl_ctx_t *acl_ctx = acl_get("route_acl");

    if (!route) {
        HAL_TRACE_ERR("Route is NULL");
        goto end;
    }

    route_rule = route_acl_rule_alloc(route->hal_handle);
    route_rule->field[ROUTE_TUPLE_VRF_ID].value.u32 = route->key.vrf_id;
    route_rule->field[ROUTE_TUPLE_VRF_ID].mask_range.u32 = 0xFFFFFFFF;
    // TODO: Extend for v6
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].value.u32 = route->key.pfx.addr.addr.v4_addr;
    route_rule->field[ROUTE_TUPLE_IP_PREFIX].mask_range.u32 = route->key.pfx.len;

    route_acl_rule_del(&acl_ctx, route_rule);
    acl_commit(acl_ctx);

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
    const acl_ctx_t *acl_ctx = acl_get("route_acl");
    route_tuple_t tuple = {0};
    route_acl_rule_t *rule;
    route_acl_user_data_t *udata = NULL;

    if (!key) {
        HAL_TRACE_ERR("Route key is NULL");
        goto end;
    }

    tuple.vrf_id = key->vrf_id;
    tuple.ip_pfx = key->pfx.addr.addr.v4_addr;
    acl_classify(acl_ctx, (const uint8_t*)&tuple, (const acl_rule_t **)&rule, 0x01);

    if (rule == NULL) {
        ret = HAL_RET_ROUTE_NOT_FOUND;
        goto end;
    }

    udata = (route_acl_user_data_t *)rule->data.userdata;
    *handle = udata->route_handle;

end:
    acl_deref(acl_ctx);
    return ret;
}

}    // namespace hal
