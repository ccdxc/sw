// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nwsec.hpp"
// #include "nic/hal/svc/nwsec_svc.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// hash table profile_id => entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
nwsec_profile_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    nwsec_profile_t             *nwsec = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nwsec = (nwsec_profile_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(nwsec->profile_id);
}

uint32_t
nwsec_profile_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return utils::hash_algo::fnv_hash(key,
                                      sizeof(nwsec_profile_id_t)) % ht_size;
}

bool
nwsec_profile_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(nwsec_profile_id_t *)key1 == *(nwsec_profile_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// Lookup l2seg from key or handle
//------------------------------------------------------------------------------
nwsec_profile_t *
nwsec_lookup_key_or_handle (const SecurityProfileKeyHandle& kh)
{
    nwsec_profile_t     *nwsec = NULL;

    if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileId) {
        nwsec = find_nwsec_profile_by_id(kh.profile_id());
    } else if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileHandle) {
        nwsec = find_nwsec_profile_by_handle(kh.profile_handle());
    }

    return nwsec;
}

//------------------------------------------------------------------------------
// insert a nwsecment to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nwsec_add_to_db (nwsec_profile_t *nwsec, hal_handle_t handle)
{
    hal_ret_t                   ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-nwsec:{}:adding to nwsec id hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from tenant id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from tenant id to its handle
    entry->handle_id = handle;
    ret = g_hal_state->nwsec_profile_id_ht()->
        insert_with_key(&nwsec->profile_id, entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to add nwsec id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    // TODO: Check if this is the right place
    nwsec->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a nwsec from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
nwsec_del_from_db (nwsec_profile_t *nwsec)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-nwsec:{}:removing from seg id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->nwsec_profile_id_ht()->
        remove(&nwsec->profile_id);

    // free up
    g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);

    return HAL_RET_OK;
}


#define NWSEC_SPEC_CHECK(fname) nwsec->fname != (uint32_t)spec.fname()
static inline hal_ret_t
nwsec_handle_update (SecurityProfileSpec& spec, nwsec_profile_t *nwsec, 
                     nwsec_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;

    if (NWSEC_SPEC_CHECK(cnxn_tracking_en) ||
            NWSEC_SPEC_CHECK(ipsg_en) ||
            NWSEC_SPEC_CHECK(tcp_rtt_estimate_en) ||
            NWSEC_SPEC_CHECK(session_idle_timeout) ||
            NWSEC_SPEC_CHECK(tcp_cnxn_setup_timeout) ||
            NWSEC_SPEC_CHECK(tcp_close_timeout) ||
            NWSEC_SPEC_CHECK(tcp_close_wait_timeout) ||
            NWSEC_SPEC_CHECK(ip_normalization_en) ||
            NWSEC_SPEC_CHECK(tcp_normalization_en) ||
            NWSEC_SPEC_CHECK(icmp_normalization_en) ||
            NWSEC_SPEC_CHECK(ip_ttl_change_detect_en) ||
            NWSEC_SPEC_CHECK(ip_rsvd_flags_action) ||
            NWSEC_SPEC_CHECK(ip_df_action) ||
            NWSEC_SPEC_CHECK(ip_options_action) ||
            NWSEC_SPEC_CHECK(ip_invalid_len_action) ||
            NWSEC_SPEC_CHECK(ip_normalize_ttl) ||
            NWSEC_SPEC_CHECK(icmp_invalid_code_action) ||
            NWSEC_SPEC_CHECK(icmp_deprecated_msgs_drop) ||
            NWSEC_SPEC_CHECK(icmp_redirect_msg_drop) ||
            NWSEC_SPEC_CHECK(tcp_non_syn_first_pkt_drop) ||
            NWSEC_SPEC_CHECK(tcp_syncookie_en) ||
            NWSEC_SPEC_CHECK(tcp_split_handshake_detect_en) ||
            NWSEC_SPEC_CHECK(tcp_split_handshake_drop) ||
            NWSEC_SPEC_CHECK(tcp_rsvd_flags_action) ||
            NWSEC_SPEC_CHECK(tcp_unexpected_mss_action) ||
            NWSEC_SPEC_CHECK(tcp_unexpected_win_scale_action) ||
            NWSEC_SPEC_CHECK(tcp_urg_ptr_not_set_action) ||
            NWSEC_SPEC_CHECK(tcp_urg_flag_not_set_action) ||
            NWSEC_SPEC_CHECK(tcp_urg_payload_missing_action) ||
            NWSEC_SPEC_CHECK(tcp_rst_with_data_action) ||
            NWSEC_SPEC_CHECK(tcp_data_len_gt_mss_action) ||
            NWSEC_SPEC_CHECK(tcp_data_len_gt_win_size_action) ||
            NWSEC_SPEC_CHECK(tcp_unexpected_ts_option_action) ||
            NWSEC_SPEC_CHECK(tcp_unexpected_echo_ts_action) ||
            NWSEC_SPEC_CHECK(tcp_ts_not_present_drop) ||
            NWSEC_SPEC_CHECK(tcp_invalid_flags_drop) ||
            NWSEC_SPEC_CHECK(tcp_nonsyn_noack_drop)) {
                app_ctxt->nwsec_changed = true;
            }

    if (NWSEC_SPEC_CHECK(ipsg_en)) {
        app_ctxt->ipsg_changed = true;
    }

    return ret;
}

