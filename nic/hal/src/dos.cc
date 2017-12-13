// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/dos.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

uint32_t
dos_policy_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
                                      sizeof(dos_policy_id_t)) % ht_size;
}

bool
dos_policy_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(dos_policy_id_t *)key1 == *(dos_policy_id_t *)key2) {
        return true;
    }
    return false;
}

static inline hal_ret_t
dos_policy_handle_update (DoSPolicySpec& spec, dos_policy_t *dosp, 
                          dos_policy_update_app_ctx_t *app_ctx)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

// initialize dos policy object from the config spec
static inline void
dos_policy_props_init_from_spec (dos_policy_prop_t *dosp,
                                 const nwsec::DoSProtectionSpec& spec)
{
    nwsec_group_t *nwsec_group = NULL;

    dosp->service.ip_proto = spec.svc().ip_protocol();
    if (spec.svc().l4_info_case() == DoSService::kIcmpMsg) {
        dosp->service.icmp_msg_type = spec.svc().icmp_msg().type();
        dosp->service.icmp_msg_code = spec.svc().icmp_msg().code();
        dosp->service.is_icmp = TRUE;
    } else {
        dosp->service.dport = spec.svc().dst_port();
        dosp->service.is_icmp = FALSE;
    }

    dosp->session_setup_rate = spec.session_setup_rate();

    dosp->session_limits.max_sessions     = spec.session_limits().max_sessions();
    dosp->session_limits.blocking_timeout = spec.session_limits().blocking_timeout();

    dosp->policer.peak_rate     = spec.policer().peak_rate();
    dosp->policer.burst_size    = spec.policer().burst_size();
    dosp->policer.bytes_per_sec = spec.policer().bytes_per_second();

    dosp->tcp_syn_flood_limits.restrict_pps =
        spec.tcp_syn_flood_limits().restrict_limits().pps();
    dosp->tcp_syn_flood_limits.restrict_burst_pps =
        spec.tcp_syn_flood_limits().restrict_limits().burst_pps();
    dosp->tcp_syn_flood_limits.restrict_duration =
        spec.tcp_syn_flood_limits().restrict_limits().duration();
    dosp->tcp_syn_flood_limits.protect_pps =
        spec.tcp_syn_flood_limits().protect_limits().pps();
    dosp->tcp_syn_flood_limits.protect_burst_pps =
        spec.tcp_syn_flood_limits().protect_limits().burst_pps();
    dosp->tcp_syn_flood_limits.protect_duration =
        spec.tcp_syn_flood_limits().protect_limits().duration();

    dosp->udp_flood_limits.restrict_pps =
        spec.udp_flood_limits().restrict_limits().pps();
    dosp->udp_flood_limits.restrict_burst_pps =
        spec.udp_flood_limits().restrict_limits().burst_pps();
    dosp->udp_flood_limits.restrict_duration =
        spec.udp_flood_limits().restrict_limits().duration();
    dosp->udp_flood_limits.protect_pps =
        spec.udp_flood_limits().protect_limits().pps();
    dosp->udp_flood_limits.protect_burst_pps =
        spec.udp_flood_limits().protect_limits().burst_pps();
    dosp->udp_flood_limits.protect_duration =
        spec.udp_flood_limits().protect_limits().duration();

    dosp->icmp_flood_limits.restrict_pps =
        spec.icmp_flood_limits().restrict_limits().pps();
    dosp->icmp_flood_limits.restrict_burst_pps =
        spec.icmp_flood_limits().restrict_limits().burst_pps();
    dosp->icmp_flood_limits.restrict_duration =
        spec.icmp_flood_limits().restrict_limits().duration();
    dosp->icmp_flood_limits.protect_pps =
        spec.icmp_flood_limits().protect_limits().pps();
    dosp->icmp_flood_limits.protect_burst_pps =
        spec.icmp_flood_limits().protect_limits().burst_pps();
    dosp->icmp_flood_limits.protect_duration =
        spec.icmp_flood_limits().protect_limits().duration();

    dosp->other_flood_limits.restrict_pps =
        spec.other_flood_limits().restrict_limits().pps();
    dosp->other_flood_limits.restrict_burst_pps =
        spec.other_flood_limits().restrict_limits().burst_pps();
    dosp->other_flood_limits.restrict_duration =
        spec.other_flood_limits().restrict_limits().duration();
    dosp->other_flood_limits.protect_pps =
        spec.other_flood_limits().protect_limits().pps();
    dosp->other_flood_limits.protect_burst_pps =
        spec.other_flood_limits().protect_limits().burst_pps();
    dosp->other_flood_limits.protect_duration =
        spec.other_flood_limits().protect_limits().duration();

    /* Lookup the SG by handle and then get the SG-id */
    if (spec.peer_sg_handle() != HAL_HANDLE_INVALID) {
        nwsec_group = nwsec_group_lookup_by_handle(spec.peer_sg_handle());
        HAL_ASSERT(nwsec_group);
        dosp->peer_sg_id = nwsec_group->sg_id;
    } else {
        dosp->peer_sg_id = HAL_NWSEC_INVALID_SG_ID;
    }

    return;
}

