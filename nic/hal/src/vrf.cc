// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/if_utils.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// hash table vrf_id => entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
vrf_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry = NULL;
    vrf_t                       *vrf      = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    vrf = (vrf_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(vrf->vrf_id);
}

// ----------------------------------------------------------------------------
// hash table vrf_id => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
vrf_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(vrf_id_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table vrf_id => entry - compare function
// ----------------------------------------------------------------------------
bool
vrf_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(vrf_id_t *)key1 == *(vrf_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a vrf to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
vrf_add_to_db (vrf_t *vrf, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-vrf:{}:adding to vrf id hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from vrf id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->vrf_id_ht()->insert_with_key(&vrf->vrf_id,
                                                        entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to add vrf id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a vrf from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
vrf_del_from_db (vrf_t *vrf)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-vrf:{}:removing from vrf id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->vrf_id_ht()->
        remove(&vrf->vrf_id);

    // free up
    g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);

    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// Print vrf spec
//-----------------------------------------------------------------------------
hal_ret_t
vrf_spec_print (VrfSpec& spec)
{
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;
    ip_addr_t           my_tep;
    ip_prefix_t         gipo_pfx;

    buf.write("Vrf Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() == VrfKeyHandle::kVrfId) {
            buf.write("vrf_id:{}, ", kh.vrf_id());
        } else if (kh.key_or_handle_case() == VrfKeyHandle::kVrfHandle) {
            buf.write("vrf_hdl:{}, ", kh.vrf_handle());
        }
    } else {
        buf.write("vrf_id_hdl:NULL, ");
    }

    if (spec.has_security_key_handle()) {
        auto kh = spec.security_key_handle();
        if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileId) {
            buf.write("sec_prof_id:{}, ", kh.profile_id());
        } else if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileHandle) {
            buf.write("sec_prof_hdl:{}, ", kh.profile_handle());
        }
    } else {
        buf.write("sec_pro_id_hdl:NULL, ");
    }

    buf.write("type: {}, ", (spec.vrf_type() == types::VrfType::VRF_TYPE_NONE) ? "none" :
              (spec.vrf_type() == types::VrfType::VRF_TYPE_INFRA) ? "infra" : "customer");

    ip_addr_spec_to_ip_addr(&my_tep, spec.mytep_ip());
    ip_pfx_spec_to_pfx_spec(&gipo_pfx, spec.gipo_prefix());
    buf.write("my_tep:{}, gipo_pfx:{}/{}", ipaddr2str(&my_tep), ipaddr2str(&gipo_pfx.addr), gipo_pfx.len);

    HAL_TRACE_DEBUG(buf.c_str());
    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming vrf create request
// TODO:
// 1. check if vrf exists
// 2. validate L4 profile existence if that handle is valid
//------------------------------------------------------------------------------
static hal_ret_t
validate_vrf_create (VrfSpec& spec, VrfResponse *rsp)
{

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-vrf:{}:vrf id and handle not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != VrfKeyHandle::kVrfId) {
        // key-handle field set, but vrf id not provided
        HAL_TRACE_ERR("pi-vrf:{}:vrf id not set in request", __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // check if vrf id is in the valid range
    if (kh.vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("pi-vrf:{}:vrf id {} invalid in the request", __FUNCTION__,
                      HAL_VRF_ID_INVALID);
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
vrf_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_vrf_args_t        pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                    *vrf = NULL;
    vrf_create_app_ctxt_t    *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (vrf_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->obj;

    // PD Call to allocate PD resources and HW programming
    pd::pd_vrf_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    pd_vrf_args.nwsec_profile = app_ctxt->sec_prof;
    ret = pd::pd_vrf_create(&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to create vrf pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as vrf_create_add_cb() was a success
//      a. Add to vrf id hash table
//------------------------------------------------------------------------------
hal_ret_t
vrf_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                       *vrf       = NULL;
    hal_handle_t                hal_handle = 0;
    vrf_create_app_ctxt_t       *app_ctxt  = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (vrf_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    // 1. a. Add to vrf id hash table
    ret = vrf_add_to_db(vrf, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to add vrf {} to db, err : {}", 
                __FUNCTION__, vrf->vrf_id, ret);
        goto end;
    }

    // Add vrf to nwsec profile
    if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        ret = nwsec_prof_add_vrf(app_ctxt->sec_prof, vrf);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-vrf:{}:failed to add rel. from nwsec. prof",
                          __FUNCTION__);
            goto end;
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// vrf_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI vrf 
//------------------------------------------------------------------------------
hal_ret_t
vrf_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_vrf_args_t pd_vrf_args = { 0 };
    dllist_ctxt_t *lnode =  NULL;
    dhl_entry_t *dhl_entry        = NULL;
    vrf_t *vrf                    = NULL;
    hal_handle_t hal_handle       = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    vrf = (vrf_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-vrf:{}:create abort CB {}",
                    __FUNCTION__, vrf->vrf_id);

    // 1. delete call to PD
    if (vrf->pd) {
        pd::pd_vrf_args_init(&pd_vrf_args);
        pd_vrf_args.vrf = vrf;
        ret = pd::pd_vrf_delete(&pd_vrf_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-vrf:{}:failed to delete vrf pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI vrf
    vrf_free(vrf);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
vrf_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
vrf_prepare_rsp (VrfResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_vrf_status()->set_vrf_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a vrf create request
// TODO: if vrf exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
vrf_create (VrfSpec& spec, VrfResponse *rsp)
{
    hal_ret_t                   ret       = HAL_RET_OK;
    vrf_t                       *vrf      = NULL;
    nwsec_profile_t             *sec_prof = NULL;
    vrf_create_app_ctxt_t       app_ctxt  = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt  = { 0 };

    hal_api_trace(" API Begin: vrf create ");
    HAL_TRACE_DEBUG("pi-vrf:{}:creating vrf with id {}",
                    __FUNCTION__, spec.key_or_handle().vrf_id());

    // dump spec
    vrf_spec_print(spec);

    // validate the request message
    ret = validate_vrf_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:validation failed, ret : {}", 
                __FUNCTION__, ret);
        goto end;
    }

    // check if vrf exists already, and reject if one is found
    if (vrf_lookup_by_id(spec.key_or_handle().vrf_id())) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to create a vrf, "
                      "vrf {} exists already", __FUNCTION__, 
                      spec.key_or_handle().vrf_id());
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate a PI vrf object
    vrf = vrf_alloc_init();
    if (vrf == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to create teanant, err : {}", ret);
        ret = HAL_RET_OOM;
        goto end;
    }
    vrf->vrf_type             = spec.vrf_type();
    vrf->vrf_id               = spec.key_or_handle().vrf_id();
    vrf->nwsec_profile_handle = spec.security_key_handle().profile_handle();
    if (vrf->nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("pi-vrf:{}: No nwsec prof passed, "
                        "using default security profile", __FUNCTION__);
        sec_prof = NULL;
    } else {
        sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
        if (sec_prof == NULL) {
            HAL_TRACE_ERR("pi-vrf:{}:Failed to create vrf, "
                          "security profile with handle {} not found", 
                          __FUNCTION__, vrf->nwsec_profile_handle);
            rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
            vrf_free(vrf);
            ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
            goto end;
        }
    }

    if (vrf->vrf_type == types::VRF_TYPE_INFRA) {
        // Update global mytep ip.
        // Assumption: There is only one mytep ip. So for all tunnel ifs,
        //             my tep ip have to be same.
        ip_addr_spec_to_ip_addr(g_hal_state->oper_db()->mytep(), 
                                spec.mytep_ip());
        ip_pfx_spec_to_pfx_spec(&vrf->gipo_prefix, spec.gipo_prefix());
        HAL_TRACE_DEBUG("pi-vrf: {} Local VTEP: {}; GIPo Prefix: {}/{}",
                        __FUNCTION__,
                        ipaddr2str(g_hal_state->oper_db()->mytep()),
                        ipaddr2str(&vrf->gipo_prefix.addr),
                        vrf->gipo_prefix.len);
    }

    // allocate hal handle id
    vrf->hal_handle = hal_handle_alloc(HAL_OBJ_ID_VRF);
    if (vrf->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-vrf:{}: failed to alloc handle {}", 
                      __FUNCTION__, vrf->vrf_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        vrf_free(vrf);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    app_ctxt.sec_prof = sec_prof;
    dhl_entry.handle  = vrf->hal_handle;
    dhl_entry.obj     = vrf;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(vrf->hal_handle, &cfg_ctxt, 
                             vrf_create_add_cb,
                             vrf_create_commit_cb,
                             vrf_create_abort_cb, 
                             vrf_create_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        if (vrf) {
            // if there is an error, if will be freed in abort CB
            vrf = NULL;
        }
    }

    vrf_prepare_rsp(rsp, ret, 
                      vrf ? vrf->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: vrf create ");
    return ret;
}


//------------------------------------------------------------------------------
// handling nwsec update
//------------------------------------------------------------------------------
hal_ret_t
vrf_handle_nwsec_update (vrf_t *vrf, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t       ret       = HAL_RET_OK;
    l2seg_t         *l2seg    = NULL;
    hal_handle_t    *p_hdl_id = NULL;

    if (vrf == NULL) {
        return ret;
    }

    HAL_TRACE_DEBUG("pi-vrf:{}:vrf_id: {}", 
                    __FUNCTION__, vrf->vrf_id);
    // Walk L2 segs
    for (const void *ptr : *vrf->l2seg_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        l2seg = find_l2seg_by_handle(*p_hdl_id);
        if (!l2seg) {
            HAL_TRACE_ERR("pi-vrf:{}:unable to find l2seg with handle:{}",
                          __FUNCTION__, *p_hdl_id);
            continue;
        }
        l2seg_handle_nwsec_update(l2seg, nwsec_prof);
    }

    return ret;
}


//------------------------------------------------------------------------------
// validate vrf update request
//------------------------------------------------------------------------------
hal_ret_t
validate_vrf_update (VrfSpec& spec, VrfResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-vrf:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// check if nwsec profile changed for vrf
//------------------------------------------------------------------------------
hal_ret_t
vrf_nwsec_update (VrfSpec& spec, vrf_t *vrf, bool *nwsec_change,
                     hal_handle_t *new_nwsec_handle)
{
    *nwsec_change = false;
    if (vrf->nwsec_profile_handle != spec.security_key_handle().profile_handle()) {
        HAL_TRACE_DEBUG("pi-vrf:{}:nwSec profile updated", __FUNCTION__);
        *nwsec_change = true;
        *new_nwsec_handle = spec.security_key_handle().profile_handle();
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update with new nwsec profile
//------------------------------------------------------------------------------
hal_ret_t
vrf_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_vrf_args_t        pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                    *vrf = NULL;
    vrf_update_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (vrf_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-vrf:{}:update upd CB {}",
                    __FUNCTION__, vrf->vrf_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_vrf_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    pd_vrf_args.nwsec_profile = app_ctxt->nwsec_prof;
    ret = pd::pd_vrf_update(&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to update vrf pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Pass the vrf(old/new is fine) and new nwsec profile
    if (app_ctxt->nwsec_prof_change) {
        // Triggers reprogramming of input properties table for enicifs and uplinks
        vrf_handle_nwsec_update(vrf, app_ctxt->nwsec_prof);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
// - Clone will have the lists copied as its just a pointer
//------------------------------------------------------------------------------
hal_ret_t
vrf_make_clone (vrf_t *ten, vrf_t **ten_clone)
{
    *ten_clone = vrf_alloc_init();
    memcpy(*ten_clone, ten, sizeof(vrf_t));

    pd::pd_vrf_make_clone(ten, *ten_clone);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD vrf.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
vrf_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_vrf_args_t        pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                    *vrf = NULL, *vrf_clone = NULL;
    vrf_update_app_ctxt_t    *app_ctxt = NULL;
    nwsec_profile_t             *nwsec_prof = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (vrf_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->obj;
    vrf_clone = (vrf_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-vrf:{}:update commit CB {}",
                    __FUNCTION__, vrf->vrf_id);

    // update clone with new attrs
    if (app_ctxt->nwsec_prof_change) {
        vrf_clone->nwsec_profile_handle = app_ctxt->nwsec_profile_handle;

        if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
            // detach from old nwsec prof
            nwsec_prof = find_nwsec_profile_by_handle(
                         vrf->nwsec_profile_handle);
            if (nwsec_prof == NULL) {
                HAL_TRACE_ERR("pi-vrf:{}:unable to find nwsec prof "
                        "with hdl:{}",
                        __FUNCTION__, vrf->nwsec_profile_handle);
                goto end;
            }
            ret = nwsec_prof_del_vrf(nwsec_prof, vrf);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-vrf:{}:unable to detach old nwsec prof",
                        __FUNCTION__);
                goto end;
            }
        }

        if (vrf_clone->nwsec_profile_handle != HAL_HANDLE_INVALID) {
            // attach to new nwsec prof
            ret = nwsec_prof_add_vrf(app_ctxt->nwsec_prof, vrf_clone);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-vrf:{}:unable to attach new nwsec prof",
                        __FUNCTION__);
                goto end;
            }
        }
    }

    // Free PD
    pd::pd_vrf_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    ret = pd::pd_vrf_mem_free(&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to delete vrf pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI. Make sure the lists copied into clone are untouched
    vrf_free(vrf);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
vrf_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_vrf_args_t           pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                       *vrf = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (vrf_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-vrf:{}:update commit CB {}",
                    __FUNCTION__, vrf->vrf_id);

    // Free PD
    pd::pd_vrf_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    ret = pd::pd_vrf_mem_free(&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to delete vrf pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free Clone
    vrf_cleanup(vrf);
end:

    return ret;
}

hal_ret_t
vrf_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a vrf update request
//------------------------------------------------------------------------------
hal_ret_t
vrf_update (VrfSpec& spec, VrfResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    vrf_t                    *vrf = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const VrfKeyHandle       &kh = spec.key_or_handle();
    vrf_update_app_ctxt_t    app_ctxt = { 0 };

    hal_api_trace(" API Begin: vrf update");

    // dump spec
    vrf_spec_print(spec);

    // validate the request message
    ret = validate_vrf_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:vrf delete validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    vrf = vrf_lookup_key_or_handle(kh);
    if (vrf == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to find vrf, id {}, handle {}",
                      __FUNCTION__, kh.vrf_id(), kh.vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-vrf:{}:update vrf {}", __FUNCTION__, 
                    vrf->vrf_id);

    ret = vrf_nwsec_update(spec, vrf, &app_ctxt.nwsec_prof_change, 
                              &app_ctxt.nwsec_profile_handle);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to check if nwsec is updated.", 
                      __FUNCTION__);
        goto end;
    }

    if (!app_ctxt.nwsec_prof_change) {
        HAL_TRACE_ERR("pi-vrf:{}:no change in vrf update: noop", __FUNCTION__);
        // Its a no-op. We can just return HAL_RET_OK
        // ret = HAL_RET_INVALID_OP;
        goto end;
    }

    if (app_ctxt.nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("pi-vrf:{}: No nwsec prof passed, "
                        "using default security profile", __FUNCTION__);
        app_ctxt.nwsec_prof = NULL;
    } else {
        app_ctxt.nwsec_prof = find_nwsec_profile_by_handle(app_ctxt.nwsec_profile_handle);
        if (app_ctxt.nwsec_prof == NULL) {
            HAL_TRACE_ERR("pi-vrf:{}:security profile with handle {} not found", 
                    __FUNCTION__, 
                    app_ctxt.nwsec_profile_handle);
            ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
            goto end;
        } else {
            HAL_TRACE_DEBUG("pi-vrf:{}:new nwsec profile id: {}", __FUNCTION__, 
                    app_ctxt.nwsec_prof->profile_id);
        }
    }

    vrf_make_clone(vrf, (vrf_t **)&dhl_entry.cloned_obj);

    // form ctxt and call infra update object
    dhl_entry.handle = vrf->hal_handle;
    dhl_entry.obj = vrf;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(vrf->hal_handle, &cfg_ctxt, 
                             vrf_update_upd_cb,
                             vrf_update_commit_cb,
                             vrf_update_abort_cb, 
                             vrf_update_cleanup_cb);

end:
    vrf_prepare_rsp(rsp, ret, 
                       vrf ? vrf->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: vrf update ");
    return ret;
}

//------------------------------------------------------------------------------
// process a vrf get request
//------------------------------------------------------------------------------
hal_ret_t
vrf_get (VrfGetRequest& req, VrfGetResponse *rsp)
{
    vrf_t        *vrf;
    nwsec_profile_t *sec_prof = NULL;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = req.key_or_handle();
    if (kh.key_or_handle_case() == VrfKeyHandle::kVrfId) {
        vrf = vrf_lookup_by_id(kh.vrf_id());
    } else if (kh.key_or_handle_case() == VrfKeyHandle::kVrfHandle) {
        vrf = vrf_lookup_by_handle(kh.vrf_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (vrf == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_VRF_NOT_FOUND;
    }

    // fill config spec of this vrf
    rsp->mutable_spec()->mutable_key_or_handle()->set_vrf_id(vrf->vrf_id);
    sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    if (sec_prof != NULL) {
        rsp->mutable_spec()->mutable_security_key_handle()->set_profile_id(sec_prof->profile_id);
    }

    // fill operational state of this vrf
    rsp->mutable_status()->set_vrf_handle(vrf->hal_handle);

    // fill stats of this vrf
    rsp->mutable_stats()->set_num_l2_segments(vrf->num_l2seg);
    rsp->mutable_stats()->set_num_security_groups(vrf->num_sg);
    rsp->mutable_stats()->set_num_l4lb_services(vrf->num_l4lb_svc);
    rsp->mutable_stats()->set_num_endpoints(vrf->num_ep);
    rsp->mutable_spec()->set_vrf_type(vrf->vrf_type);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate vrf delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_vrf_delete_req (VrfDeleteRequest& req, VrfDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-vrf:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Lookup vrf from key or handle
//------------------------------------------------------------------------------
vrf_t *
vrf_lookup_key_or_handle (const VrfKeyHandle& kh)
{
    vrf_t     *vrf = NULL;

    if (kh.key_or_handle_case() == VrfKeyHandle::kVrfId) {
        vrf = vrf_lookup_by_id(kh.vrf_id());
    } else if (kh.key_or_handle_case() == VrfKeyHandle::kVrfHandle) {
        vrf = vrf_lookup_by_handle(kh.vrf_handle());
    }

    return vrf;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret         = HAL_RET_OK;
    pd::pd_vrf_args_t           pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode      = NULL;
    dhl_entry_t                 *dhl_entry  = NULL;
    vrf_t                       *vrf        = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the vrf. 
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    vrf = (vrf_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-vrf:{}:delete del CB {}",
                    __FUNCTION__, vrf->vrf_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_vrf_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    ret = pd::pd_vrf_delete(&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to delete vrf pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as vrf_delete_del_cb() was a succcess
//      a. Delete from vrf id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                    *vrf = NULL;
    hal_handle_t                hal_handle = 0;
    nwsec_profile_t             *sec_prof = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    vrf = (vrf_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-vrf:{}:delete commit CB {}",
                    __FUNCTION__, vrf->vrf_id);

    // Remove vrf references from other objects
    if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
        ret = nwsec_prof_del_vrf(sec_prof, vrf);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-vrf:{}:failed to del rel. from nwsec. prof",
                          __FUNCTION__);
            goto end;
        }

    }

    // a. Remove from vrf id hash table
    ret = vrf_del_from_db(vrf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to del vrf {} from db, err : {}", 
                      __FUNCTION__, vrf->vrf_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI vrf
    vrf_free(vrf);

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate vrf delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_vrf_delete (vrf_t *vrf)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of l2segs
    if (vrf->l2seg_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("pi-vrf:{}:l2segs still referring:", __FUNCTION__);
        hal_print_handles_block_list(vrf->l2seg_list);
    }

    return ret;
}

//------------------------------------------------------------------------------
// process a vrf delete request
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete (VrfDeleteRequest& req, VrfDeleteResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    vrf_t                    *vrf = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const VrfKeyHandle       &kh = req.key_or_handle();

    hal_api_trace(" API Begin: vrf delete ");

    // validate the request message
    ret = validate_vrf_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:vrf delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    vrf = vrf_lookup_key_or_handle(kh);
    if (vrf == NULL) {
        HAL_TRACE_ERR("pi-vrf:{}:failed to find vrf, id {}, handle {}",
                      __FUNCTION__, kh.vrf_id(), kh.vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("pi-vrf:{}:deleting vrf {}", 
                    __FUNCTION__, vrf->vrf_id);

    // validate if there no objects referring this sec. profile
    ret = validate_vrf_delete(vrf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-vrf:{}:vrf delete validation failed, "
                      "ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = vrf->hal_handle;
    dhl_entry.obj = vrf;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(vrf->hal_handle, &cfg_ctxt, 
                             vrf_delete_del_cb,
                             vrf_delete_commit_cb,
                             vrf_delete_abort_cb, 
                             vrf_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: vrf delete ");
    return ret;
}


//-----------------------------------------------------------------------------
// Adds l2seg into vrf list
//-----------------------------------------------------------------------------
hal_ret_t
vrf_add_l2seg (vrf_t *vrf, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;
    
    if (vrf == NULL || l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    vrf_lock(vrf, __FILENAME__, __LINE__, __func__);      // lock
    ret = vrf->l2seg_list->insert(&l2seg->hal_handle);
    vrf_unlock(vrf, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to insert into vrf's l2seg list. ret:{}",
                      __FUNCTION__, ret);
        goto end;
    }

end:
    HAL_TRACE_DEBUG("pi-vrf:{}:add vrf => l2seg, {} => {}, ret:{}",
                    __FUNCTION__, vrf ? vrf->vrf_id : 0, 
                    l2seg ? l2seg->seg_id : 0, ret);

    return ret;
}

//-----------------------------------------------------------------------------
// Remove l2seg from vrf list
//-----------------------------------------------------------------------------
hal_ret_t
vrf_del_l2seg (vrf_t *vrf, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (vrf == NULL || l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    vrf_lock(vrf, __FILENAME__, __LINE__, __func__);      // lock
    ret = vrf->l2seg_list->remove(&l2seg->hal_handle);
    vrf_unlock(vrf, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to remove from vrf's l2seg list. ret:{}",
                      __FUNCTION__, ret);
        goto end;
    }

    HAL_TRACE_DEBUG("pi-vrf:{}:del vrf =/=> l2seg, {} =/=> {}, ret:{}",
                    __FUNCTION__, vrf->vrf_id, l2seg->seg_id, ret);
end:
    return ret;
}

}    // namespace hal