#define NWSEC_SPEC_ASSIGN(fname) sec_prof->fname = spec.fname()
// initialize security profile object from the config spec
static inline void
nwsec_profile_init_from_spec (nwsec_profile_t *sec_prof,
                              nwsec::SecurityProfileSpec& spec)
{

    sec_prof->profile_id = spec.key_or_handle().profile_id();

    NWSEC_SPEC_ASSIGN(cnxn_tracking_en);
    NWSEC_SPEC_ASSIGN(ipsg_en);
    NWSEC_SPEC_ASSIGN(tcp_rtt_estimate_en);
    NWSEC_SPEC_ASSIGN(session_idle_timeout);
    NWSEC_SPEC_ASSIGN(tcp_cnxn_setup_timeout);
    NWSEC_SPEC_ASSIGN(tcp_close_timeout);
    NWSEC_SPEC_ASSIGN(tcp_close_wait_timeout);
    NWSEC_SPEC_ASSIGN(ip_normalization_en);
    NWSEC_SPEC_ASSIGN(tcp_normalization_en);
    NWSEC_SPEC_ASSIGN(icmp_normalization_en);
    NWSEC_SPEC_ASSIGN(ip_ttl_change_detect_en);
    NWSEC_SPEC_ASSIGN(ip_rsvd_flags_action);
    NWSEC_SPEC_ASSIGN(ip_df_action);
    NWSEC_SPEC_ASSIGN(ip_options_action);
    NWSEC_SPEC_ASSIGN(ip_invalid_len_action);
    NWSEC_SPEC_ASSIGN(ip_normalize_ttl);
    NWSEC_SPEC_ASSIGN(icmp_invalid_code_action);
    NWSEC_SPEC_ASSIGN(icmp_deprecated_msgs_drop);
    NWSEC_SPEC_ASSIGN(icmp_redirect_msg_drop);
    NWSEC_SPEC_ASSIGN(tcp_non_syn_first_pkt_drop);
    NWSEC_SPEC_ASSIGN(tcp_syncookie_en);
    NWSEC_SPEC_ASSIGN(tcp_split_handshake_detect_en);
    NWSEC_SPEC_ASSIGN(tcp_split_handshake_drop);
    NWSEC_SPEC_ASSIGN(tcp_rsvd_flags_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_mss_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_win_scale_action);
    NWSEC_SPEC_ASSIGN(tcp_urg_ptr_not_set_action);
    NWSEC_SPEC_ASSIGN(tcp_urg_flag_not_set_action);
    NWSEC_SPEC_ASSIGN(tcp_urg_payload_missing_action);
    NWSEC_SPEC_ASSIGN(tcp_rst_with_data_action);
    NWSEC_SPEC_ASSIGN(tcp_data_len_gt_mss_action);
    NWSEC_SPEC_ASSIGN(tcp_data_len_gt_win_size_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_ts_option_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_echo_ts_action);
    NWSEC_SPEC_ASSIGN(tcp_ts_not_present_drop);
    NWSEC_SPEC_ASSIGN(tcp_invalid_flags_drop);
    NWSEC_SPEC_ASSIGN(tcp_nonsyn_noack_drop);

    return;
}