static inline void
dos_policy_init_from_spec (dos_policy_t *dosp,
                           nwsec::DoSPolicySpec& spec)
{
    int                         num_sgs, sg_id;
    dos_policy_sg_list_entry_t  *entry = NULL;
    nwsec_group_t               *nwsec_group = NULL;

    /*
     * Populate the list of security groups that this DoS policy
     * is attached to
     */
    utils::dllist_reset(&dosp->sg_list_head);
    num_sgs = spec.sg_handle_size();
    for (int i = 0; i < num_sgs; i++) {
        /* Lookup the SG by handle and then get the SG-id */
        nwsec_group = nwsec_group_lookup_by_handle(spec.sg_handle(i));
        HAL_ASSERT(nwsec_group);
        sg_id = nwsec_group->sg_id;
        /* Add to the security group list */
        entry = (dos_policy_sg_list_entry_t *)g_hal_state->
                 dos_policy_sg_list_entry_slab()->alloc();
        entry->sg_id = sg_id;
        // Insert into the list
        utils::dllist_add(&dosp->sg_list_head, &entry->dllist_ctxt);
    }

    /* Populate Ingress and Egress policy params if present in the config */
    if (spec.has_ingress_policy()) {
        dosp->ingr_pol_valid = TRUE;
        dos_policy_props_init_from_spec(&dosp->ingress,
                            spec.ingress_policy().dos_protection());
    }
    if (spec.has_egress_policy()) {
        dosp->egr_pol_valid = TRUE;
        dos_policy_props_init_from_spec(&dosp->egress,
                            spec.egress_policy().dos_protection());
    }
    return;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_create_add_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_dos_policy_args_t        pd_dosp_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    dos_policy_t                    *dosp = NULL;
    // dos_policy_create_app_ctx_t         *app_ctx = NULL; 

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}: invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctx = (dos_policy_create_app_ctx_t *)cfg_ctx->app_ctx;

    dosp = (dos_policy_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-dos-policy:{}:create add CB",
                    __FUNCTION__);

    // PD Call to allocate PD resources and HW programming
    pd::pd_dos_policy_args_init(&pd_dosp_args);
    pd_dosp_args.dos_policy = dosp;
    ret = pd::pd_dos_policy_create(&pd_dosp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to create dosp pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as dos_policy_create_add_cb() was a success
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_create_commit_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    //dllist_ctxt_t               *lnode = NULL;
    //dhl_entry_t                 *dhl_entry = NULL;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    //lnode = cfg_ctx->dhl.next;
    //dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    HAL_TRACE_DEBUG("pi-dos-policy:{}:create commit CB",
                    __FUNCTION__);

    // TODO: Increment the ref counts of dependent objects
    //  - Have to increment ref count for vrf

end:
    return ret;
}

