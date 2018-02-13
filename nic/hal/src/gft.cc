// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/gft.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

//----------------------------------------------------------------------------
// get GFT exact match profile's key
//----------------------------------------------------------------------------
void *
gft_exact_match_profile_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t     *ht_entry;
    gft_exact_match_profile_t    *profile = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    profile =
        (gft_exact_match_profile_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(profile->profile_id);
}

//----------------------------------------------------------------------------
// compute the hash given GFT exact match profile's key (i.e., profile id)
//----------------------------------------------------------------------------
uint32_t
gft_exact_match_profile_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(gft_profile_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// compare the keys of GFT exact match profiles
//----------------------------------------------------------------------------
bool
gft_exact_match_profile_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(gft_profile_id_t *)key1 == *(gft_profile_id_t *)key2) {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// get GFT header transposition profile's key
//----------------------------------------------------------------------------
void *
gft_hdr_transposition_profile_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t       *ht_entry;
    gft_hdr_xposition_profile_t    *profile = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    profile =
        (gft_hdr_xposition_profile_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(profile->profile_id);
}

//-----------------------------------------------------------------------------
// compute hash given GFT header transposition profile's key (i.e., profile id)
//-----------------------------------------------------------------------------
uint32_t
gft_hdr_transposition_profile_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(gft_profile_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// compare the keys of GFT header transposition profiles
//----------------------------------------------------------------------------
bool
gft_hdr_transposition_profile_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(gft_profile_id_t *)key1 == *(gft_profile_id_t *)key2) {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// get GFT exact match flow entry's key
//----------------------------------------------------------------------------
void *
gft_exact_match_flow_entry_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t        *ht_entry;
    gft_exact_match_flow_entry_t    *flow_entry = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    flow_entry =
        (gft_exact_match_flow_entry_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(flow_entry->flow_entry_id);
}

//------------------------------------------------------------------------------
// compute the hash given GFT exact match flow entry's key (i.e., flow entry id)
//------------------------------------------------------------------------------
uint32_t
gft_exact_match_flow_entry_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(gft_flow_entry_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// compare the keys of GFT exact match flow entries
//----------------------------------------------------------------------------
bool
gft_exact_match_flow_entry_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(gft_flow_entry_id_t *)key1 == *(gft_flow_entry_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a GFT exact match profile to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emp_add_to_db (gft_exact_match_profile_t *profile, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding GFT exact match profile {} to db",
                    profile->profile_id);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from profile id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->gft_exact_match_profile_id_ht()->
                  insert_with_key(&profile->profile_id, entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add GFT exact match profile {} to db, "
                      "err : {}", profile->profile_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a nwsec from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emp_del_from_db (gft_exact_match_profile_t *profile)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing GFT exact match profile {} from db",
                    profile->profile_id);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->gft_exact_match_profile_id_ht()->remove(&profile->profile_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate a given GFT exact match profile config spec
//------------------------------------------------------------------------------
static hal_ret_t
validate_gft_emp_create (GftExactMatchProfileSpec& spec,
                         GftExactMatchProfileResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize GFT exact match profile object from the config spec
//------------------------------------------------------------------------------
static inline void
gft_emp_init_from_spec (gft_exact_match_profile_t *profile,
                        GftExactMatchProfileSpec& spec)
{
}

//------------------------------------------------------------------------------
// lookup GFT exact match profile from key or handle
//------------------------------------------------------------------------------
static gft_exact_match_profile_t *
gft_emp_lookup_key_or_handle (const GftExactMatchProfileKeyHandle& kh)
{
    gft_exact_match_profile_t    *profile = NULL;

    if (kh.key_or_handle_case() == GftExactMatchProfileKeyHandle::kProfileId) {
        profile = find_gft_exact_match_profile_by_id(kh.profile_id());
    } else if (kh.key_or_handle_case() ==
               GftExactMatchProfileKeyHandle::kProfileHandle) {
        profile = find_gft_exact_match_profile_by_handle(kh.profile_handle());
    }

    return profile;
}

//------------------------------------------------------------------------------
// pd call to allocate pd resources and h/w programming
//------------------------------------------------------------------------------
static hal_ret_t
gft_emp_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                                    ret = HAL_RET_OK;
    //pd::pd_gft_args_t                            pd_gft_args = { 0 };
    dllist_ctxt_t                                *lnode = NULL;
    dhl_entry_t                                  *dhl_entry = NULL;
    gft_exact_match_profile_t                    *profile = NULL;
    //gft_exact_match_profile_create_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Failed to create GFT exact match profile");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    //app_ctxt = (gft_exact_match_profile_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    profile = (gft_exact_match_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("GFT exact match profile create add CB {}",
                    profile->profile_id);

    // PD Call to allocate PD resources and h/w programming, if any
    //pd::pd_gft_exact_match_profile_create_args_init(&pd_gft_args);
    //pd_l2seg_args.l2seg = l2seg;
    //pd_l2seg_args.vrf = app_ctxt->vrf;
    //ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_CREATE, (void *)&pd_l2seg_args);
    //if (ret != HAL_RET_OK) {
        //HAL_TRACE_ERR("{}:failed to create l2seg pd, err : {}",
                      //__FUNCTION__, ret);
    //}

end:

    return ret;
}

//------------------------------------------------------------------------------
// update PI databases as gft_emp_create_add_cb was a success
//------------------------------------------------------------------------------
static hal_ret_t
gft_emp_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// gft_emp_create_add_cb was a failure, do the cleanup
//------------------------------------------------------------------------------
hal_ret_t
gft_emp_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dummy create cleanup callback
//-----------------------------------------------------------------------------
static hal_ret_t
gft_emp_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// convert hal_ret_t to API status and prepare response to the API call
//------------------------------------------------------------------------------
static hal_ret_t
gft_emp_prepare_rsp (GftExactMatchProfileResponse *rsp,
                     gft_exact_match_profile_t *profile,
                     hal_ret_t ret)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_profile_handle(profile->hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a GFT exact match profile create request
//------------------------------------------------------------------------------
hal_ret_t
gft_exact_match_profile_create (GftExactMatchProfileSpec& spec,
                                GftExactMatchProfileResponse *rsp)
{
    hal_ret_t                    ret;
    gft_exact_match_profile_t    *profile;
    gft_emp_create_app_ctxt_t    app_ctxt; // = { 0 };
    cfg_op_ctxt_t                cfg_ctxt = { 0 };
    dhl_entry_t                  dhl_entry = { 0 };

    HAL_TRACE_DEBUG("GFT exact match profile create");
    // validate the request message
    ret = validate_gft_emp_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("GFT exact match profile object validation failure, "
                      "err : {}", ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Creating GFT exact match profile id {}",
                    spec.key_or_handle().profile_id());

    // check if this profile exists already, and reject if one is found
    profile = gft_emp_lookup_key_or_handle(spec.key_or_handle());
    if (profile) {
        HAL_TRACE_ERR("Failed to create GFT exact match profile, "
                      "profile {} exists already",
                      spec.key_or_handle().profile_id());
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate the gft exact match profile
    profile = gft_exact_match_profile_alloc_init();
    if (profile == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("Failed to allocate GFT exact match profile, err : {}",
                      ret);
        goto end;
    }

    // consume the config
    gft_emp_init_from_spec(profile, spec);

    // allocate hal handle id
    profile->hal_handle = hal_handle_alloc(HAL_OBJ_ID_GFT_EXACT_MATCH_PROFILE);
    if (profile->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for GFT exact match profile {}",
                      profile->profile_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = profile->hal_handle;
    dhl_entry.obj = profile;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(profile->hal_handle, &cfg_ctxt,
                             gft_emp_create_add_cb,
                             gft_emp_create_commit_cb,
                             gft_emp_create_abort_cb,
                             gft_emp_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
        if (profile) {
            profile = NULL;
        }
    }
    gft_emp_prepare_rsp(rsp, profile, ret);
    return ret;
}

//------------------------------------------------------------------------------
// insert a GFT header transposition profile to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_htp_add_to_db (gft_hdr_xposition_profile_t *profile, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding GFT header transposition profile {} to db",
                    profile->profile_id);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from profile id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->gft_hdr_transposition_profile_id_ht()->
                  insert_with_key(&profile->profile_id, entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add GFT header transposition profile {} to db, "
                      "err : {}", profile->profile_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a GFT header transposition profile from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_htp_del_from_db (gft_hdr_xposition_profile_t *profile)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing GFT header transposition profile {} from db",
                    profile->profile_id);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->gft_hdr_transposition_profile_id_ht()->
                    remove(&profile->profile_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate a given GFT header transposition profile config spec
//------------------------------------------------------------------------------
static hal_ret_t
validate_gft_htp_create (GftHeaderTranspositionProfileSpec& spec,
                         GftHeaderTranspositionProfileResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize GFT header transposition profile object from the config spec
//------------------------------------------------------------------------------
static inline void
gft_htp_init_from_spec (gft_hdr_xposition_profile_t *profile,
                        GftHeaderTranspositionProfileSpec& spec)
{
}

//------------------------------------------------------------------------------
// lookup GFT header transposition profile from key or handle
//------------------------------------------------------------------------------
static gft_hdr_xposition_profile_t *
gft_htp_lookup_key_or_handle (const GftHeaderTranspositionProfileKeyHandle& kh)
{
    gft_hdr_xposition_profile_t    *profile = NULL;

    if (kh.key_or_handle_case() == GftHeaderTranspositionProfileKeyHandle::kProfileId) {
        profile = find_gft_hdr_xposition_profile_by_id(kh.profile_id());
    } else if (kh.key_or_handle_case() ==
               GftHeaderTranspositionProfileKeyHandle::kProfileHandle) {
        profile = find_gft_hdr_xposition_profile_by_handle(kh.profile_handle());
    }

    return profile;
}

//------------------------------------------------------------------------------
// pd call to allocate pd resources and h/w programming
//------------------------------------------------------------------------------
static hal_ret_t
gft_htp_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                      ret = HAL_RET_OK;
    //pd::pd_gft_args_t            pd_gft_args = { 0 };
    dllist_ctxt_t                  *lnode = NULL;
    dhl_entry_t                    *dhl_entry = NULL;
    gft_hdr_xposition_profile_t    *profile = NULL;
    //gft_htp_create_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Failed to create GFT hdr transposition profile");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    //app_ctxt = (//gft_htp_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    profile = (gft_hdr_xposition_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("GFT hdr transposition profile create add CB {}",
                    profile->profile_id);

    // PD Call to allocate PD resources and h/w programming, if any
    //pd::pd_gft_exact_match_profile_create_args_init(&pd_gft_args);
    //pd_l2seg_args.l2seg = l2seg;
    //pd_l2seg_args.vrf = app_ctxt->vrf;
    //ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_CREATE, (void *)&pd_l2seg_args);
    //if (ret != HAL_RET_OK) {
        //HAL_TRACE_ERR("{}:failed to create l2seg pd, err : {}",
                      //__FUNCTION__, ret);
    //}

end:

    return ret;
}

//------------------------------------------------------------------------------
// update PI databases as gft_htp_create_add_cb was a success
//------------------------------------------------------------------------------
static hal_ret_t
gft_htp_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// gft_htp_create_add_cb was a failure, do the cleanup
//------------------------------------------------------------------------------
hal_ret_t
gft_htp_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dummy create cleanup callback
//-----------------------------------------------------------------------------
static hal_ret_t
gft_htp_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// convert hal_ret_t to API status and prepare response to the API call
//------------------------------------------------------------------------------
static hal_ret_t
gft_htp_prepare_rsp (GftHeaderTranspositionProfileResponse *rsp,
                     gft_hdr_xposition_profile_t *profile,
                     hal_ret_t ret)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_profile_handle(profile->hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a GFT header transposition profile create request
//------------------------------------------------------------------------------
hal_ret_t
gft_header_transposition_profile_create (
    GftHeaderTranspositionProfileSpec &spec,
    GftHeaderTranspositionProfileResponse *rsp)
{
    hal_ret_t                      ret;
    gft_hdr_xposition_profile_t    *profile;
    gft_htp_create_app_ctxt_t      app_ctxt; // = { 0 };
    cfg_op_ctxt_t                  cfg_ctxt = { 0 };
    dhl_entry_t                    dhl_entry = { 0 };

    HAL_TRACE_DEBUG("GFT header transposition profile create");
    // validate the request message
    ret = validate_gft_htp_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("GFT header transposition profile object validation failure, "
                      "err : {}", ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Creating GFT header transposition id {}",
                    spec.key_or_handle().profile_id());

    // check if this profile exists already, and reject if one is found
    profile = gft_htp_lookup_key_or_handle(spec.key_or_handle());
    if (profile) {
        HAL_TRACE_ERR("Failed to create GFT header transposition profile, "
                      "profile {} exists already",
                      spec.key_or_handle().profile_id());
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate the gft header transposition profile
    profile = gft_hdr_transposition_profile_alloc_init();
    if (profile == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("Failed to allocate GFT header transposition profile, err : {}",
                      ret);
        goto end;
    }

    // consume the config
    gft_htp_init_from_spec(profile, spec);

    // allocate hal handle id
    profile->hal_handle = hal_handle_alloc(HAL_OBJ_ID_GFT_HDR_TRANSPOSITION_PROFILE);
    if (profile->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for GFT header transposition profile {}",
                      profile->profile_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = profile->hal_handle;
    dhl_entry.obj = profile;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(profile->hal_handle, &cfg_ctxt,
                             gft_htp_create_add_cb,
                             gft_htp_create_commit_cb,
                             gft_htp_create_abort_cb,
                             gft_htp_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
        if (profile) {
            profile = NULL;
        }
    }
    gft_htp_prepare_rsp(rsp, profile, ret);
    return ret;
}

//------------------------------------------------------------------------------
// process a GFT exact match flow entry create request
//------------------------------------------------------------------------------
hal_ret_t
gft_exact_match_flow_entry_create (GftExactMatchFlowEntrySpec &spec,
                                   GftExactMatchFlowEntryResponse *rsp)
{
    return HAL_RET_OK;
}

}    // namespace hal
