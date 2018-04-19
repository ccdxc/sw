//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for Nexthops
//-----------------------------------------------------------------------------
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/src/nw/nh.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Get key function for nexthop hash table
// ----------------------------------------------------------------------------
void *
nexthop_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    nexthop_t                   *nh = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nh = (nexthop_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(nh->nh_id);

}

// ----------------------------------------------------------------------------
// Compute hash function for nexthop hash table
// ----------------------------------------------------------------------------
uint32_t
nexthop_id_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(nh_id_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// Compare key function for nexthop hash table
// ----------------------------------------------------------------------------
bool
nexthop_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(nh_id_t *)key1 == *(nh_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert nexthop to db
//------------------------------------------------------------------------------
static inline hal_ret_t
nexthop_add_to_db (nexthop_t *nh, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;
    if_t                        *hal_if;
    ep_t                        *ep;

    HAL_TRACE_DEBUG("{}:adding to nexthop key hash table",
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
    sdk_ret = g_hal_state->nexthop_id_ht()->insert_with_key(&nh->nh_id,
                                                            entry,
                                                            &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("failed to add nexthop id to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // add nexthop as back ref to if
    if (nh->if_handle != HAL_HANDLE_INVALID) {
        hal_if = find_if_by_handle(nh->if_handle);
        HAL_ASSERT(hal_if != NULL);
        ret = if_add_nh(hal_if, nh);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to add nh to if. err: {}", ret);
            goto end;
        }
    }

    // add nexthop as back ref to ep
    if (nh->ep_handle != HAL_HANDLE_INVALID) {
        ep = find_ep_by_handle(nh->ep_handle);
        HAL_ASSERT(ep != NULL);
        ret = ep_add_nh(ep, nh);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to add nh to if. err: {}", ret);
            goto end;
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// delete a nh from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
nexthop_del_from_db (nexthop_t *nh)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("removing from nexthop id hash table");
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->nexthop_id_ht()->
        remove(&nh->nh_id);

    if (entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    } else {
        HAL_TRACE_ERR("unable to find nexthop:{}", nexthop_to_str(nh));
        ret = HAL_RET_NEXTHOP_NOT_FOUND;
        goto end;
    }

end:
    return ret;;
}



//------------------------------------------------------------------------------
// validate an incoming nexthop create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_nexthop_create (NexthopSpec& spec, NexthopResponse *rsp)
{
    // key-handle field must be set
    if (!spec.has_key_or_handle() ||
        spec.key_or_handle().key_or_handle_case() == NexthopKeyHandle::KEY_OR_HANDLE_NOT_SET) {
        HAL_TRACE_ERR("{}:spec has no key or handle", __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// create add callback
// ----------------------------------------------------------------------------
hal_ret_t
nexthop_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;

    // No PD calls
    return ret;
}

// ----------------------------------------------------------------------------
// create commit callback
// ----------------------------------------------------------------------------
hal_ret_t
nexthop_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    nexthop_t                       *nh = NULL;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    hal_handle_t                    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nexthop_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nh = (nexthop_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb {}", nexthop_to_str(nh));

    // Add nexthop to key DB
    ret = nexthop_add_to_db (nh, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to add nexthop to DB. err: {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("added nexthop to DB");
end:
    return ret;
}

// ----------------------------------------------------------------------------
// create abort callback
// ----------------------------------------------------------------------------
hal_ret_t
nexthop_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret = HAL_RET_OK;
    dhl_entry_t    *dhl_entry = NULL;
    nexthop_t      *nh = NULL;
    hal_handle_t   hal_handle = 0;
    dllist_ctxt_t  *lnode = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nh = (nexthop_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("nexthop:{}: create abort cb", nexthop_to_str(nh));

    // remove the object
    hal_handle_free(hal_handle);

    // free PI nh
    nexthop_cleanup(nh);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
nexthop_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
nexthop_prepare_rsp (NexthopResponse *rsp, hal_ret_t ret,
                     hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_nexthop_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a nexthop create request
// TODO: if nexthop exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
nexthop_create (NexthopSpec& spec, NexthopResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    nexthop_t                       *nh = NULL;
    if_t                            *hal_if = NULL;
    ep_t                            *ep = NULL;
    nexthop_create_app_ctxt_t       app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };

    hal_api_trace(" API Begin: nexthop create ");

    auto kh = spec.key_or_handle();

    // validate the request message
    ret = validate_nexthop_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    nh = nexthop_lookup_by_id(kh.nexthop_id());
    if (nh) {
        HAL_TRACE_ERR("nexthop already {}", nexthop_to_str(nh));
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // check if if is present
    if (spec.if_or_ep_case() == NexthopSpec::kIfKeyOrHandle) {
        auto if_key_or_handle = spec.if_key_or_handle();
        hal_if = if_lookup_key_or_handle(if_key_or_handle);
        if (hal_if == NULL) {
            ret = HAL_RET_IF_NOT_FOUND;
            HAL_TRACE_ERR("Unable to find if {}",
                          if_lookup_key_or_handle_to_str(if_key_or_handle));
            goto end;
        }
    }

    // check if ep is present
    if (spec.if_or_ep_case() == NexthopSpec::kEpKeyOrHandle) {
        auto ep_kh = spec.ep_key_or_handle();
        ret = find_ep(ep_kh, &ep, NULL);
        if (ep == NULL) {
            HAL_TRACE_ERR("Unable to find ep err: {}", ret);
            ret = HAL_RET_EP_NOT_FOUND;
            goto end;
        }
    }

    // instantiate a nexthop
    nh = nexthop_alloc_init();
    if (nh == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("out of memory. err: {}", ret);
        goto end;
    }

    // allocate hal handle id
    nh->hal_handle = hal_handle_alloc(HAL_OBJ_ID_NEXTHOP);
    if (nh->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle",
                      __FUNCTION__);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }


    nh->nh_id = kh.nexthop_id();
    nh->if_handle = hal_if ? hal_if->hal_handle : HAL_HANDLE_INVALID;
    nh->ep_handle = ep ? ep->hal_handle : HAL_HANDLE_INVALID;

    dhl_entry.handle = nh->hal_handle;
    dhl_entry.obj = nh;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(nh->hal_handle, &cfg_ctxt,
                             nexthop_create_add_cb,
                             nexthop_create_commit_cb,
                             nexthop_create_abort_cb,
                             nexthop_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
	    if (nh != NULL) {
            // freed in abort CB
            nh = NULL;
	    }
	    HAL_API_STATS_INC(HAL_API_NEXTHOP_CREATE_FAIL);
    } else {
	    HAL_API_STATS_INC(HAL_API_NEXTHOP_CREATE_SUCCESS);
    }
    nexthop_prepare_rsp(rsp, ret, nh ? nh->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: nexthop create ");
    return ret;
}

//------------------------------------------------------------------------------
// validate nexthop update request
//------------------------------------------------------------------------------
hal_ret_t
validate_nexthop_update (NexthopSpec& spec, NexthopResponse *rsp)
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
// Lookup nexthop from key or handle
//------------------------------------------------------------------------------
nexthop_t *
nexthop_lookup_key_or_handle (NexthopKeyHandle& kh)
{
    nexthop_t                       *nh = NULL;

    if (kh.key_or_handle_case() == NexthopKeyHandle::kNexthopId) {
        nh = nexthop_lookup_by_id(kh.nexthop_id());
    } else if (kh.key_or_handle_case() == NexthopKeyHandle::kNexthopHandle) {
        nh = nexthop_lookup_by_handle(kh.nexthop_handle());
    }

    return nh;
}

//------------------------------------------------------------------------------
// Lookup nexthop from key or handle to str
//------------------------------------------------------------------------------
const char *
nexthop_lookup_key_or_handle_to_str (NexthopKeyHandle& kh)
{
	static thread_local char       if_str[4][50];
	static thread_local uint8_t    if_str_next = 0;
	char                           *buf;

	buf = if_str[if_str_next++ & 0x3];
	memset(buf, 0, 50);

    if (kh.key_or_handle_case() == NexthopKeyHandle::kNexthopId) {
        snprintf(buf, 50, "nh_id: %d", kh.nexthop_id());
    } else if (kh.key_or_handle_case() == NexthopKeyHandle::kNexthopHandle) {
        snprintf(buf, 50, "nh_handle: 0x%lx", kh.nexthop_handle());
    }

    return buf;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
nexthop_make_clone (nexthop_t *nh, nexthop_t **nh_clone)
{
    *nh_clone = nexthop_alloc_init();
    memcpy(*nh_clone, nh, sizeof(nexthop_t));

    return HAL_RET_OK;
}

hal_ret_t
nexthop_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// nexthop update commit cb
//------------------------------------------------------------------------------
hal_ret_t
nexthop_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    nexthop_t                       *nh = NULL/*, *nh_clone = NULL*/;
    // nexthop_update_app_ctxt_t       *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nexthop_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nh = (nexthop_t *)dhl_entry->obj;
    // nh_clone = (nexthop_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}", nexthop_to_str(nh));

    // Free PI
    nexthop_free(nh);

end:
    return ret;
}

//------------------------------------------------------------------------------
// nexthop update abort cb
//------------------------------------------------------------------------------
hal_ret_t
nexthop_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    nexthop_t                       *nh = NULL;
    // nexthop_update_app_ctxt_t       *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nexthop_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nh = (nexthop_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort cb {}", nexthop_to_str(nh));

    // Free PI
    nexthop_free(nh);

end:
    return ret;
}

//------------------------------------------------------------------------------
// nexthop update cleanup cb
//------------------------------------------------------------------------------
hal_ret_t
nexthop_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// check what changes in the nexthop update
//------------------------------------------------------------------------------
hal_ret_t
nexthop_check_update (NexthopSpec& spec, nexthop_t *nh,
                      nexthop_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;

    // TODO: Check for If or EP change

    return ret;
}


//------------------------------------------------------------------------------
// process a nexthop update request
//------------------------------------------------------------------------------
hal_ret_t
nexthop_update (NexthopSpec& spec, NexthopResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    nexthop_t                       *nh = NULL;
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };
    nexthop_update_app_ctxt_t       app_ctxt = { 0 };

    hal_api_trace(" API Begin: nexthop update ");

    auto kh = spec.key_or_handle();

    // validate the request message
    ret = validate_nexthop_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nexthop update validation failed, ret : {}", ret);
        goto end;
    }

    // retrieve nexthop object
    nh = nexthop_lookup_key_or_handle(kh);
    if (nh == NULL) {
        HAL_TRACE_ERR("failed to find nh {}",
                      nexthop_lookup_key_or_handle_to_str(kh));
        ret = HAL_RET_NEXTHOP_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("nexthop update for {}", nexthop_to_str(nh));

    ret = nexthop_check_update(spec, nh, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nexthop check update failed, err: {}", ret);
        goto end;
    }

    // check if anything changed
    if (!app_ctxt.nexthop_changed) {
        HAL_TRACE_ERR("no change in nexthop update: noop");
        goto end;
    }

    nexthop_make_clone(nh, (nexthop_t **)&dhl_entry.cloned_obj);

    dhl_entry.handle = nh->hal_handle;
    dhl_entry.obj = nh;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(nh->hal_handle, &cfg_ctxt,
                             nexthop_update_upd_cb,
                             nexthop_update_commit_cb,
                             nexthop_update_abort_cb,
                             nexthop_update_cleanup_cb);

end:
    if (ret == HAL_RET_OK) {
	    HAL_API_STATS_INC(HAL_API_NEXTHOP_UPDATE_SUCCESS);
    } else {
	    HAL_API_STATS_INC(HAL_API_NEXTHOP_UPDATE_FAIL);
    }
    nexthop_prepare_rsp(rsp, ret,
                        nh ? nh->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: nexthop update ");
    return ret;

}

//------------------------------------------------------------------------------
// process a get request for a given nexthop
//------------------------------------------------------------------------------
static void
nexthop_process_get (nexthop_t *nh, NexthopGetResponse *rsp)
{
    // fill config spec of this vrf
    rsp->mutable_spec()->mutable_key_or_handle()->set_nexthop_id(nh->nh_id);

    // fill operational state of this vrf
    rsp->mutable_status()->set_nexthop_handle(nh->hal_handle);


    if (nh->if_handle != HAL_HANDLE_INVALID) {
        rsp->mutable_spec()->mutable_if_key_or_handle()->
            set_if_handle(nh->if_handle);
    }

    if (nh->ep_handle != HAL_HANDLE_INVALID) {
        rsp->mutable_spec()->mutable_ep_key_or_handle()->
            set_endpoint_handle(nh->ep_handle);
    }

    rsp->set_api_status(types::API_STATUS_OK);
}

//------------------------------------------------------------------------------
// callback invoked from nexthop hash table while processing nexthop get request
//------------------------------------------------------------------------------
static bool
nexthop_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    NexthopGetResponseMsg *rsp      = (NexthopGetResponseMsg *)ctxt;
    NexthopGetResponse *response    = rsp->add_response();
    nexthop_t *nh                   = NULL;

    nh = (nexthop_t *)hal_handle_get_obj(entry->handle_id);
    nexthop_process_get(nh, response);

    // return false here, so that we walk through all hash table entries.
    return false;
}

//------------------------------------------------------------------------------
// process a nh get request
//------------------------------------------------------------------------------
hal_ret_t
nexthop_get (NexthopGetRequest& req, NexthopGetResponseMsg *rsp)
{
    nexthop_t *nh = NULL;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        // If the Vrf key handle field is not set, then this is a request
        // for information from all VRFs. Run through all VRFs in the hash
        // table and populate the response.
        g_hal_state->nexthop_id_ht()->walk(nexthop_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        nh = nexthop_lookup_key_or_handle(kh);
        auto response = rsp->add_response();
        if (nh == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_NEXTHOP_GET_FAIL);
            return HAL_RET_NEXTHOP_NOT_FOUND;
        } else {
            nexthop_process_get(nh, response);
        }
    }

    HAL_API_STATS_INC(HAL_API_NEXTHOP_GET_SUCCESS);
    return HAL_RET_OK;
}

hal_ret_t
validate_nexthop_delete_req (NexthopDeleteRequest& req,
                             NexthopDeleteResponse* rsp)
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
// validate nexthop delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_nexthop_delete (nexthop_t *nh)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of l2segs
    if (nh->route_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("NH delete failure, routes still referring :");
        hal_print_handles_block_list(nh->route_list);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Delete main cb
//------------------------------------------------------------------------------
hal_ret_t
nexthop_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as nexthop_delete_del_cb() was a succcess
//------------------------------------------------------------------------------
hal_ret_t
nexthop_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nexthop_t                   *nh = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nh = (nexthop_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {}", nexthop_to_str(nh));


    // Remove from nexhtop key hash table
    ret = nexthop_del_from_db(nh);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to del nexthop {} from db, err : {}",
                      nexthop_to_str(nh), ret);
        goto end;
    }

    // Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // Free PI nexthop
    nexthop_free(nh);

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
nexthop_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
nexthop_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a nexthop delete request
//------------------------------------------------------------------------------
hal_ret_t
nexthop_delete (NexthopDeleteRequest& req, NexthopDeleteResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    nexthop_t           *nh = NULL;
    cfg_op_ctxt_t       cfg_ctxt = { 0 };
    dhl_entry_t         dhl_entry = { 0 };

    hal_api_trace(" API Begin: nexthop delete ");

    auto kh = req.key_or_handle();

    // validate the request message
    ret = validate_nexthop_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nexthop delete validation failed, err:: {}", ret);
        goto end;
    }

    nh = nexthop_lookup_key_or_handle(kh);
    if (nh == NULL) {
        HAL_TRACE_ERR("failed to find nh {}",
                      nexthop_lookup_key_or_handle_to_str(kh));
        ret = HAL_RET_NEXTHOP_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("deleting NH :{}", nexthop_to_str(nh));

    ret = validate_nexthop_delete(nh);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nexthop delete validation failed, err: {}", ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = nh->hal_handle;
    dhl_entry.obj = nh;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(nh->hal_handle, &cfg_ctxt,
                             nexthop_delete_del_cb,
                             nexthop_delete_commit_cb,
                             nexthop_delete_abort_cb,
                             nexthop_delete_cleanup_cb);

end:
    if (ret == HAL_RET_OK) {
	    HAL_API_STATS_INC(HAL_API_NEXTHOP_DELETE_SUCCESS);
    } else {
	    HAL_API_STATS_INC(HAL_API_NEXTHOP_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: nexthop delete ");
    return ret;
}

const char *
nexthop_to_str (nexthop_t *nh)
{
    static thread_local char       nh_str[4][50];
    static thread_local uint8_t    nh_str_next = 0;
    char                           *buf;

    buf = nh_str[nh_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (nh) {
        if (nh->if_handle != HAL_HANDLE_INVALID) {
            snprintf(buf, 50, "nh_id: %d, if_handle: %lu", nh->nh_id, nh->if_handle);
        } else {
            snprintf(buf, 50, "nh_id: %d, ep_handle: %lu", nh->nh_id, nh->ep_handle);
        }
    }
    return buf;
}

hal_ret_t
hal_nh_init_cb (hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_nh_cleanup_cb (void)
{
    return HAL_RET_OK;
}

}    // namespace hal