//------------------------------------------------------------------------------
// dos_policy_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI vrf 
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_create_abort_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    //pd::pd_dos_policy_args_t      pd_dosp_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    dos_policy_t                    *dosp = NULL;
    hal_handle_t                    hal_handle = 0;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    dosp = (dos_policy_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-dos-policy:{}:create abort CB {}",
                    __FUNCTION__, dosp->hal_handle);

    // 1. delete call to PD
    if (dosp->pd) {
#if 0
        pd::pd_dos_policy_args_init(&pd_dosp_args);
        pd_dosp_args.dos_policy = dosp;
        ret = pd::pd_dos_policy_delete(&pd_dosp_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-dos-policy:{}:failed to delete dosp pd, err : {}", 
                          __FUNCTION__, ret);
        }
#endif
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI vrf
    dos_policy_free(dosp);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
dos_policy_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_prepare_rsp (DoSPolicyResponse *rsp, hal_ret_t ret, 
                        hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_dos_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

// create an instance of dos policy
hal_ret_t
dos_policy_create (nwsec::DoSPolicySpec& spec,
                   nwsec::DoSPolicyResponse *rsp)
{
    hal_ret_t                   ret;
    dos_policy_t                *dosp = NULL;
    //dos_policy_create_app_ctx_t app_ctx;
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctx = { 0 };
    vrf_id_t                 tid;
    vrf_t                    *vrf = NULL;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("pi-dos-policy:{}: creating dos policy for ten id {}", __FUNCTION__,
                    spec.meta().vrf_id());

    // check if dos policy exists already, and reject if one is found
    if (find_dos_policy_by_handle(spec.dos_handle())) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to create a dosp, "
                      "dosp{} exists already", __FUNCTION__, 
                      spec.dos_handle());
        ret =  HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate the dos policy
    dosp = dos_policy_alloc_init();
    if (dosp == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // consume the config
    dos_policy_init_from_spec(dosp, spec);

    // allocate hal handle id
    dosp->hal_handle = hal_handle_alloc(HAL_OBJ_ID_DOS_POLICY);
    if (dosp->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-dos-policy:{}: failed to alloc handle {}", 
                      __FUNCTION__, dosp->hal_handle);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }
    // fetch the vrf information
    tid = spec.meta().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }
    dosp->vrf_handle = vrf->hal_handle;

    // form ctxt and call infra add. nothing to populate in app ctxt
    dhl_entry.handle = dosp->hal_handle;
    dhl_entry.obj = dosp;
    //cfg_ctx.app_ctx = &app_ctx;
    utils::dllist_reset(&cfg_ctx.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctx.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(dosp->hal_handle, &cfg_ctx, 
                             dos_policy_create_add_cb,
                             dos_policy_create_commit_cb,
                             dos_policy_create_abort_cb, 
                             dos_policy_create_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        if (dosp) {
            dos_policy_free(dosp);
            dosp = NULL;
        }
    }

    dos_policy_prepare_rsp(rsp, ret, dosp ? dosp->hal_handle : HAL_HANDLE_INVALID);
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// validate dosp update request
//------------------------------------------------------------------------------
hal_ret_t
validate_dos_policy_update (DoSPolicySpec & spec, DoSPolicyResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (!spec.has_meta() ||
        spec.meta().vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("pi-ep:{}:vrf id not valid",
                      __FUNCTION__);
        return HAL_RET_VRF_ID_INVALID;
    }

    // key-handle field must be set
    if (spec.dos_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-dos-policy:{}:spec has no handle", __FUNCTION__);
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
dos_policy_update_upd_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    //pd::pd_dos_policy_args_t        pd_dosp_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    dos_policy_t                    *dosp = NULL;
    //dos_policy_t                  *dosp_clone = NULL;
    // dos_policy_update_app_ctx_t  *app_ctx = NULL;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dosp{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctx = (dos_policy_update_app_ctx_t *)cfg_ctx->app_ctx;

    dosp = (dos_policy_t *)dhl_entry->obj;
    //dosp_clone = (dos_policy_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-dos-policy:{}: update upd cb {}",
                    __FUNCTION__, dosp->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
#if 0
    pd::pd_dos_policy_args_init(&pd_dosp_args);
    pd_dosp_args.dos_policy = dosp;
    pd_dosp_args.clone_profile = dosp_clone;
    ret = pd::pd_dos_policy_update(&pd_dosp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to update dosp pd, err : {}",
                      __FUNCTION__, ret);
    }
#endif

    // TODO: If IPSG changes, it has to trigger to vrfs.
end:
    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_make_clone (dos_policy_t *dosp, dos_policy_t **dosp_clone, 
                  DoSPolicySpec& spec)
{
    *dosp_clone = dos_policy_alloc_init();
    memcpy(*dosp_clone, dosp, sizeof(dos_policy_t));

    //pd::pd_dos_policy_make_clone(dosp, *dosp_clone);


    // Keep new values in the clone
    dos_policy_init_from_spec(*dosp_clone, spec);


    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD dosp.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_update_commit_cb(cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    //pd::pd_dos_policy_args_t pd_dosp_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    dos_policy_t             *dosp = NULL;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dosp{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    dosp = (dos_policy_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-dos-policy:{}:update commit CB {}",
                    __FUNCTION__, dosp->hal_handle);

    // Free PD
#if 0
    pd::pd_dos_policy_args_init(&pd_dosp_args);
    pd_dosp_args.dos_policy = dosp;
    ret = pd::pd_dos_policy_mem_free(&pd_dosp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to delete dosp pd, err : {}",
                      __FUNCTION__, ret);
    }
#endif

    // Free PI
    dos_policy_free(dosp);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Free the clones
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_update_abort_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    //pd::pd_dos_policy_args_t pd_dosp_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    dos_policy_t             *dosp = NULL;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dosp{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // assign clone as we are trying to free only the clone
    dosp = (dos_policy_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-dos-policy:{}:update commit CB {}",
                    __FUNCTION__, dosp->hal_handle);

    // Free PD
#if 0
    pd::pd_dos_policy_args_init(&pd_dosp_args);
    pd_dosp_args.dos_policy = dosp;
    ret = pd::pd_dos_policy_mem_free(&pd_dosp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to delete dosp pd, err : {}",
                      __FUNCTION__, ret);
    }
#endif

    // Free PI
    dos_policy_free(dosp);
end:

    return ret;
}

