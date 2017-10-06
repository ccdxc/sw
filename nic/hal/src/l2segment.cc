#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/oif_list_api.hpp"
#include "nic/hal/src/if_utils.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// hash table seg_id => entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
l2seg_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    l2seg_t                     *l2seg = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    l2seg = (l2seg_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(l2seg->seg_id);
}

// ----------------------------------------------------------------------------
// hash table seg_id => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
l2seg_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return utils::hash_algo::fnv_hash(key, sizeof(l2seg_id_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table seg_id => entry - compare function
// ----------------------------------------------------------------------------
bool
l2seg_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(l2seg_id_t *)key1 == *(l2seg_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a l2segment to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
l2seg_add_to_db (l2seg_t *l2seg, hal_handle_t handle)
{
    hal_ret_t                   ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-l2seg:{}:adding to l2seg id hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from seg id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from seg id to its handle
    entry->handle_id = handle;
    ret = g_hal_state->l2seg_id_ht()->insert_with_key(&l2seg->seg_id,
                                                       entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to add seg id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    // TODO: Check if this is the right place
    l2seg->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a l2seg from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
l2seg_del_from_db (l2seg_t *l2seg)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-l2seg:{}:removing from seg id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->l2seg_id_ht()->
        remove(&l2seg->seg_id);

    // free up
    g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming L2 segment create request
// TODO:
// 1. check if L2 segment exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_l2segment_create (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    if (!spec.has_meta() ||
        spec.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        HAL_TRACE_ERR("pi-l2seg:{}:invalid tenant id",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // TODO: Uncomment this once DOL config adds network handle
#if 0
    // must have network handle
    if (spec.network_handle() == HAL_HANDLE_INVALID) {
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        return HAL_RET_HANDLE_INVALID;
    }
#endif

    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-l2seg:{}:l2seg id and handle not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            L2SegmentKeyHandle::kSegmentId) {
        HAL_TRACE_ERR("pi-l2seg:{}:l2seg id not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have either access and/or fabric encap set
    if (!spec.has_access_encap() && !spec.has_fabric_encap()) {
        HAL_TRACE_ERR("pi-l2seg:{}:no access or fabric encap",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_ENCAP_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
l2seg_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_args_t         pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;
    l2seg_create_app_ctxt_t     *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (l2seg_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    l2seg = (l2seg_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-l2seg:{}:create add CB {}",
                    __FUNCTION__, l2seg->seg_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_l2seg_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    pd_l2seg_args.tenant = app_ctxt->tenant;
    ret = pd::pd_l2seg_create(&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to create l2seg pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as l2seg_create_add_cb() was a success
//      a. Create the flood list
//      b. Add to l2seg id hash table
//------------------------------------------------------------------------------
hal_ret_t
l2seg_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-l2seg:{}:create commit CB {}",
                    __FUNCTION__, l2seg->seg_id);


    // create the broadcast/flood list for this l2seg
    ret = oif_list_create(&l2seg->bcast_oif_list);
    HAL_ASSERT(ret == HAL_RET_OK);

    // 1. a. Add to l2seg id hash table
    ret = l2seg_add_to_db(l2seg, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to add l2seg {} to db, err : {}", 
                __FUNCTION__, l2seg->seg_id, ret);
        goto end;
    }

    // TODO: Increment the ref counts of dependent objects
    //  - Have to increment ref count for tenant

end:
    return ret;
}

//------------------------------------------------------------------------------
// l2seg_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI tenant 
//------------------------------------------------------------------------------
hal_ret_t
l2seg_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_args_t         pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                    *l2seg = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-l2seg:{}:create abort CB {}",
                    __FUNCTION__, l2seg->seg_id);

    // 1. delete call to PD
    if (l2seg->pd) {
        pd::pd_l2seg_args_init(&pd_l2seg_args);
        pd_l2seg_args.l2seg = l2seg;
        ret = pd::pd_l2seg_delete(&pd_l2seg_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-l2seg:{}:failed to delete l2seg pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI tenant
    l2seg_free(l2seg);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
l2seg_prepare_rsp (L2SegmentResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_l2segment_status()->set_l2segment_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a L2 segment create request
// TODO: if L2 segment exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
l2segment_create (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t                   ret;
    tenant_t                    *tenant;
    l2seg_t                     *l2seg = NULL;
    tenant_id_t                 tid;
    // pd::pd_l2seg_args_t         pd_l2seg_args;
    hal_handle_t                nw_handle;
    network_t                   *nw = NULL;
    l2seg_create_app_ctxt_t     app_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };


    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("pi-l2seg:{}:l2seg create with id:{}", __FUNCTION__, 
                    spec.key_or_handle().segment_id());

    // validate the request message
    ret = validate_l2segment_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("pi-l2seg:{}:validation Failed. ret: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    // check if l2segment exists already, and reject if one is found
    if (find_l2seg_by_id(spec.key_or_handle().segment_id())) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to create a l2seg, "
                      "l2seg {} exists already", __FUNCTION__, 
                      spec.key_or_handle().segment_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }

    // fetch the tenant
    tid = spec.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}: Fetch Tenant Id:{} Failed. ret: {}",
                      __FUNCTION__, tid, ret);
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // instantiate the L2 segment
    l2seg = l2seg_alloc_init();
    if (l2seg == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    l2seg->tenant_handle = tenant->hal_handle;
    l2seg->seg_id = spec.key_or_handle().segment_id();
    l2seg->segment_type = spec.segment_type();
    l2seg->mcast_fwd_policy = spec.mcast_fwd_policy();
    l2seg->bcast_fwd_policy = spec.bcast_fwd_policy();
    if (spec.has_access_encap()) {
        l2seg->access_encap.type = spec.access_encap().encap_type();
        l2seg->access_encap.val = spec.access_encap().encap_value();
        HAL_TRACE_ERR("pi-l2seg:{}:access enc_type:{} enc_val:{}",
                __FUNCTION__, l2seg->access_encap.type, 
                l2seg->access_encap.val);
    }
    if (spec.has_fabric_encap()) {
        l2seg->fabric_encap.type = spec.fabric_encap().encap_type();
        l2seg->fabric_encap.val = spec.fabric_encap().encap_value();
        HAL_TRACE_ERR("pi-l2seg:{}:fabric enc_type:{} enc_val:{}",
                __FUNCTION__, l2seg->fabric_encap.type, 
                l2seg->fabric_encap.val);
    }

    // consume network handles
    // TODO: After fixing network object, come back and fix this
    HAL_TRACE_DEBUG("pi-l2seg:{}:received {} networks", 
            __FUNCTION__, spec.network_handle_size());
    utils::dllist_reset(&l2seg->nw_list_head);
    for (int i = 0; i < spec.network_handle_size(); i++) {
        nw_handle = spec.network_handle(i);        
        nw = find_network_by_handle(nw_handle);
        if (nw == NULL) {
            rsp->set_api_status(types::API_STATUS_NETWORK_NOT_FOUND);
            return HAL_RET_INVALID_ARG;
        }
        HAL_TRACE_DEBUG("pi-l2seg:{}:adding network: {}", __FUNCTION__, 
                        ippfx2str(&nw->nw_key.ip_pfx));

        // add nw to list
        utils::dllist_add(&l2seg->nw_list_head, &nw->l2seg_nw_lentry);
    }

    if (l2seg->segment_type == types::L2_SEGMENT_TYPE_INFRA) {
        HAL_TRACE_DEBUG("pi-l2seg:{} id:{} is infra ", __FUNCTION__, 
                        l2seg->seg_id);
        g_hal_state->set_infra_l2seg(l2seg);
    }

    // allocate hal handle id
    l2seg->hal_handle = hal_handle_alloc(HAL_OBJ_ID_L2SEG);
    if (l2seg->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-l2seg:{}: failed to alloc handle {}", 
                      __FUNCTION__, l2seg->seg_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        l2seg_free(l2seg);
        return HAL_RET_HANDLE_INVALID;
    }

    // form ctxt and call infra add
    app_ctxt.tenant = tenant;
    dhl_entry.handle = l2seg->hal_handle;
    dhl_entry.obj = l2seg;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(l2seg->hal_handle, &cfg_ctxt, 
                             l2seg_create_add_cb,
                             l2seg_create_commit_cb,
                             l2seg_create_abort_cb, 
                             l2seg_create_cleanup_cb);

    l2seg_prepare_rsp(rsp, ret, l2seg->hal_handle);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;

}

//------------------------------------------------------------------------------
// validate l2seg update request
//------------------------------------------------------------------------------
hal_ret_t
validate_l2seg_update (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-l2seg:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// check if fwd policy changed for l2seg
//------------------------------------------------------------------------------
hal_ret_t
l2seg_fwdpolicy_update (L2SegmentSpec& spec, l2seg_t *l2seg, 
                        l2seg_update_app_ctxt_t *app_ctxt)
{
    if (!app_ctxt) {
        return HAL_RET_INVALID_ARG;
    }
    app_ctxt->mcast_fwd_policy_change = false;
    app_ctxt->bcast_fwd_policy_change = false;

    if (l2seg->mcast_fwd_policy != spec.mcast_fwd_policy()) {
        HAL_TRACE_DEBUG("pi-l2seg:{}:mcast_fwd_policy updated", __FUNCTION__);
        app_ctxt->mcast_fwd_policy_change = true;
        app_ctxt->new_mcast_fwd_policy = spec.mcast_fwd_policy();
    }
    if (l2seg->bcast_fwd_policy != spec.bcast_fwd_policy()) {
        HAL_TRACE_DEBUG("pi-l2seg:{}:bcast_fwd_policy updated", __FUNCTION__);
        app_ctxt->bcast_fwd_policy_change = true;
        app_ctxt->new_bcast_fwd_policy = spec.bcast_fwd_policy();
    }

    return HAL_RET_OK;
}



//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update new l2seg properties
//------------------------------------------------------------------------------
hal_ret_t
l2seg_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_args_t         pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                    *l2seg = NULL;
    l2seg_update_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (l2seg_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    l2seg = (l2seg_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-l2seg:{}: update upd cb {}",
                    __FUNCTION__, l2seg->seg_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_l2seg_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::pd_l2seg_update(&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to update l2seg pd, err : {}",
                      __FUNCTION__, ret);
    }

    // TODO: Call the CB for mcast and bcast fwd policy change
    if (app_ctxt->mcast_fwd_policy_change) {
    }
    if (app_ctxt->bcast_fwd_policy_change) {
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
//------------------------------------------------------------------------------
hal_ret_t
l2seg_make_clone (l2seg_t *l2seg, l2seg_t **l2seg_clone)
{
    *l2seg_clone = l2seg_alloc_init();
    memcpy(*l2seg_clone, l2seg, sizeof(l2seg_t));

    pd::pd_l2seg_make_clone(l2seg, *l2seg_clone);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD l2seg.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
l2seg_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_args_t         pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-l2seg:{}:update commit CB {}",
                    __FUNCTION__, l2seg->seg_id);

    // Free PD
    pd::pd_l2seg_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::pd_l2seg_mem_free(&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to delete l2seg pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    l2seg_free(l2seg);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Free the clones
//------------------------------------------------------------------------------
hal_ret_t
l2seg_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_args_t         pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // assign clone as we are trying to free only the clone
    l2seg = (l2seg_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-l2seg:{}:update commit CB {}",
                    __FUNCTION__, l2seg->seg_id);

    // Free PD
    pd::pd_l2seg_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::pd_l2seg_mem_free(&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to delete l2seg pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    l2seg_free(l2seg);
end:

    return ret;
}

hal_ret_t
l2seg_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a L2 segment update request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_update (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    l2seg_t                     *l2seg = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const L2SegmentKeyHandle    &kh = spec.key_or_handle();
    l2seg_update_app_ctxt_t     app_ctxt = { 0 };

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

    // validate the request message
    ret = validate_l2seg_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:l2seg update validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    l2seg = l2seg_lookup_key_or_handle(kh);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to find l2seg, id {}, handle {}",
                      __FUNCTION__, kh.segment_id(), kh.l2segment_handle());
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-l2seg:{}:update l2seg {}", __FUNCTION__, 
                    l2seg->seg_id);

    ret = l2seg_fwdpolicy_update(spec, l2seg, &app_ctxt); 
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to check if fwdpolicy is updated.", 
                      __FUNCTION__);
        goto end;
    }

    if (!app_ctxt.mcast_fwd_policy_change && !app_ctxt.bcast_fwd_policy_change) {
        HAL_TRACE_ERR("pi-l2seg:{}:no change in l2seg update: noop", __FUNCTION__);
        // Its a no-op. We can just return HAL_RET_OK
        // ret = HAL_RET_INVALID_OP;
        goto end;
    }

    // TODO: Check if we have to do something as fwd policy changed

    l2seg_make_clone(l2seg, (l2seg_t **)&dhl_entry.cloned_obj);

    // form ctxt and call infra update object
    dhl_entry.handle = l2seg->hal_handle;
    dhl_entry.obj = l2seg;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(l2seg->hal_handle, &cfg_ctxt, 
                             l2seg_update_upd_cb,
                             l2seg_update_commit_cb,
                             l2seg_update_abort_cb, 
                             l2seg_update_cleanup_cb);

end:
    l2seg_prepare_rsp(rsp, ret, l2seg->hal_handle);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// Lookup l2seg from key or handle
//------------------------------------------------------------------------------
l2seg_t *
l2seg_lookup_key_or_handle (const L2SegmentKeyHandle& kh)
{
    l2seg_t     *l2seg = NULL;

    if (kh.key_or_handle_case() == L2SegmentKeyHandle::kSegmentId) {
        l2seg = find_l2seg_by_id(kh.segment_id());
    } else if (kh.key_or_handle_case() == L2SegmentKeyHandle::kL2SegmentHandle) {
        l2seg = find_l2seg_by_handle(kh.l2segment_handle());
    }

    return l2seg;
}

//------------------------------------------------------------------------------
// validate l2seg delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_l2seg_delete (L2SegmentDeleteRequest& req, L2SegmentDeleteResponseMsg* rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-l2seg:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
l2seg_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_args_t         pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the l2seg. whoever is refering
    //       to l2seg should be counted against this ref count.
    //      - Ifs 
    //      - Sessions 
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-l2seg:{}:delete del CB {}",
                    __FUNCTION__, l2seg->seg_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_l2seg_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::pd_l2seg_delete(&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to delete l2seg pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as l2seg_delete_del_cb() was a succcess
//      a. Delete from l2seg id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI l2seg
//------------------------------------------------------------------------------
hal_ret_t
l2seg_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-l2seg:{}:delete commit CB {}",
                    __FUNCTION__, l2seg->seg_id);

    // a. Remove from l2seg id hash table
    ret = l2seg_del_from_db(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to del l2seg {} from db, err : {}", 
                      __FUNCTION__, l2seg->seg_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI l2seg
    l2seg_free(l2seg);

    // TODO: Decrement the ref counts of dependent objects
    //  - tenant

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
l2seg_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
l2seg_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a l2seg delete request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_delete (L2SegmentDeleteRequest& req, L2SegmentDeleteResponseMsg *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    l2seg_t                     *l2seg = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const L2SegmentKeyHandle    &kh = req.key_or_handle();

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

    // validate the request message
    ret = validate_l2seg_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2seg:{}:l2seg delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }


    l2seg = l2seg_lookup_key_or_handle(kh);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("pi-l2seg:{}:failed to find l2seg, id {}, handle {}",
                      __FUNCTION__, kh.segment_id(), kh.l2segment_handle());
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-l2seg:{}:deleting l2seg {}", 
                    __FUNCTION__, l2seg->seg_id);

    // form ctxt and call infra add
    dhl_entry.handle = l2seg->hal_handle;
    dhl_entry.obj = l2seg;
    cfg_ctxt.app_ctxt = NULL;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(l2seg->hal_handle, &cfg_ctxt, 
                             l2seg_delete_del_cb,
                             l2seg_delete_commit_cb,
                             l2seg_delete_abort_cb, 
                             l2seg_delete_cleanup_cb);

end:
    rsp->add_api_status(hal_prepare_rsp(ret));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a L2 segment get request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_get (L2SegmentGetRequest& req, L2SegmentGetResponse *rsp)
{
    l2seg_t    *l2seg;

    if (!req.has_meta() || req.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    auto kh = req.key_or_handle();

    if (kh.key_or_handle_case() == l2segment::L2SegmentKeyHandle::kSegmentId) {
        l2seg = find_l2seg_by_id(kh.segment_id());
    } else if (kh.key_or_handle_case() == l2segment::L2SegmentKeyHandle::kL2SegmentHandle) {
        l2seg = find_l2seg_by_handle(kh.l2segment_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (l2seg == NULL) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    // fill config spec of this L2 segment
    rsp->mutable_spec()->mutable_meta()->set_tenant_id(tenant_lookup_by_handle(l2seg->tenant_handle)->tenant_id);
    rsp->mutable_spec()->mutable_key_or_handle()->set_segment_id(l2seg->seg_id);
    rsp->mutable_spec()->set_segment_type(l2seg->segment_type);
    rsp->mutable_spec()->set_mcast_fwd_policy(l2seg->mcast_fwd_policy);
    rsp->mutable_spec()->set_bcast_fwd_policy(l2seg->bcast_fwd_policy);
    rsp->mutable_spec()->mutable_access_encap()->set_encap_type(l2seg->access_encap.type);
    rsp->mutable_spec()->mutable_access_encap()->set_encap_value(l2seg->access_encap.val);
    rsp->mutable_spec()->mutable_fabric_encap()->set_encap_type(l2seg->fabric_encap.type);
    rsp->mutable_spec()->mutable_fabric_encap()->set_encap_value(l2seg->fabric_encap.val);

    // fill operational state of this L2 segment
    rsp->mutable_status()->set_l2segment_handle(l2seg->hal_handle);

    // fill stats of this L2 segment
    rsp->mutable_stats()->set_num_endpoints(l2seg->num_ep);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Adds If into l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_add_if (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (l2seg == NULL || hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Allocate the entry
    entry = (hal_handle_id_list_entry_t *)g_hal_state->
        hal_handle_id_list_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->handle_id = hal_if->hal_handle;

    l2seg_lock(l2seg);      // lock
    // Insert into the list
    utils::dllist_add(&l2seg->if_list_head, &entry->dllist_ctxt);
    l2seg_unlock(l2seg);    // unlock

end:
    HAL_TRACE_DEBUG("pi-l2seg:{}:add if:{} to l2seg:{}, ret:{}",
                    __FUNCTION__, hal_if->if_id, l2seg->seg_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove If from l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_del_if (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr, *next;


    l2seg_lock(l2seg);      // lock
    dllist_for_each_safe(curr, next, &l2seg->if_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == hal_if->hal_handle) {
            // Remove from list
            utils::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);

            ret = HAL_RET_OK;
        }
    }
    l2seg_unlock(l2seg);    // unlock

    HAL_TRACE_DEBUG("pi-l2seg:{}:del if:{} to l2seg:{}, ret:{}",
                    __FUNCTION__, hal_if->if_id, l2seg->seg_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Hanlde nwsec update coming from tenant
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_handle_nwsec_update (l2seg_t *l2seg, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    hal_handle_id_list_entry_t  *entry = NULL;
    if_t                        *hal_if = NULL;

    if (l2seg == NULL) {
        return ret;
    }

    HAL_TRACE_DEBUG("pi-l2seg:{}:handling nwsec update seg_id: {}", 
                    __FUNCTION__, l2seg->seg_id);
    // Walk through Ifs and call respective functions
    dllist_for_each(lnode, &l2seg->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        // TODO: Uncomment this after if is migrated to new scheme
        // hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);
        hal_if = find_if_by_handle(entry->handle_id);
        if (!hal_if) {
            HAL_TRACE_ERR("pi-l2seg:{}:unable to find if with handle:{}",
                          __FUNCTION__, entry->handle_id);
            continue;
        }
        if_handle_nwsec_update(l2seg, hal_if, nwsec_prof);
    }

    return ret;
}

}    // namespace hal
