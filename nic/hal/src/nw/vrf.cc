//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for VRFs
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/if_utils.hpp"

namespace hal {

//------------------------------------------------------------------------------
// hash table vrf_id => entry
//  - Get key from entry
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// hash table vrf_id => entry - compute hash
//------------------------------------------------------------------------------
uint32_t
vrf_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(vrf_id_t)) % ht_size;
}

//------------------------------------------------------------------------------
// hash table vrf_id => entry - compare function
//------------------------------------------------------------------------------
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
// allocate a vrf instance
//------------------------------------------------------------------------------
static inline vrf_t *
vrf_alloc (void)
{
    vrf_t    *vrf;

    vrf = (vrf_t *)g_hal_state->vrf_slab()->alloc();
    if (vrf == NULL) {
        return NULL;
    }
    return vrf;
}

//------------------------------------------------------------------------------
// initialize a vrf instance
//------------------------------------------------------------------------------
static inline vrf_t *
vrf_init (vrf_t *vrf)
{
    if (!vrf) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&vrf->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    vrf->hal_handle   = HAL_HANDLE_INVALID;
    vrf->num_l2seg    = 0;
    vrf->num_sg       = 0;
    vrf->num_l4lb_svc = 0;
    vrf->num_ep       = 0;
    vrf->pd           = NULL;

    // initialize meta information
    vrf->l2seg_list = block_list::factory(sizeof(hal_handle_t));
    vrf->acl_list = block_list::factory(sizeof(hal_handle_t));
    // utils::dllist_reset(&vrf->l2seg_list_head);
    // utils::dllist_reset(&vrf->ep_list_head);
    // utils::dllist_reset(&vrf->session_list_head);

    return vrf;
}

//------------------------------------------------------------------------------
// allocate and initialize a vrf instance
//------------------------------------------------------------------------------
static inline vrf_t *
vrf_alloc_init (void)
{
    return vrf_init(vrf_alloc());
}

// free vrf instance
// Note: This is not a deep free wherein the list or other pointers have to
//       be freed separately
static inline hal_ret_t
vrf_free (vrf_t *vrf)
{
    HAL_SPINLOCK_DESTROY(&vrf->slock);
    hal::delay_delete_to_slab(HAL_SLAB_VRF, vrf);
    return HAL_RET_OK;
}

