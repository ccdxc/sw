//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for Routes
//-----------------------------------------------------------------------------
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/src/nw/route.hpp"
#include "nic/hal/src/nw/route_acl.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Get key function for route hash table
// ----------------------------------------------------------------------------
void *
route_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    route_t                     *route = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    route = (route_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(route->key);

}

// ----------------------------------------------------------------------------
// Compute hash function for route hash table
// ----------------------------------------------------------------------------
uint32_t
route_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(route_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// Compare key function for route hash table
// ----------------------------------------------------------------------------
bool
route_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(route_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert route to db
//------------------------------------------------------------------------------
static inline hal_ret_t
route_add_to_db (route_t *route, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("{}:adding to route key hash table",
                    __FUNCTION__);
    // allocate an entry to establish mapping from l2key to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->route_ht()->insert_with_key(&route->key,
                                                       entry,
                                                       &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("failed to add route id to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // TODO(bharat): Add to "ACL"

    return ret;
}

//------------------------------------------------------------------------------
// delete a route from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
route_del_from_db (route_t *route)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("removing from route id hash table");
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->route_ht()->
        remove(&route->key);

    if (entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    } else {
        HAL_TRACE_ERR("unable to find route:{}", route_to_str(route));
        ret = HAL_RET_ROUTE_NOT_FOUND;
        goto end;
    }

end:
    return ret;;
}

//------------------------------------------------------------------------------
// validate an incoming route create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_route_create (RouteSpec& spec, RouteResponse *rsp)
{
    // key-handle field must be set
    if (!spec.has_key_or_handle() ||
        spec.key_or_handle().key_or_handle_case() == RouteKeyHandle::KEY_OR_HANDLE_NOT_SET) {
        HAL_TRACE_ERR("spec has no key or handle");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // check for NH to be present
    if (!spec.has_nh_key_or_handle()) {
        HAL_TRACE_ERR("spec has no nexthop key or handle");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// create add callback
// ----------------------------------------------------------------------------
hal_ret_t
route_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;

    // No PD calls
    return ret;
}

// ----------------------------------------------------------------------------
// create commit callback
// ----------------------------------------------------------------------------
hal_ret_t
route_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    route_t                         *route = NULL;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    hal_handle_t                    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (route_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    route = (route_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb {}", route_to_str(route));

    // Add route to key DB
    ret = route_add_to_db (route, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to add route to DB. err: {}", ret);
        goto end;
    }

    // add route to route "ACL"
    ret = route_acl_add_route(route);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to add route to \"ACL\". err: {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("added route to DB");
end:
    return ret;
}

// ----------------------------------------------------------------------------
// create abort callback
// ----------------------------------------------------------------------------
hal_ret_t
route_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret = HAL_RET_OK;
    dhl_entry_t    *dhl_entry = NULL;
    route_t        *route = NULL;
    hal_handle_t   hal_handle = 0;
    dllist_ctxt_t  *lnode = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    route = (route_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("route:{}: create abort cb", route_to_str(route));

    // remove the object
    hal_handle_free(hal_handle);

    // free PI route
    // route_cleanup(route);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
route_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
route_prepare_rsp (RouteResponse *rsp, hal_ret_t ret,
                     hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_route_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a route create request
// TODO: if route exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
route_create (RouteSpec& spec, RouteResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    route_key_t                     route_key = {0};
    route_t                         *route = NULL;
    nexthop_t                       *nh = NULL;
    vrf_t                           *vrf = NULL;
    route_create_app_ctxt_t         app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };

    hal_api_trace(" API Begin: route create ");

    auto kh = spec.key_or_handle();

    // validate the request message
    ret = validate_route_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // check for vrf
    vrf = vrf_lookup_key_or_handle(kh.route_key().vrf_key_handle());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Route create failure. Unable to find vrf {}",
                      vrf_lookup_key_or_handle_to_str(kh.route_key().vrf_key_handle()));
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    route_key.vrf_id = vrf->vrf_id;
    ip_pfx_spec_to_pfx_spec(&route_key.pfx, kh.route_key().ip_prefix());

    // check for route
    route = route_lookup_by_key(&route_key);
    if (route) {
        HAL_TRACE_ERR("Route create failure: {} Already exists",
                      route_to_str(route));
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // check for nexthop
    nh = nexthop_lookup_key_or_handle(spec.nh_key_or_handle());
    if (nh == NULL) {
        HAL_TRACE_ERR("Route Create failure. Unable to find nexthop {}",
                      nexthop_lookup_key_or_handle_to_str(spec.nh_key_or_handle()));
        ret = HAL_RET_NEXTHOP_NOT_FOUND;
        goto end;
    }

    // instantiate a route
    route = route_alloc_init();
    if (route == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("out of memory. err: {}", ret);
        goto end;
    }

    // allocate hal handle id
    route->hal_handle = hal_handle_alloc(HAL_OBJ_ID_ROUTE);
    if (route->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Route Create failure. Unable to alloc handle");
        route_cleanup(route);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    route->key = route_key;
    route->nh_handle = nh->hal_handle;

    dhl_entry.handle = route->hal_handle;
    dhl_entry.obj = route;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(route->hal_handle, &cfg_ctxt,
                             route_create_add_cb,
                             route_create_commit_cb,
                             route_create_abort_cb,
                             route_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK && ret != HAL_RET_ENTRY_EXISTS) {
	    if (route != NULL) {
            route_cleanup(route);
            route = NULL;
	    }
	    HAL_API_STATS_INC(HAL_API_ROUTE_CREATE_FAIL);
    } else {
	    HAL_API_STATS_INC(HAL_API_ROUTE_CREATE_SUCCESS);
    }
    route_prepare_rsp(rsp, ret, route ? route->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: route create ");
    return ret;
}

//------------------------------------------------------------------------------
// validate route update request
//------------------------------------------------------------------------------
hal_ret_t
validate_route_update (RouteSpec& spec, RouteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Lookup route from key or handle
//------------------------------------------------------------------------------
route_t *
route_lookup_key_or_handle (RouteKeyHandle& kh)
{
    vrf_t       *vrf = NULL;
    route_key_t route_key = {0};
    route_t     *route = NULL;

    vrf = vrf_lookup_key_or_handle(kh.route_key().vrf_key_handle());
    if (vrf == NULL) {
        goto end;
    }

    route_key.vrf_id = vrf->vrf_id;
    ip_pfx_spec_to_pfx_spec(&route_key.pfx, kh.route_key().ip_prefix());

    if (kh.key_or_handle_case() == RouteKeyHandle::kRouteKey) {
        route = route_lookup_by_key(&route_key);
    } else if (kh.key_or_handle_case() == RouteKeyHandle::kRouteHandle) {
        route = route_lookup_by_handle(kh.route_handle());
    }

end:
    return route;
}

//------------------------------------------------------------------------------
// Lookup route from key or handle to str
//------------------------------------------------------------------------------
const char *
route_lookup_key_or_handle_to_str (RouteKeyHandle& kh)
{
	static thread_local char       if_str[4][50];
	static thread_local uint8_t    if_str_next = 0;
	char                           *buf;
    ip_prefix_t                    pfx;

	buf = if_str[if_str_next++ & 0x3];
	memset(buf, 0, 50);

    if (kh.key_or_handle_case() == RouteKeyHandle::kRouteKey) {
        ip_pfx_spec_to_pfx_spec(&pfx, kh.route_key().ip_prefix());
        snprintf(buf, 50, "vrf: %s, pfx: %s",
                 vrf_lookup_key_or_handle_to_str(kh.route_key().vrf_key_handle()),
                 ippfx2str(&pfx));
    } else if (kh.key_or_handle_case() == RouteKeyHandle::kRouteHandle) {
        snprintf(buf, 50, "route_handle: 0x%lx", kh.route_handle());
    }

    return buf;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
route_make_clone (route_t *route, route_t **route_clone)
{
    *route_clone = route_alloc_init();
    memcpy(*route_clone, route, sizeof(route_t));

    // Make a PD clone if needed

    return HAL_RET_OK;
}

hal_ret_t
route_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// route update commit cb
//------------------------------------------------------------------------------
hal_ret_t
route_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    route_t                         *route = NULL/*, *route_clone = NULL*/;
    // route_update_app_ctxt_t       *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (route_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    route = (route_t *)dhl_entry->obj;
    // route_clone = (route_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}", route_to_str(route));

    // Free up original
    route_free(route);

end:
    return ret;
}

//------------------------------------------------------------------------------
// route update abort cb
//------------------------------------------------------------------------------
hal_ret_t
route_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    route_t                         *route = NULL;
    // route_update_app_ctxt_t       *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (route_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    route = (route_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort cb {}", route_to_str(route));

    // Free PI
    route_free(route);

end:
    return ret;
}

//------------------------------------------------------------------------------
// route update cleanup cb
//------------------------------------------------------------------------------
hal_ret_t
route_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// check what changes in the route update
//------------------------------------------------------------------------------
hal_ret_t
route_check_update (RouteSpec& spec, route_t *nh,
                      route_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;

    // TODO: Check for nh changes

    return ret;
}


//------------------------------------------------------------------------------
// process a route update request
//------------------------------------------------------------------------------
hal_ret_t
route_update (RouteSpec& spec, RouteResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    route_t                         *route = NULL;
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };
    route_update_app_ctxt_t         app_ctxt = { 0 };

    hal_api_trace(" API Begin: route update ");

    auto kh = spec.key_or_handle();

    // validate the request message
    ret = validate_route_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("route update validation failed, ret : {}", ret);
        goto end;
    }

    // retrieve route object
    route = route_lookup_key_or_handle(kh);
    if (route == NULL) {
        HAL_TRACE_ERR("failed to find route {}",
                      route_lookup_key_or_handle_to_str(kh));
        ret = HAL_RET_ROUTE_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("route update for {}", route_to_str(route));

    ret = route_check_update(spec, route, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("route check update failed, err: {}", ret);
        goto end;
    }

    // check if anything changed
    if (!app_ctxt.route_changed) {
        HAL_TRACE_ERR("no change in route update: noop");
        goto end;
    }

    route_make_clone(route, (route_t **)&dhl_entry.cloned_obj);

    dhl_entry.handle = route->hal_handle;
    dhl_entry.obj = route;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(route->hal_handle, &cfg_ctxt,
                             route_update_upd_cb,
                             route_update_commit_cb,
                             route_update_abort_cb,
                             route_update_cleanup_cb);

end:
    if (ret == HAL_RET_OK) {
	    HAL_API_STATS_INC(HAL_API_ROUTE_UPDATE_SUCCESS);
    } else {
        // TODO: Check if we have to cleanup clone
	    HAL_API_STATS_INC(HAL_API_ROUTE_UPDATE_FAIL);
    }
    route_prepare_rsp(rsp, ret,
                        route ? route->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: route update ");
    return ret;

}

//------------------------------------------------------------------------------
// process a get request for a given route
//------------------------------------------------------------------------------
static void
route_process_get (route_t *route, RouteGetResponse *rsp)
{
    // fill route key
    auto route_key_spec = rsp->mutable_spec()->mutable_key_or_handle()->
        mutable_route_key();
    route_key_spec->mutable_vrf_key_handle()->set_vrf_id(route->key.vrf_id);
    ip_pfx_to_spec(route_key_spec->mutable_ip_prefix(), &route->key.pfx);

    // fill operational state of this vrf
    rsp->mutable_status()->set_route_handle(route->hal_handle);


    if (route->nh_handle != HAL_HANDLE_INVALID) {
        rsp->mutable_spec()->mutable_nh_key_or_handle()->
            set_nexthop_handle(route->nh_handle);
    }

    rsp->set_api_status(types::API_STATUS_OK);
}

//------------------------------------------------------------------------------
// callback invoked from route hash table while processing route get request
//------------------------------------------------------------------------------
static bool
route_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    RouteGetResponseMsg *rsp      = (RouteGetResponseMsg *)ctxt;
    RouteGetResponse *response    = rsp->add_response();
    route_t *route                   = NULL;

    route = (route_t *)hal_handle_get_obj(entry->handle_id);
    route_process_get(route, response);

    // return false here, so that we walk through all hash table entries.
    return false;
}

//------------------------------------------------------------------------------
// process a route get request
//------------------------------------------------------------------------------
hal_ret_t
route_get (RouteGetRequest& req, RouteGetResponseMsg *rsp)
{
    route_t *route = NULL;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        // If the Vrf key handle field is not set, then this is a request
        // for information from all VRFs. Run through all VRFs in the hash
        // table and populate the response.
        g_hal_state->route_ht()->walk(route_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        route = route_lookup_key_or_handle(kh);
        auto response = rsp->add_response();
        if (route == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_ROUTE_GET_FAIL);
            return HAL_RET_ROUTE_NOT_FOUND;
        } else {
            route_process_get(route, response);
        }
    }

    HAL_API_STATS_INC(HAL_API_ROUTE_GET_SUCCESS);
    return HAL_RET_OK;
}

hal_ret_t
validate_route_delete_req (RouteDeleteRequest& req,
                             RouteDeleteResponse* rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate route delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_route_delete (route_t *route)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for back refs

    return ret;
}

//------------------------------------------------------------------------------
// Delete main cb
//------------------------------------------------------------------------------
hal_ret_t
route_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as route_delete_del_cb() was a succcess
//------------------------------------------------------------------------------
hal_ret_t
route_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    route_t                     *route = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    route = (route_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {}", route_to_str(route));


    // Remove from nexhtop key hash table
    ret = route_del_from_db(route);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to del route {} from db, err : {}",
                      route_to_str(route), ret);
        goto end;
    }

    // Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // Free PI route
    route_cleanup(route);

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit cbs can't fail: ret:{}", ret);
        HAL_ASSERT(0);
    }
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
route_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
route_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a route delete request
//------------------------------------------------------------------------------
hal_ret_t
route_delete (RouteDeleteRequest& req, RouteDeleteResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    route_t             *route = NULL;
    cfg_op_ctxt_t       cfg_ctxt = { 0 };
    dhl_entry_t         dhl_entry = { 0 };

    hal_api_trace(" API Begin: route delete ");

    auto kh = req.key_or_handle();

    // validate the request message
    ret = validate_route_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("route delete validation failed, err:: {}", ret);
        goto end;
    }

    route = route_lookup_key_or_handle(kh);
    if (route == NULL) {
        HAL_TRACE_ERR("failed to find route {}",
                      route_lookup_key_or_handle_to_str(kh));
        ret = HAL_RET_ROUTE_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("deleting route :{}", route_to_str(route));

    ret = validate_route_delete(route);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("route delete validation failed, err: {}", ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = route->hal_handle;
    dhl_entry.obj = route;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(route->hal_handle, &cfg_ctxt,
                             route_delete_del_cb,
                             route_delete_commit_cb,
                             route_delete_abort_cb,
                             route_delete_cleanup_cb);

end:
    if (ret == HAL_RET_OK) {
	    HAL_API_STATS_INC(HAL_API_ROUTE_DELETE_SUCCESS);
    } else {
	    HAL_API_STATS_INC(HAL_API_ROUTE_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: route delete ");
    return ret;
}

const char *
route_to_str (route_t *route)
{
    static thread_local char       route_str[4][50];
    static thread_local uint8_t    route_str_next = 0;
    char                           *buf;

    buf = route_str[route_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (route) {
        snprintf(buf, 50, "vrf_id: %lu, pfx: %s, nh_handle: %lu", route->key.vrf_id,
                 ippfx2str(&route->key.pfx), route->nh_handle);
    }
    return buf;
}

hal_ret_t
hal_route_init_cb (hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_route_cleanup_cb (void)
{
    return HAL_RET_OK;
}

}    // namespace hal