static hal_ret_t
validate_nwsec_create(SecurityProfileSpec& spec, SecurityProfileResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-nwsec:{}:nwsec id and handle not set in request",
                      __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            SecurityProfileKeyHandle::kProfileId) {
        HAL_TRACE_ERR("pi-nwsec:{}:nwsec id not set in request",
                      __FUNCTION__);
        ret = HAL_RET_NWSEC_ID_INVALID;
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
nwsec_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_nwsec_profile_args_t     pd_nwsec_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    nwsec_profile_t                 *nwsec = NULL;
    // nwsec_create_app_ctxt_t         *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nwsec_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nwsec = (nwsec_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-nwsec:{}:create add CB {}",
                    __FUNCTION__, nwsec->profile_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_nwsec_profile_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    ret = pd::pd_nwsec_profile_create(&pd_nwsec_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to create nwsec pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as nwsec_create_add_cb() was a success
//      a. Create the flood list
//      b. Add to nwsec id hash table
//------------------------------------------------------------------------------
hal_ret_t
nwsec_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-nwsec:{}:create commit CB {}",
                    __FUNCTION__, nwsec->profile_id);


    // 1. a. Add to nwsec id hash table
    ret = nwsec_add_to_db(nwsec, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to add nwsec {} to db, err : {}", 
                __FUNCTION__, nwsec->profile_id, ret);
        goto end;
    }

    // TODO: Increment the ref counts of dependent objects
    //  - Have to increment ref count for tenant

end:
    return ret;
}