// anti vrf_alloc_init
static inline hal_ret_t
vrf_cleanup (vrf_t *vrf)
{
    if (vrf->l2seg_list) {
        block_list::destroy(vrf->l2seg_list);
    }
    if (vrf->acl_list) {
        block_list::destroy(vrf->acl_list);
    }
    vrf_free(vrf);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// find a vrf instance by its id
//------------------------------------------------------------------------------
vrf_t *
vrf_lookup_by_id (vrf_id_t tid)
{
    hal_handle_id_ht_entry_t    *entry;
    vrf_t                    *vrf;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->vrf_id_ht()->lookup(&tid);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {

        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                HAL_OBJ_ID_VRF);

        vrf = (vrf_t *)hal_handle_get_obj(entry->handle_id);
        return vrf;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// insert a vrf to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
vrf_add_to_db (vrf_t *vrf, hal_handle_t handle, nwsec_profile_t *sec_prof)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding to vrf id hash table");
    // allocate an entry to establish mapping from vrf id to its handle
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->vrf_id_ht()->insert_with_key(&vrf->vrf_id,
                                                        entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add vrf id to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // add vrf to nwsec profile
    if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        ret = nwsec_prof_add_vrf(sec_prof, vrf);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add vrf {} to nwsec prof {}, err : {}",
                          vrf->vrf_id, vrf->nwsec_profile_handle, ret);
            goto end;
        }
    }

    // statsh this in hal_state, if this is infra VRF
    if (vrf->vrf_type == types::VRF_TYPE_INFRA) {
        g_hal_state->set_infra_vrf_handle(handle);
        g_hal_state->oper_db()->set_mytep_ip(&vrf->mytep_ip);
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// delete a vrf from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
vrf_del_from_db (vrf_t *vrf)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing from vrf id hash table");

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->vrf_id_ht()->
        remove(&vrf->vrf_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// print vrf spec
//-----------------------------------------------------------------------------
static hal_ret_t
vrf_spec_dump (VrfSpec& spec)
{
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;
    ip_addr_t           my_tep;
    ip_prefix_t         gipo_pfx;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return HAL_RET_OK;
    }

    buf.write("Vrf Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() == VrfKeyHandle::kVrfId) {
            buf.write("vrf_id : {}, ", kh.vrf_id());
        } else if (kh.key_or_handle_case() == VrfKeyHandle::kVrfHandle) {
            buf.write("vrf_hdl : {}, ", kh.vrf_handle());
        }
    } else {
        buf.write("vrf_id_hdl : NULL, ");
    }

    if (spec.has_security_key_handle()) {
        auto kh = spec.security_key_handle();
        if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileId) {
            buf.write("sec_prof_id : {}, ", kh.profile_id());
        } else if (kh.key_or_handle_case() ==
                       SecurityProfileKeyHandle::kProfileHandle) {
            buf.write("sec_prof_hdl : {}, ", kh.profile_handle());
        }
    } else {
        buf.write("sec_pro_id_hdl : NULL, ");
    }

    buf.write("type : {}, ",
              (spec.vrf_type() == types::VrfType::VRF_TYPE_NONE) ? "none" :
              (spec.vrf_type() == types::VrfType::VRF_TYPE_INFRA) ? "infra" :
              "customer");

    if (spec.has_mytep_ip()) {
        ip_addr_spec_to_ip_addr(&my_tep, spec.mytep_ip());
        buf.write("my_tep : {}", ipaddr2str(&my_tep));
    }
    if (spec.has_gipo_prefix()) {
        ret = ip_pfx_spec_to_pfx_spec(&gipo_pfx, spec.gipo_prefix());
        if (ret == HAL_RET_OK) {
            buf.write("gipo_pfx : {}/{}", ipaddr2str(&gipo_pfx.addr),
                      gipo_pfx.len);
        }
    }
    HAL_TRACE_DEBUG("{}", buf.c_str());

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
    hal_handle_t infra_vrf_handle = HAL_HANDLE_INVALID;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("vrf id and handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != VrfKeyHandle::kVrfId) {
        // key-handle field set, but vrf id not provided
        HAL_TRACE_ERR("vrf id not set in request");
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // check if vrf id is in the valid range
    if (kh.vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("Invalid vrf id {} in the request",
                      HAL_VRF_ID_INVALID);
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    infra_vrf_handle = g_hal_state->infra_vrf_handle();
    if ((spec.vrf_type() == types::VRF_TYPE_INFRA) &&
        (infra_vrf_handle != HAL_HANDLE_INVALID)) {
        HAL_TRACE_ERR("Infra VRF already exists with handle {}",
                      infra_vrf_handle);
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_INVALID_ARG;
    }
#if 0
    infra_vrf = (vrf_t *)g_hal_state->infra_vrf();
    if ((spec.vrf_type() == types::VRF_TYPE_INFRA) &&
        (infra_vrf != NULL)) {
        HAL_TRACE_ERR("Infra VRF already exists vrf id: {}",
                      infra_vrf->vrf_id);
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_INVALID_ARG;
    }
#endif

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
vrf_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_vrf_create_args_t    pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    vrf_t                       *vrf = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    vrf = (vrf_t *)dhl_entry->obj;

    // PD Call to allocate PD resources and HW programming
    pd::pd_vrf_create_args_init(&pd_vrf_args);
    pd_vrf_args.vrf           = vrf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_CREATE, (void *)&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create vrf {} pd, err : {}",
                      vrf->vrf_id, ret);
    }
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

    HAL_ASSERT(cfg_ctxt != NULL);
    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (vrf_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    // add to vrf id hash table
    ret = vrf_add_to_db(vrf, hal_handle, app_ctxt->sec_prof);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add vrf {} to db, err : {}", vrf->vrf_id, ret);
    }
    return ret;
}