hal_ret_t
dos_policy_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctx)
{
    return HAL_RET_OK;
}

// update a dos policy instance
hal_ret_t
dos_policy_update (nwsec::DoSPolicySpec& spec,
                   nwsec::DoSPolicyResponse *rsp)
{
    hal_ret_t                   ret;
    dos_policy_t                *dosp;
    // dos_policy_t             local_dosp;
    cfg_op_ctxt_t               cfg_ctx = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    dos_policy_update_app_ctx_t app_ctx;

    HAL_TRACE_DEBUG("---------------- Sec. Profile Update API Start ---------");
    HAL_TRACE_DEBUG("pi-dos-policy:{}: dos policy update for handle{}", __FUNCTION__, spec.dos_handle());

    // validate the request message
    ret = validate_dos_policy_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:dosp update validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    // lookup this dos policy
    dosp = find_dos_policy_by_handle(spec.dos_handle());
    if (!dosp) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to find dosp, handle {}",
                      __FUNCTION__, spec.dos_handle());
        ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        goto end;
    }

    ret = dos_policy_handle_update(spec, dosp, &app_ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:dosp check update failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

#if 0
    if (!app_ctx.dos_policy_changed) {
        HAL_TRACE_ERR("pi-dos-policy:{}:no change in dosp update: noop", __FUNCTION__);
        goto end;
    }
#endif

    dos_policy_make_clone(dosp, (dos_policy_t **)&dhl_entry.cloned_obj, spec);

    // form ctxt and call infra update object
    dhl_entry.handle = dosp->hal_handle;
    dhl_entry.obj = dosp;
    //cfg_ctx.app_ctx = &app_ctx;
    utils::dllist_reset(&cfg_ctx.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctx.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(dosp->hal_handle, &cfg_ctx, 
                             dos_policy_update_upd_cb,
                             dos_policy_update_commit_cb,
                             dos_policy_update_abort_cb, 
                             dos_policy_update_cleanup_cb);

end:
    dos_policy_prepare_rsp(rsp, ret, dosp ? dosp->hal_handle : HAL_HANDLE_INVALID);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;


#if 0
    // Calling PD update
    pd::pd_dos_policy_args_init(&pd_dos_policy_args);
    pd_dos_policy_args.dos_policy = &local_dosp;
    ret = pd::pd_dos_policy_update(&pd_dos_policy_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD dos policy update failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    } else {
        // Success: Update the store PI object 
        dos_policy_init_from_spec(dosp, spec);
    }

end:

    HAL_TRACE_DEBUG("PI-Nwsec:{}: Nwsec Update for id {} handle {} ret{}", __FUNCTION__, 
                    spec.key_or_handle().security_group_id(), spec.key_or_handle().profile_handle(), ret);
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
#endif
}

//------------------------------------------------------------------------------
// validate dosp delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_dos_policy_delete (DoSPolicyDeleteRequest& req, 
                       DoSPolicyDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (req.dos_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-dos-policy:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_delete_del_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    //pd::pd_dos_policy_args_t pd_dosp_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    dos_policy_t             *dosp = NULL;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the dosp. 
    //       If its non zero, fail the delete.


    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    dosp = (dos_policy_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-dos-policy:{}:delete del CB {}",
                    __FUNCTION__, dosp->hal_handle);

    // 1. PD Call to allocate PD resources and HW programming
#if 0
    pd::pd_dos_policy_args_init(&pd_dosp_args);
    pd_dosp_args.dos_policy = dosp;
    ret = pd::pd_dos_policy_delete(&pd_dosp_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to delete dosp pd, err : {}", 
                      __FUNCTION__, ret);
    }
#endif

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
dos_policy_delete_commit_cb (cfg_op_ctxt_t *cfg_ctx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    dos_policy_t                     *dosp = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctx == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}:invalid cfg_ctx", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctx->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    dosp = (dos_policy_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-dos-policy:{}:delete commit CB {}",
                    __FUNCTION__, dosp->hal_handle);

    // a. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // b. Free PI dosp
    dos_policy_free(dosp);

    // TODO: Decrement the ref counts of dependent objects
    //  - Have to decrement ref count for dos policy

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_delete_abort_cb (cfg_op_ctxt_t *cfg_ctx)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctx)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a dosp delete request
//------------------------------------------------------------------------------
hal_ret_t
dos_policy_delete (DoSPolicyDeleteRequest& req, 
                   DoSPolicyDeleteResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    dos_policy_t    *dosp = NULL;
    cfg_op_ctxt_t   cfg_ctx = { 0 };
    dhl_entry_t     dhl_entry = { 0 };

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

    // validate the request message
    ret = validate_dos_policy_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-dos-policy:{}:dosp delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }


    dosp = find_dos_policy_by_handle(req.dos_handle());
    if (dosp == NULL) {
        HAL_TRACE_ERR("pi-dos-policy:{}:failed to find dosp, handle {}",
                      __FUNCTION__, req.dos_handle());
        ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-dos-policy:{}:deleting dos policy {}", 
                    __FUNCTION__, dosp->hal_handle);

    // form ctxt and call infra add
    dhl_entry.handle = dosp->hal_handle;
    dhl_entry.obj = dosp;
    //cfg_ctx.app_ctx = NULL;
    utils::dllist_reset(&cfg_ctx.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctx.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(dosp->hal_handle, &cfg_ctx, 
                             dos_policy_delete_del_cb,
                             dos_policy_delete_commit_cb,
                             dos_policy_delete_abort_cb, 
                             dos_policy_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
dos_policy_get (nwsec::DoSPolicyGetRequest& req,
                nwsec::DoSPolicyGetResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    dos_policy_t    *dosp;

    if (req.dos_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-dos-policy:{}:dosp update validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    // lookup this dos policy
    dosp = find_dos_policy_by_handle(req.dos_handle());
    if (!dosp) {
        rsp->set_api_status(types::API_STATUS_DOS_POLICY_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this profile
    // fill operational state of this profile
    // fill stats, if any, of this profile

end:
    return HAL_RET_OK;
}

}    // namespace hal