//------------------------------------------------------------------------------
// nwsec_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI tenant 
//------------------------------------------------------------------------------
hal_ret_t
nwsec_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_nwsec_profile_args_t     pd_nwsec_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    nwsec_profile_t                 *nwsec = NULL;
    hal_handle_t                    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-nwsec:{}:create abort CB {}",
                    __FUNCTION__, nwsec->profile_id);

    // 1. delete call to PD
    if (nwsec->pd) {
        pd::pd_nwsec_profile_args_init(&pd_nwsec_args);
        pd_nwsec_args.nwsec_profile = nwsec;
        ret = pd::pd_nwsec_profile_delete(&pd_nwsec_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-nwsec:{}:failed to delete nwsec pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI tenant
    nwsec_profile_free(nwsec);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
nwsec_prepare_rsp (SecurityProfileResponse *rsp, hal_ret_t ret, 
                   hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_profile_status()->set_profile_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

// create an instance of security profile
hal_ret_t
security_profile_create (nwsec::SecurityProfileSpec& spec,
                         nwsec::SecurityProfileResponse *rsp)
{
    hal_ret_t                       ret;
    nwsec_profile_t                 *sec_prof = NULL;
    nwsec_create_app_ctxt_t         app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("pi-nwsec:{}: creating nwsec profile, id {}", __FUNCTION__,
                    spec.key_or_handle().profile_id());

    // validate the request message
    ret = validate_nwsec_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("pi-nwsec:{}:validation failed. ret: {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // check if nwsec profile exists already, and reject if one is found
    if (find_nwsec_profile_by_id(spec.key_or_handle().profile_id())) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to create a nwsec, "
                      "nwsec{} exists already", __FUNCTION__, 
                      spec.key_or_handle().profile_id());
        ret =  HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate the nwsec profile
    sec_prof = nwsec_profile_alloc_init();
    if (sec_prof == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // consume the config
    nwsec_profile_init_from_spec(sec_prof, spec);

    // allocate hal handle id
    sec_prof->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_PROFILE);
    if (sec_prof->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-nwsec:{}: failed to alloc handle {}", 
                      __FUNCTION__, sec_prof->profile_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add. nothing to populate in app ctxt
    dhl_entry.handle = sec_prof->hal_handle;
    dhl_entry.obj = sec_prof;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(sec_prof->hal_handle, &cfg_ctxt, 
                             nwsec_create_add_cb,
                             nwsec_create_commit_cb,
                             nwsec_create_abort_cb, 
                             nwsec_create_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        if (sec_prof) {
            nwsec_profile_free(sec_prof);
            sec_prof = NULL;
        }
    }

    nwsec_prepare_rsp(rsp, ret, sec_prof ? sec_prof->hal_handle : 0);
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// validate nwsec update request
//------------------------------------------------------------------------------
hal_ret_t
validate_nwsec_update (SecurityProfileSpec & spec, SecurityProfileResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-nwsec:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update new l2seg properties
//------------------------------------------------------------------------------
hal_ret_t
nwsec_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_nwsec_profile_args_t     pd_nwsec_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    nwsec_profile_t                 *nwsec = NULL, *nwsec_clone = NULL;
    // nwsec_update_app_ctxt_t         *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nwsec_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    nwsec_clone = (nwsec_profile_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-nwsec:{}: update upd cb {}",
                    __FUNCTION__, nwsec->profile_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_nwsec_profile_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    pd_nwsec_args.clone_profile = nwsec_clone;
    ret = pd::pd_nwsec_profile_update(&pd_nwsec_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to update nwsec pd, err : {}",
                      __FUNCTION__, ret);
    }

    // TODO: If IPSG changes, it has to trigger to tenants.
end:
    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
//------------------------------------------------------------------------------
hal_ret_t
nwsec_make_clone (nwsec_profile_t *nwsec, nwsec_profile_t **nwsec_clone, 
                  SecurityProfileSpec& spec)
{
    *nwsec_clone = nwsec_profile_alloc_init();
    memcpy(*nwsec_clone, nwsec, sizeof(nwsec_profile_t));

    pd::pd_nwsec_profile_make_clone(nwsec, *nwsec_clone);


    // Keep new values in the clone
    nwsec_profile_init_from_spec(*nwsec_clone, spec);


    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD nwsec.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
nwsec_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_nwsec_profile_args_t pd_nwsec_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-nwsec:{}:update commit CB {}",
                    __FUNCTION__, nwsec->profile_id);

    // Free PD
    pd::pd_nwsec_profile_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    ret = pd::pd_nwsec_profile_mem_free(&pd_nwsec_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to delete nwsec pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    nwsec_profile_free(nwsec);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Free the clones
//------------------------------------------------------------------------------
hal_ret_t
nwsec_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_nwsec_profile_args_t pd_nwsec_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // assign clone as we are trying to free only the clone
    nwsec = (nwsec_profile_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-nwsec:{}:update commit CB {}",
                    __FUNCTION__, nwsec->profile_id);

    // Free PD
    pd::pd_nwsec_profile_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    ret = pd::pd_nwsec_profile_mem_free(&pd_nwsec_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to delete nwsec pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    nwsec_profile_free(nwsec);
end:

    return ret;
}

hal_ret_t
nwsec_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

// update a security profile instance
hal_ret_t
security_profile_update (nwsec::SecurityProfileSpec& spec,
                         nwsec::SecurityProfileResponse *rsp)
{
    hal_ret_t                      ret;
    nwsec_profile_t                *sec_prof;
    // nwsec_profile_t                local_sec_prof;
    cfg_op_ctxt_t                  cfg_ctxt = { 0 };
    dhl_entry_t                    dhl_entry = { 0 };
    nwsec_update_app_ctxt_t        app_ctxt = { 0 };
    const SecurityProfileKeyHandle  &kh = spec.key_or_handle();

    HAL_TRACE_DEBUG("---------------- Sec. Profile Update API Start ---------");
    HAL_TRACE_DEBUG("pi-nwsec:{}: nwsec update for id {} handle{}",
                    __FUNCTION__,
                    spec.key_or_handle().profile_id(),
                    spec.key_or_handle().profile_handle());

    // validate the request message
    ret = validate_nwsec_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:nwsec update validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    // lookup this security profile
    sec_prof = nwsec_lookup_key_or_handle(spec.key_or_handle());
    if (!sec_prof) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to find nwsec, id {}, handle {}",
                      __FUNCTION__, kh.profile_id(), kh.profile_handle());
        ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        goto end;
    }

    ret = nwsec_handle_update(spec, sec_prof, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:nwsec check update failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    if (!app_ctxt.nwsec_changed) {
        HAL_TRACE_ERR("pi-nwsec:{}:no change in nwsec update: noop", __FUNCTION__);
        goto end;
    }

    nwsec_make_clone(sec_prof, (nwsec_profile_t **)&dhl_entry.cloned_obj, spec);

    // form ctxt and call infra update object
    dhl_entry.handle = sec_prof->hal_handle;
    dhl_entry.obj = sec_prof;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(sec_prof->hal_handle, &cfg_ctxt, 
                             nwsec_update_upd_cb,
                             nwsec_update_commit_cb,
                             nwsec_update_abort_cb, 
                             nwsec_update_cleanup_cb);

end:
    nwsec_prepare_rsp(rsp, ret, sec_prof ? sec_prof->hal_handle : 0);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;


#if 0
    // Calling PD update
    pd::pd_nwsec_profile_args_init(&pd_nwsec_profile_args);
    pd_nwsec_profile_args.nwsec_profile = &local_sec_prof;
    ret = pd::pd_nwsec_profile_update(&pd_nwsec_profile_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD security profile update failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    } else {
        // Success: Update the store PI object 
        nwsec_profile_init_from_spec(sec_prof, spec);
    }

end:

    HAL_TRACE_DEBUG("PI-Nwsec:{}: Nwsec Update for id {} handle {} ret{}", __FUNCTION__, 
                    spec.key_or_handle().profile_id(), spec.key_or_handle().profile_handle(), ret);
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
#endif
}

//------------------------------------------------------------------------------
// validate nwsec delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_nwsec_delete (SecurityProfileDeleteRequest& req, 
                       SecurityProfileDeleteResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-nwsec:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_nwsec_profile_args_t pd_nwsec_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the nwsec. 
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-nwsec:{}:delete del CB {}",
                    __FUNCTION__, nwsec->profile_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_nwsec_profile_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    ret = pd::pd_nwsec_profile_delete(&pd_nwsec_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to delete nwsec pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as tenant_delete_del_cb() was a succcess
//      a. Delete from tenant id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI tenant
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t                     *nwsec = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-nwsec:{}:delete commit CB {}",
                    __FUNCTION__, nwsec->profile_id);

    // a. Remove from nwsec id hash table
    ret = nwsec_del_from_db(nwsec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to del nwsec {} from db, err : {}", 
                      __FUNCTION__, nwsec->profile_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI nwsec
    nwsec_profile_free(nwsec);

    // TODO: Decrement the ref counts of dependent objects
    //  - Have to decrement ref count for nwsec profile

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a nwsec delete request
//------------------------------------------------------------------------------
hal_ret_t
security_profile_delete (SecurityProfileDeleteRequest& req, 
                         SecurityProfileDeleteResponseMsg *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    nwsec_profile_t                 *nwsec = NULL;
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };
    const SecurityProfileKeyHandle  &kh = req.key_or_handle();

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

    // validate the request message
    ret = validate_nwsec_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:nwsec delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }


    nwsec = nwsec_lookup_key_or_handle(kh);
    if (nwsec == NULL) {
        HAL_TRACE_ERR("pi-nwsec:{}:failed to find nwsec, id {}, handle {}",
                      __FUNCTION__, kh.profile_id(), kh.profile_handle());
        ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-nwsec:{}:deleting nwsec profile {}", 
                    __FUNCTION__, nwsec->profile_id);

    // form ctxt and call infra add
    dhl_entry.handle = nwsec->hal_handle;
    dhl_entry.obj = nwsec;
    cfg_ctxt.app_ctxt = NULL;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(nwsec->hal_handle, &cfg_ctxt, 
                             nwsec_delete_del_cb,
                             nwsec_delete_commit_cb,
                             nwsec_delete_abort_cb, 
                             nwsec_delete_cleanup_cb);

end:
    rsp->add_api_status(hal_prepare_rsp(ret));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
security_profile_get (nwsec::SecurityProfileGetRequest& req,
                      nwsec::SecurityProfileGetResponse *rsp)
{
    nwsec::SecurityProfileSpec    *spec;
    nwsec_profile_t               *sec_prof;

    // key or handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // lookup this security profile
    sec_prof = nwsec_lookup_key_or_handle(req.key_or_handle());
    if (!sec_prof) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this profile
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_profile_id(sec_prof->profile_id);
    spec->set_cnxn_tracking_en(sec_prof->cnxn_tracking_en);
    spec->set_ipsg_en(sec_prof->ipsg_en);
    spec->set_tcp_rtt_estimate_en(sec_prof->tcp_rtt_estimate_en);
    spec->set_session_idle_timeout(sec_prof->session_idle_timeout);
    spec->set_tcp_cnxn_setup_timeout(sec_prof->tcp_cnxn_setup_timeout);
    spec->set_tcp_close_timeout(sec_prof->tcp_close_timeout);
    spec->set_tcp_close_wait_timeout(sec_prof->tcp_close_wait_timeout);

    spec->set_ip_normalization_en(sec_prof->ip_normalization_en);
    spec->set_tcp_normalization_en(sec_prof->tcp_normalization_en);
    spec->set_icmp_normalization_en(sec_prof->icmp_normalization_en);

    spec->set_ip_ttl_change_detect_en(sec_prof->ip_ttl_change_detect_en);
    spec->set_ip_rsvd_flags_action(
             static_cast<nwsec::NormalizationAction>(sec_prof->ip_rsvd_flags_action));
    spec->set_ip_df_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_df_action));
    spec->set_ip_options_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_options_action));
    spec->set_ip_invalid_len_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_invalid_len_action));
    spec->set_ip_normalize_ttl(sec_prof->ip_normalize_ttl);

    spec->set_icmp_invalid_code_action(
          static_cast<nwsec::NormalizationAction>(sec_prof->icmp_invalid_code_action));
    spec->set_icmp_deprecated_msgs_drop(sec_prof->icmp_deprecated_msgs_drop);
    spec->set_icmp_redirect_msg_drop(sec_prof->icmp_redirect_msg_drop);

    spec->set_tcp_non_syn_first_pkt_drop(sec_prof->tcp_non_syn_first_pkt_drop);
    spec->set_tcp_syncookie_en(sec_prof->tcp_syncookie_en);
    spec->set_tcp_split_handshake_detect_en(sec_prof->tcp_split_handshake_detect_en);
    spec->set_tcp_split_handshake_drop(sec_prof->tcp_split_handshake_drop);
    spec->set_tcp_rsvd_flags_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rsvd_flags_action));
    spec->set_tcp_unexpected_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_mss_action));
    spec->set_tcp_unexpected_win_scale_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_win_scale_action));
    spec->set_tcp_urg_ptr_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_ptr_not_set_action));
    spec->set_tcp_urg_flag_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_flag_not_set_action));
    spec->set_tcp_urg_payload_missing_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_payload_missing_action));
    spec->set_tcp_rst_with_data_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rst_with_data_action));
    spec->set_tcp_data_len_gt_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_mss_action));
    spec->set_tcp_data_len_gt_win_size_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_win_size_action));
    spec->set_tcp_unexpected_ts_option_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_ts_option_action));
    spec->set_tcp_unexpected_echo_ts_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_echo_ts_action));
    spec->set_tcp_ts_not_present_drop(sec_prof->tcp_ts_not_present_drop);
    spec->set_tcp_invalid_flags_drop(sec_prof->tcp_invalid_flags_drop);
    spec->set_tcp_nonsyn_noack_drop(sec_prof->tcp_nonsyn_noack_drop);

    // fill operational state of this profile
    rsp->mutable_status()->set_profile_handle(sec_prof->hal_handle);

    // fill stats, if any, of this profile

    return HAL_RET_OK;
}

}    // namespace hal