//------------------------------------------------------------------------------
// helper function to cleanup all the vrf related state during abort operation
// when create failed
//------------------------------------------------------------------------------
static hal_ret_t
vrf_create_abort_cleanup (vrf_t *vrf, hal_handle_t hal_handle)
{
    hal_ret_t                   ret;
    pd::pd_vrf_delete_args_t    pd_vrf_args = { 0 };

    // 1. delete call to PD
    if (vrf->pd) {
        pd::pd_vrf_delete_args_init(&pd_vrf_args);
        pd_vrf_args.vrf = vrf;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_DELETE, (void *)&pd_vrf_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete vrf {} pd, err : {}", vrf->vrf_id, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. free vrf
    vrf_cleanup(vrf);

    return HAL_RET_OK;
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
    hal_ret_t ret                        = HAL_RET_OK;
    dllist_ctxt_t *lnode                 = NULL;
    dhl_entry_t *dhl_entry               = NULL;
    vrf_t *vrf                           = NULL;
    hal_handle_t hal_handle              = 0;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    vrf = (vrf_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("VRF {} create abort cb", vrf->vrf_id);
    ret = vrf_create_abort_cleanup(vrf, hal_handle);

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
    if ((ret == HAL_RET_OK) || (ret == HAL_RET_ENTRY_EXISTS)) {
        rsp->mutable_vrf_status()->set_vrf_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a vrf object from its spec
//------------------------------------------------------------------------------
static hal_ret_t
vrf_init_from_spec (vrf_t *vrf, const VrfSpec& spec)
{
    hal_ret_t          ret;
    nwsec_profile_t    *sec_prof;

    vrf->vrf_type             = spec.vrf_type();
    vrf->vrf_id               = spec.key_or_handle().vrf_id();
    ret = find_nwsec_by_key_or_handle(spec.security_key_handle(), &sec_prof);
    if (sec_prof) {
        vrf->nwsec_profile_handle = sec_prof->hal_handle;
    } else if (ret == HAL_RET_KEY_HANDLE_NOT_SPECIFIED) {
        HAL_TRACE_DEBUG("No nwsec prof passed, "
                        "using default security profile");
        vrf->nwsec_profile_handle = HAL_HANDLE_INVALID;
    } else {
        // either invalid key or handle
        return HAL_RET_SECURITY_PROFILE_NOT_FOUND;
    }

    if (vrf->vrf_type == types::VRF_TYPE_INFRA) {
        if (spec.has_mytep_ip()) {
            ret = ip_addr_spec_to_ip_addr(&vrf->mytep_ip, spec.mytep_ip());
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Invalid mytep address in VRF {} spec, err : {}",
                               vrf->vrf_id, ret);
                return ret;
            }
        }
        if (spec.has_gipo_prefix()) {
            ret = ip_pfx_spec_to_pfx_spec(&vrf->gipo_prefix,
                                          spec.gipo_prefix());
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Invalid GIPo prefix in VRF {} spec, err : {}",
                               vrf->vrf_id, ret);
                return ret;
            }
        }
        HAL_TRACE_DEBUG("Local VTEP : {}, GIPo Prefix : {}/{}",
                        ipaddr2str(&vrf->mytep_ip),
                        ipaddr2str(&vrf->gipo_prefix.addr),
                        vrf->gipo_prefix.len);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a vrf's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
vrf_init_from_status (vrf_t *vrf, const VrfStatus& status)
{
    vrf->hal_handle = status.vrf_handle();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a vrf's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
vrf_init_from_stats (vrf_t *vrf, const VrfStats& stats)
{
    vrf->num_l2seg = stats.num_l2_segments();
    vrf->num_sg = stats.num_security_groups();
    vrf->num_l4lb_svc = stats.num_l4lb_services();
    vrf->num_ep = stats.num_endpoints();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a vrf create request
// TODO: if vrf exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
vrf_create (VrfSpec& spec, VrfResponse *rsp)
{
    hal_ret_t                   ret;
    vrf_t                       *vrf;
    nwsec_profile_t             *sec_prof = NULL;
    vrf_create_app_ctxt_t       app_ctxt  = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt  = { 0 };

    HAL_TRACE_DEBUG("Creating vrf with id {}", spec.key_or_handle().vrf_id());

    // dump spec
    vrf_spec_dump(spec);

    // validate the request message
    ret = validate_vrf_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("vrf validation failed, err : {}", ret);
        goto end;
    }

    // check if vrf exists already, and reject if one is found
    if ((vrf = vrf_lookup_by_id(spec.key_or_handle().vrf_id()))) {
        HAL_TRACE_ERR("Failed to create a vrf, vrf {} exists already",
                      spec.key_or_handle().vrf_id());
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate a PI vrf object
    vrf = vrf_alloc_init();
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init vrf");
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize vrf attrs from its spec
    ret = vrf_init_from_spec(vrf, spec);
    if ((ret != HAL_RET_OK) &&
        (vrf->nwsec_profile_handle == HAL_HANDLE_INVALID)) {
        HAL_TRACE_ERR("Failed to create vrf {}, security profile not found",
                      vrf->vrf_id);
        goto end;
    } else if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    }

    // allocate hal handle id
    vrf->hal_handle = hal_handle_alloc(HAL_OBJ_ID_VRF);
    if (vrf->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle {}", vrf->vrf_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        vrf_cleanup(vrf);
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

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (vrf) {
            // if there is an error, if will be freed in abort cb
            vrf = NULL;
        }
        HAL_API_STATS_INC(HAL_API_VRF_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_VRF_CREATE_SUCCESS);
    }

    vrf_prepare_rsp(rsp, ret, vrf ? vrf->hal_handle : HAL_HANDLE_INVALID);
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

    // walk L2 segs
    for (const void *ptr : *vrf->l2seg_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        l2seg = l2seg_lookup_by_handle(*p_hdl_id);
        if (!l2seg) {
            HAL_TRACE_ERR("Failed to find L2seg with handle {}", *p_hdl_id);
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
        HAL_TRACE_ERR("VRF spec has no key or handle");
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
        *nwsec_change = true;
        *new_nwsec_handle = spec.security_key_handle().profile_handle();
        HAL_TRACE_DEBUG("Updated nwsec profile to {}", *new_nwsec_handle);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// check if gipo prefix changed for vrf
//------------------------------------------------------------------------------
hal_ret_t
vrf_gipo_prefix_update (VrfSpec& spec, vrf_t *vrf, bool *gipo_prefix_change,
                        ip_prefix_t *new_gipo_prefix)
{
    hal_ret_t               ret = HAL_RET_OK;

    *gipo_prefix_change = false;

    if (vrf->vrf_type != types::VRF_TYPE_INFRA) {
        // If no infra, ignore gipo prefix change
        goto end;
    }

    if (spec.has_gipo_prefix()) {
        ret = ip_pfx_spec_to_pfx_spec(new_gipo_prefix, spec.gipo_prefix());
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Invalid GIPO prefix specified for VRF Update {}",
                           vrf->vrf_id);
            goto end;
        }
    }
    if (!ip_prefix_is_equal(&vrf->gipo_prefix, new_gipo_prefix)) {
        HAL_TRACE_DEBUG("gipo prefix change {} => {}",
                        ippfx2str(&vrf->gipo_prefix),
                        ippfx2str(new_gipo_prefix));
        *gipo_prefix_change = true;
    }

end:

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// check if anything changed for vrf
//------------------------------------------------------------------------------
hal_ret_t
vrf_check_for_updates (VrfSpec& spec, vrf_t *vrf,
                       vrf_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (vrf->vrf_type != spec.vrf_type()) {
        HAL_TRACE_ERR("Vrf type change from {} to {} not allowed",
                      vrf->vrf_type, spec.vrf_type());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // check for nwsec update
    ret = vrf_nwsec_update(spec, vrf, &app_ctxt->nwsec_prof_change,
                           &app_ctxt->nwsec_profile_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to check for nwsec update. err : {}", ret);
        goto end;
    }

    // check for gipo prefix update
    ret = vrf_gipo_prefix_update(spec, vrf, &app_ctxt->gipo_prefix_change,
                                 &app_ctxt->new_gipo_prefix);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to check for gipo pfx update. err : {}", ret);
        goto end;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update with new nwsec profile
//------------------------------------------------------------------------------
hal_ret_t
vrf_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret         = HAL_RET_OK;
    pd::pd_vrf_update_args_t    pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode      = NULL;
    dhl_entry_t                 *dhl_entry  = NULL;
    vrf_t                       *vrf        = NULL, *vrf_clone = NULL;
    vrf_update_app_ctxt_t       *app_ctxt   = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt  = (vrf_update_app_ctxt_t *)cfg_ctxt->app_ctxt;
    vrf       = (vrf_t *)dhl_entry->obj;
    vrf_clone = (vrf_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("VRF update upd cb for vrf {}", vrf->vrf_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_vrf_update_args_init(&pd_vrf_args);
    pd_vrf_args.vrf                = vrf_clone;
    pd_vrf_args.nwsec_profile      = app_ctxt->nwsec_prof;
    pd_vrf_args.gipo_prefix_change = app_ctxt->gipo_prefix_change;
    pd_vrf_args.new_gipo_prefix    = &app_ctxt->new_gipo_prefix;
    HAL_TRACE_DEBUG("new_gipo_pfx: {}", ippfx2str(pd_vrf_args.new_gipo_prefix));
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_UPDATE, (void *)&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update vrf pd, err : {}", ret);
    }

    // Pass the vrf(old/new is fine) and new nwsec profile
    if (app_ctxt->nwsec_prof_change) {
        // Triggers reprogramming of input properties table for enicifs and uplinks
        vrf_handle_nwsec_update(vrf, app_ctxt->nwsec_prof);
    }
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
    pd::pd_vrf_make_clone_args_t args;

    pd::pd_vrf_make_clone_args_init(&args);

    *ten_clone = vrf_alloc_init();
    memcpy(*ten_clone, ten, sizeof(vrf_t));

    args.vrf = ten;
    args.clone = *ten_clone;
    pd::hal_pd_call(pd::PD_FUNC_ID_VRF_MAKE_CLONE, (void *)&args);

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
    hal_ret_t                         ret         = HAL_RET_OK;
    pd::pd_vrf_mem_free_args_t        pd_vrf_args = { 0 };
    dllist_ctxt_t                     *lnode      = NULL;
    dhl_entry_t                       *dhl_entry  = NULL;
    vrf_t                             *vrf        = NULL, *vrf_clone = NULL;
    vrf_update_app_ctxt_t             *app_ctxt   = NULL;
    nwsec_profile_t                   *nwsec_prof = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (vrf_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->obj;
    vrf_clone = (vrf_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("VRF update commit cb {}", vrf->vrf_id);
    // update clone with new attrs
    if (app_ctxt->nwsec_prof_change) {
        vrf_clone->nwsec_profile_handle = app_ctxt->nwsec_profile_handle;

        if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
            // detach from old nwsec prof
            nwsec_prof = find_nwsec_profile_by_handle(
                         vrf->nwsec_profile_handle);
            if (nwsec_prof == NULL) {
                HAL_TRACE_ERR("Failed to find nwsec prof with hdl:{}",
                              vrf->nwsec_profile_handle);
                goto end;
            }
            ret = nwsec_prof_del_vrf(nwsec_prof, vrf);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to detach old nwsec prof");
                goto end;
            }
        }

        if (vrf_clone->nwsec_profile_handle != HAL_HANDLE_INVALID) {
            // attach to new nwsec prof
            ret = nwsec_prof_add_vrf(app_ctxt->nwsec_prof, vrf_clone);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to attach new nwsec prof");
                goto end;
            }
        }
    }

    // update clone with gipo prefix
    if (app_ctxt->gipo_prefix_change) {
        memcpy(&vrf_clone->gipo_prefix, &app_ctxt->new_gipo_prefix,
               sizeof(ip_prefix_t));
    }

    // Free PD
    pd::pd_vrf_mem_free_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_MEM_FREE, (void *)&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete vrf pd, err : {}", ret);
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
    hal_ret_t                       ret         = HAL_RET_OK;
    pd::pd_vrf_mem_free_args_t      pd_vrf_args = { 0 };
    dllist_ctxt_t                   *lnode      = NULL;
    dhl_entry_t                     *dhl_entry  = NULL;
    vrf_t                           *vrf        = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (vrf_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    vrf = (vrf_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("VRF update commit cb {}", vrf->vrf_id);
    // Free PD
    pd::pd_vrf_mem_free_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_MEM_FREE, (void *)&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete vrf pd, err : {}", ret);
    }

    // free the cloned object, do not free the lists as they are still being
    // referenced by the original object
    vrf_free(vrf);
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
    hal_ret_t                ret       = HAL_RET_OK;
    vrf_t                    *vrf      = NULL;
    cfg_op_ctxt_t            cfg_ctxt  = { 0 };
    dhl_entry_t              dhl_entry = { 0 };
    const VrfKeyHandle       &kh       = spec.key_or_handle();
    vrf_update_app_ctxt_t    app_ctxt  = { 0 };

    // dump spec
    vrf_spec_dump(spec);

    // validate the request message
    ret = validate_vrf_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("VRF delete validation failure, err : {}", ret);
        goto end;
    }

    vrf = vrf_lookup_key_or_handle(kh);
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf, id {}, handle {}",
                      kh.vrf_id(), kh.vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("Updating vrf {}", vrf->vrf_id);
    ret = vrf_check_for_updates(spec, vrf, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to check if vrf is updated.");
        goto end;
    }

    if (!app_ctxt.nwsec_prof_change && !app_ctxt.gipo_prefix_change) {
        HAL_TRACE_ERR("No change in vrf update detected");
        // Its a no-op. We can just return HAL_RET_OK
        // ret = HAL_RET_INVALID_OP;
        goto end;
    }

    if (app_ctxt.nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("No nwsec prof passed, "
                        "using default security profile");
        app_ctxt.nwsec_prof = NULL;
    } else {
        app_ctxt.nwsec_prof = find_nwsec_profile_by_handle(app_ctxt.nwsec_profile_handle);
        if (app_ctxt.nwsec_prof == NULL) {
            HAL_TRACE_ERR("nwsec profile with handle {} not found",
                    app_ctxt.nwsec_profile_handle);
            ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
            HAL_API_STATS_INC(HAL_API_VRF_UPDATE_FAIL);
            goto end;
        } else {
            HAL_TRACE_DEBUG("new nwsec profile id : {}",
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

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_VRF_UPDATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_VRF_UPDATE_FAIL);
    }

    vrf_prepare_rsp(rsp, ret,
                       vrf ? vrf->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//------------------------------------------------------------------------------
// process a get request for a given vrf
//------------------------------------------------------------------------------
static void
vrf_process_get (vrf_t *vrf, VrfGetResponse *rsp)
{
    nwsec_profile_t *sec_prof = NULL;

    // fill config spec of this vrf
    rsp->mutable_spec()->mutable_key_or_handle()->set_vrf_id(vrf->vrf_id);
    sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    if (sec_prof != NULL) {
        rsp->mutable_spec()->mutable_security_key_handle()->set_profile_id(sec_prof->profile_id);
    }

    // fill operational state of this vrf
    rsp->mutable_status()->set_vrf_handle(vrf->hal_handle);

    // fill stats of this vrf
    rsp->mutable_stats()->set_num_l2_segments(vrf->l2seg_list->num_elems());
    rsp->mutable_stats()->set_num_acls(vrf->acl_list->num_elems());
    rsp->mutable_stats()->set_num_security_groups(vrf->num_sg);
    rsp->mutable_stats()->set_num_l4lb_services(vrf->num_l4lb_svc);
    rsp->mutable_stats()->set_num_endpoints(vrf->num_ep);
    rsp->mutable_spec()->set_vrf_type(vrf->vrf_type);

    rsp->set_api_status(types::API_STATUS_OK);
}

//------------------------------------------------------------------------------
// callback invoked from vrf hash table while processing vrf get request
//------------------------------------------------------------------------------
static bool
vrf_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    VrfGetResponseMsg *rsp          = (VrfGetResponseMsg *)ctxt;
    VrfGetResponse *response        = rsp->add_response();
    vrf_t          *vrf             = NULL;

    vrf = (vrf_t *)hal_handle_get_obj(entry->handle_id);
    vrf_process_get(vrf, response);

    // return false here, so that we walk through all hash table entries.
    return false;
}

//------------------------------------------------------------------------------
// process a vrf get request
//------------------------------------------------------------------------------
hal_ret_t
vrf_get (VrfGetRequest& req, VrfGetResponseMsg *rsp)
{
    vrf_t        *vrf;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        // If the Vrf key handle field is not set, then this is a request
        // for information from all VRFs. Run through all VRFs in the hash
        // table and populate the response.
        g_hal_state->vrf_id_ht()->walk(vrf_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        vrf = vrf_lookup_key_or_handle(kh);
        auto response = rsp->add_response();
        if (vrf == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_VRF_GET_FAIL);
            return HAL_RET_VRF_NOT_FOUND;
        } else {
            vrf_process_get(vrf, response);
        }
    }

    HAL_API_STATS_INC(HAL_API_VRF_GET_SUCCESS);
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
        HAL_TRACE_ERR("VRF spec has no key or handle");
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
    pd::pd_vrf_delete_args_t    pd_vrf_args = { 0 };
    dllist_ctxt_t               *lnode      = NULL;
    dhl_entry_t                 *dhl_entry  = NULL;
    vrf_t                       *vrf        = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    // TODO: Check the dependency ref count for the vrf.
    //       If its non zero, fail the delete.

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    vrf = (vrf_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("VRF delete del cb {}", vrf->vrf_id);
    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_vrf_delete_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_DELETE, (void *)&pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete vrf pd, err : {}",
                      ret);
    }
    return ret;
}

//------------------------------------------------------------------------------
// update PI DBs as vrf_delete_del_cb() was a succcess
//      a. Delete from vrf id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t          ret = HAL_RET_OK;
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    vrf_t              *vrf = NULL;
    hal_handle_t       hal_handle = 0;
    nwsec_profile_t    *sec_prof = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    vrf = (vrf_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("VRF delete commit cb {}", vrf->vrf_id);

    // Remove vrf references from other objects
    if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
        ret = nwsec_prof_del_vrf(sec_prof, vrf);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del rel. from nwsec. prof");
            goto end;
        }

    }

    // a. Remove from vrf id hash table
    ret = vrf_del_from_db(vrf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del vrf {} from db, err : {}",
                      vrf->vrf_id, ret);
        goto end;
    }

    // If the VRF deleted is Infra VRF, remove from hal_state
    if (vrf->vrf_type == types::VRF_TYPE_INFRA) {
        g_hal_state->set_infra_vrf_handle(HAL_HANDLE_INVALID);
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI vrf
    vrf_cleanup(vrf);

end:

    return ret;
}

//------------------------------------------------------------------------------
// if delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
vrf_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// if delete fails, nothing to do
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
        HAL_TRACE_ERR("VRF delete failure, l2segs still referring :");
        hal_print_handles_block_list(vrf->l2seg_list);
        goto end;
    }

    // check for no presence of acls
    if (vrf->acl_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("VRF delete failure, acls still referring :");
        hal_print_handles_block_list(vrf->acl_list);
        goto end;
    }

end:
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

    // validate the request message
    ret = validate_vrf_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("vrf delete validation failed, err : {}", ret);
        goto end;
    }

    vrf = vrf_lookup_key_or_handle(kh);
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf, id {}, handle {}",
                      kh.vrf_id(), kh.vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("Deleting vrf {}", vrf->vrf_id);

    // validate if there no objects referring this sec. profile
    ret = validate_vrf_delete(vrf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("vrf delete validation failed, err : {}", ret);
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

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC (HAL_API_VRF_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC (HAL_API_VRF_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

//-----------------------------------------------------------------------------
// adds l2seg into vrf list
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
        HAL_TRACE_DEBUG("Failed to add l2seg {} to vrf {}",
                        l2seg->seg_id, vrf->vrf_id);
        goto end;
    }

end:
    HAL_TRACE_DEBUG("Added l2seg {} to vrf {}", l2seg->seg_id, vrf->vrf_id);
    return ret;
}

//-----------------------------------------------------------------------------
// remove l2seg from vrf list
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
        HAL_TRACE_ERR("Failed to remove l2seg {} from from vrf {}, err : {}",
                       l2seg->seg_id, vrf->vrf_id, ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted l2seg {} from vrf {}", l2seg->seg_id, vrf->vrf_id);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// adds acl into vrf list
//-----------------------------------------------------------------------------
hal_ret_t
vrf_add_acl (vrf_t *vrf, acl_t *acl)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (vrf == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    vrf_lock(vrf, __FILENAME__, __LINE__, __func__);      // lock
    ret = vrf->acl_list->insert(&acl->hal_handle);
    vrf_unlock(vrf, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add acl {} to vrf {}",
                        acl->key, vrf->vrf_id);
        goto end;
    }

end:

    HAL_TRACE_DEBUG("Added acl {} to vrf {}", acl->key, vrf->vrf_id);

    return ret;
}

//-----------------------------------------------------------------------------
// remove acl from vrf list
//-----------------------------------------------------------------------------
hal_ret_t
vrf_del_acl (vrf_t *vrf, acl_t *acl)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (vrf == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    vrf_lock(vrf, __FILENAME__, __LINE__, __func__);      // lock
    ret = vrf->acl_list->remove(&acl->hal_handle);
    vrf_unlock(vrf, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove acl {} from from vrf {}, err : {}",
                       acl->key, vrf->vrf_id, ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted acl {} from vrf {}", acl->key, vrf->vrf_id);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// given a vrf, marshall it for persisting the vrf state (spec, status, stats)
//
// obj points to vrf object i.e., vrf_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
vrf_marshall_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    VrfGetResponse    vrf_info;
    uint32_t          serialized_state_sz;
    vrf_t             *vrf = (vrf_t *)obj;

    HAL_ASSERT((vrf != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this vrf (includes spec, status & stats)
    vrf_process_get(vrf, &vrf_info);
    serialized_state_sz = vrf_info.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall VRF {}, not enough room, "
                      "required size {}, available size {}",
                      vrf->vrf_id, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (vrf_info.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize vrf {}", vrf->vrf_id);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled vrf {}, len {}",
                    vrf->vrf_id, serialized_state_sz);
    return HAL_RET_OK;
}

static hal_ret_t
vrf_restore_add (vrf_t *vrf, const VrfGetResponse& vrf_info)
{
    hal_ret_t                    ret;
    pd::pd_vrf_restore_args_t    pd_vrf_args = { 0 };

    // restore pd state
    pd::pd_vrf_restore_args_init(&pd_vrf_args);
    pd_vrf_args.vrf = vrf;
    pd_vrf_args.vrf_status = &vrf_info.status();
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_VRF_RESTORE, &pd_vrf_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore vrf {} pd, err : {}",
                      vrf->vrf_id, ret);
    }
    return ret;
}

static hal_ret_t
vrf_restore_commit (vrf_t *vrf, const VrfGetResponse& vrf_info)
{
    hal_ret_t          ret;
    nwsec_profile_t    *sec_prof = NULL;

    HAL_TRACE_DEBUG("Committing vrf {} restore", vrf->vrf_id);

    if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        sec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    }
    ret = vrf_add_to_db(vrf, vrf->hal_handle, sec_prof);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore vrf {} to db, err : {}",
                      vrf->vrf_id, ret);
    }
    return ret;
}

static hal_ret_t
vrf_restore_abort (vrf_t *vrf, const VrfGetResponse& vrf_info)
{
    HAL_TRACE_ERR("Aborting vrf {} restore", vrf->vrf_id);
    vrf_create_abort_cleanup(vrf, vrf->hal_handle);
    return HAL_RET_OK;
}

uint32_t
vrf_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t         ret;
    VrfGetResponse    vrf_info;
    vrf_t             *vrf;

    // de-serialize the object
    if (vrf_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized vrf obj");
        return 0;
    }

    // allocate VRF obj from slab
    vrf = vrf_alloc_init();
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init vrf, err : {}", ret);
        return 0;
    }

    // initialize vrf attrs from its spec
    vrf_init_from_spec(vrf, vrf_info.spec());
    vrf_init_from_status(vrf, vrf_info.status());
    vrf_init_from_stats(vrf, vrf_info.stats());

    // repopulate handle db
    hal_handle_alloc(HAL_OBJ_ID_VRF, vrf->hal_handle);

    ret = vrf_restore_add(vrf, vrf_info);
    if (ret != HAL_RET_OK) {
        vrf_restore_abort(vrf, vrf_info);
    }
    vrf_restore_commit(vrf, vrf_info);
    return 0;    // TODO: fix me
}

}    // namespace hal
