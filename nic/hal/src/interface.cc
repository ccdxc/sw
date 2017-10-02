#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/proto/hal/l2segment.pb.h"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/if_utils.hpp"
#include "nic/hal/src/rdma.hpp"
#include "nic/include/oif_list_api.hpp"

using hal::pd::pd_if_args_t;

namespace hal {

void *
if_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    if_t                        *hal_if = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    hal_if = (if_t *)hal_handle_get_obj(ht_entry->handle_id);
    HAL_ASSERT(hal_if != NULL);
    return (void *)&(hal_if->if_id);
}

uint32_t
if_id_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(if_id_t)) % ht_size;
}

bool
if_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(if_id_t *)key1 == *(if_id_t *)key2) {
        return true;
    }
    return false;
}

#if 0
//------------------------------------------------------------------------------
// insert this interface in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_if_to_db (if_t *hal_if)
{
    g_hal_state->if_id_ht()->insert(hal_if, &hal_if->ht_ctxt);
    g_hal_state->if_hwid_ht()->insert(hal_if, &hal_if->hw_ht_ctxt);
    g_hal_state->if_hal_handle_ht()->insert(hal_if,
                                            &hal_if->hal_handle_ht_ctxt);
    return HAL_RET_OK;
}
#endif

//------------------------------------------------------------------------------
// insert a if to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
if_add_to_db (if_t *hal_if, hal_handle_t handle)
{
    hal_ret_t                   ret;
    hal_handle_id_ht_entry_t    *entry;
    // if_t                        *if_temp = NULL;

    HAL_TRACE_DEBUG("pi-hal_if:{}:adding to hal_if id hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from if id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from tenant id to its handle
    entry->handle_id = handle;
    ret = g_hal_state->if_id_ht()->insert_with_key(&hal_if->if_id,
                                                   entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to add if id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    // TODO: Check if this is the right place
    hal_if->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a if from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
if_del_from_db (if_t *hal_if)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-if:{}:removing from if id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->if_id_ht()->
        remove(&hal_if->if_id);

    // free up
    g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// get if from either id or handle
//------------------------------------------------------------------------------
if_t *
if_lookup_key_or_handle (const intf::InterfaceKeyHandle& key_handle)
{
    if (key_handle.key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId) {
        return find_if_by_id(key_handle.interface_id());
    }
    if (key_handle.key_or_handle_case() == 
            intf::InterfaceKeyHandle::kIfHandle) {
        return find_if_by_handle(key_handle.if_handle());
    }

    return NULL;

}


//------------------------------------------------------------------------------
// validate an incoming interface create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_interface_create (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    intf::IfType    if_type;
    tenant_id_t     tid;
    tenant_t        *tenant = NULL;

    HAL_TRACE_DEBUG("pi-if:{}:validating if", __FUNCTION__);

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-if:{}:if id and handle not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            InterfaceKeyHandle::kInterfaceId) {
        HAL_TRACE_ERR("pi-if:{}:if id not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // interface type must be valid
    if_type = spec.type();
    if (if_type == intf::IF_TYPE_NONE) {
        HAL_TRACE_ERR("pi-if:{}:if type not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (if_type == intf::IF_TYPE_ENIC) {
        // fetch the tenant information
        tid = spec.meta().tenant_id();
        tenant = tenant_lookup_by_id(tid);
        if (tenant == NULL) {
            HAL_TRACE_ERR("pi-enicif:{}: invalid tenanit id. tenid:{}, err:{} ",
                          __FUNCTION__, tid, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
            return HAL_RET_INVALID_ARG;
        }

        // make sure ENIC info is provided
        if (!spec.has_if_enic_info()) {
            HAL_TRACE_ERR("pi-enicif:{}: no enic info. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

        // check if lif is provided
        if (!spec.if_enic_info().has_lif_key_or_handle()) {
            HAL_TRACE_ERR("pi-enicif:{}: no lif for enic. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

    } else if (if_type == intf::IF_TYPE_UPLINK) {
        // uplink specific validation
        if (!spec.has_if_uplink_info()) {
            HAL_TRACE_ERR("pi-uplinkif:{}: no uplink info. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_UPLINK_PC) {
        // uplink PC specific validation
        if (!spec.has_if_uplink_pc_info()) {
            HAL_TRACE_ERR("pi-uplinkpc:{}: no uplinkpc info. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_TUNNEL) {
        // tunnel specification validation
        if (!spec.has_if_tunnel_info()) {
            HAL_TRACE_ERR("pi-tunnelif:{}: no tunnel info. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_CPU) {
        // CPU specific validation
        if (!spec.has_if_cpu_info()) {
            HAL_TRACE_ERR("pi-cpuif:{}: no cpu if info. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        HAL_TRACE_ERR("pi-if:{}: invalid type err:{} ",
                __FUNCTION__, HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
if_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_args_t            pd_if_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if = NULL;
    // if_create_app_ctxt_t        *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (if_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-if:{}:create add CB {}",
                    __FUNCTION__, hal_if->if_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;
    ret = pd::pd_if_create(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to create if pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as if_create_add_cb() was a success
//------------------------------------------------------------------------------
hal_ret_t
if_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if = NULL;
    hal_handle_t                hal_handle = 0;
    if_create_app_ctxt_t        *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-if:{}:create commit CB {}",
                    __FUNCTION__, hal_if->if_id);


    // 1. a. Add to if id hash table
    ret = if_add_to_db(hal_if, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to add if {} to db, err : {}", 
                __FUNCTION__, hal_if->if_id, ret);
        goto end;
    }

    // If its enic, add to l2seg
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        ret = l2seg_add_if(app_ctxt->l2seg, hal_if);
        HAL_ABORT(ret == HAL_RET_OK);

        // Add to bcast list
        if (app_ctxt->l2seg) {
            // TODO: Clean this as l2seg should not have list of oifs.
            //       It should be handles.
            // Add the new interface to the broadcast list of the associated l2seg. This applies to enicifs only.
            // Its here because the multicast oif call requires the pi_if to have been created fully.
            oif_t  oif;
            // oif.if_id = hal_if->if_id;
            // oif.l2_seg_id = app_ctxt->l2seg->seg_id;
            oif.intf = hal_if;
            oif.l2seg = app_ctxt->l2seg;
            ret = oif_list_add_oif(app_ctxt->l2seg->bcast_oif_list, &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Add oif to oif_list failed, err : {}", ret);
                goto end;
            }
        }
    }

    // TODO: Increment the ref counts of dependent objects

end:
    return ret;
}

//------------------------------------------------------------------------------
// if_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI tenant 
//------------------------------------------------------------------------------
hal_ret_t
if_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_args_t            pd_if_args = { 0 };
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if = NULL;
    hal_handle_t                hal_handle = 0;
    dllist_ctxt_t               *lnode = NULL;
    dllist_ctxt_t               *curr, *next;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    hal_if = (if_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-if:{}:create abort CB {}",
                    __FUNCTION__, hal_if->if_id);

    // delete call to PD
    if (hal_if->pd_if) {
        pd::pd_if_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        ret = pd::pd_if_delete(&pd_if_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-if:{}:failed to delete if pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // members are populated in add_cb itself. So if it fails, we have to clean
    if (hal_if->if_type == intf::IF_TYPE_UPLINK_PC) {
        // if uplinkpc, clean up the member ports
        dllist_for_each_safe(curr, next, &hal_if->mbr_if_list_head) {
            entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            // Remove from list
            utils::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
        }
    }

    // remove the object
    hal_handle_free(hal_handle);

    // free PI if
    if_free(hal_if);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
if_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
if_prepare_rsp (InterfaceResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_if_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a interface create request
// TODO: if interface already exists, treat it as modify
//------------------------------------------------------------------------------
hal_ret_t
interface_create (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    l2seg_t                     *l2seg = NULL;
    if_t                        *hal_if = NULL, *hal_if1 = NULL;
    if_create_app_ctxt_t        app_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("---------------- Interface API Start ------------------");
    HAL_TRACE_DEBUG("pi-if:{}:if create for id {} Type: {} EnicType: {}",
                    __FUNCTION__, spec.key_or_handle().interface_id(),
                    spec.type(), (spec.type() == intf::IF_TYPE_ENIC) ?
                    spec.if_enic_info().enic_type() : intf::IF_ENIC_TYPE_NONE);

    // do basic validations on interface
    ret = validate_interface_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-IF:{}: Validation failed. Err: {} ApiStatus: {}",
                      __FUNCTION__, ret, rsp->api_status());
        return ret;
    }

    // check if intf exists already, and reject if one is found
    if (find_if_by_id(spec.key_or_handle().interface_id())) {
        HAL_TRACE_ERR("pi-if:{}:failed to create an if, "
                      "if {} exists already", __FUNCTION__, 
                      spec.key_or_handle().interface_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }
    HAL_TRACE_DEBUG("pi-if:{}:Done validating", __FUNCTION__);

    // allocate and initialize interface instance
    hal_if = if_alloc_init();
    if (hal_if == NULL) {
        HAL_TRACE_ERR("pi-if:{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("pi-if:{}:Done allocating.", __FUNCTION__);
    printf("allocated hal_if:%p\n", hal_if);

    // consume the config
    hal_if->if_id = spec.key_or_handle().interface_id();
    hal_if->if_type = spec.type();
    hal_if->if_admin_status = spec.admin_status();
    hal_if->if_op_status = intf::IF_STATUS_NONE;      // TODO: set this later !!

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        ret = enic_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        l2seg = find_l2seg_by_handle(hal_if->l2seg_handle);
        break;

    case intf::IF_TYPE_UPLINK:
        ret = uplink_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_uplink_to_l2seg() call
        break;

    case intf::IF_TYPE_UPLINK_PC:
        ret = uplink_pc_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_uplink_to_l2seg() call
        break;

    case intf::IF_TYPE_TUNNEL:
        ret = tunnel_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_CPU:
        ret = cpu_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;
    default:
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        goto end;
    }


    // allocate hal handle id
    hal_if->hal_handle = hal_handle_alloc(HAL_OBJ_ID_INTERFACE);
    if (hal_if->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-if:{}: failed to alloc handle {}", 
                      __FUNCTION__, hal_if->if_id);
        if_free(hal_if);
        hal_if = NULL;
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    app_ctxt.l2seg = l2seg;
    dhl_entry.handle = hal_if->hal_handle;
    dhl_entry.obj = hal_if;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(hal_if->hal_handle, &cfg_ctxt, 
                             if_create_add_cb,
                             if_create_commit_cb,
                             if_create_abort_cb, 
                             if_create_cleanup_cb);

    hal_if1 = find_if_by_handle(hal_if->hal_handle);
    HAL_ASSERT(hal_if == hal_if1);

end:
    if_prepare_rsp(rsp, ret, hal_if ? hal_if->hal_handle : 0);
    HAL_TRACE_DEBUG("------------------- Interface API End -------------------");
    return ret;
}

//------------------------------------------------------------------------------
// validate if update request
//------------------------------------------------------------------------------
hal_ret_t
validate_if_update (InterfaceSpec& spec, InterfaceResponse*rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-if:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
//------------------------------------------------------------------------------
hal_ret_t
if_make_clone (if_t *hal_if, if_t **if_clone)
{
    *if_clone = if_alloc_init();
    memcpy(*if_clone, hal_if, sizeof(if_t));

    pd::pd_if_make_clone(hal_if, *if_clone);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Enic If Update
//------------------------------------------------------------------------------
hal_ret_t
enic_if_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
                                   if_update_app_ctxt_t *app_ctxt,
                                   bool *has_changed)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Tunnel If Update
//------------------------------------------------------------------------------
hal_ret_t
tunnelif_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
                                   if_update_app_ctxt_t *app_ctxt,
                                   bool *has_changed)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// CPU If Update
//------------------------------------------------------------------------------
hal_ret_t
cpuif_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
                               if_update_app_ctxt_t *app_ctxt,
                               bool *has_changed)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Uplink If Update
//  - Handle native l2seg change
//------------------------------------------------------------------------------
hal_ret_t
uplink_if_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
                                   if_update_app_ctxt_t *app_ctxt,
                                   bool *has_changed)
{
    hal_ret_t           ret = HAL_RET_OK;
    l2seg_id_t          new_seg_id = 0;

    HAL_TRACE_DEBUG("pi-uplinkif:{}: update for if_id:{}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    HAL_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->native_l2seg != spec.if_uplink_info().native_l2segment_id()) {
        new_seg_id = spec.if_uplink_info().native_l2segment_id();
        HAL_TRACE_DEBUG("pi-uplinkif:{}: updating native_l2seg_id:{} to {}", 
                        __FUNCTION__, hal_if->native_l2seg, new_seg_id);


        app_ctxt->native_l2seg = find_l2seg_by_id(new_seg_id);
        if (!app_ctxt->native_l2seg) {
            HAL_TRACE_ERR("pi-uplinkif:{}:unable to find new l2seg:{}",
                          __FUNCTION__, new_seg_id);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        app_ctxt->native_l2seg_change = true;
        *has_changed = true;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Uplink PC Update
//  - Handle native l2seg change
//  - Handle membership changes
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
                                   if_update_app_ctxt_t *app_ctxt,
                                   bool *has_changed)
{
    hal_ret_t           ret = HAL_RET_OK;
    l2seg_id_t          new_seg_id = 0;

    HAL_TRACE_DEBUG("pi-uplinkpc:{}: update for if_id:{}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    HAL_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->native_l2seg != spec.if_uplink_pc_info().native_l2segment_id()) {
        new_seg_id = spec.if_uplink_pc_info().native_l2segment_id();
        HAL_TRACE_DEBUG("pi-uplinkpc:{}: updating native_l2seg_id:{} to {}", 
                        __FUNCTION__, hal_if->native_l2seg, new_seg_id);

        app_ctxt->native_l2seg = find_l2seg_by_id(new_seg_id);
        if (!app_ctxt->native_l2seg) {
            HAL_TRACE_ERR("pi-uplinkpc:{}:unable to find new l2seg:{}",
                          __FUNCTION__, new_seg_id);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        app_ctxt->native_l2seg_change = true;
        *has_changed = true;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// check for changes in if update
//------------------------------------------------------------------------------
hal_ret_t
if_update_check_for_change (InterfaceSpec& spec, if_t *hal_if, 
                            if_update_app_ctxt_t *app_ctxt, bool *has_changed)
{
    hal_ret_t           ret = HAL_RET_OK;

    switch (hal_if->if_type) {
        case intf::IF_TYPE_ENIC:
            ret = enic_if_update_check_for_change(spec, hal_if, app_ctxt, 
                                                  has_changed);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = uplink_if_update_check_for_change(spec, hal_if, app_ctxt, 
                                                    has_changed);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = uplink_pc_update_check_for_change(spec, hal_if, app_ctxt, 
                                                    has_changed);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = tunnelif_update_check_for_change(spec, hal_if, app_ctxt,
                                                   has_changed);
            break;
        case intf::IF_TYPE_CPU:
            ret = cpuif_update_check_for_change(spec, hal_if, app_ctxt,
                                                has_changed);
            break;

        default:
            HAL_TRACE_ERR("pi-if:{}:invalid if type: {}", __FUNCTION__, 
                          hal_if->if_type);
            ret = HAL_RET_INVALID_ARG;
    }

    return ret;
}


//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update new l2seg properties
//------------------------------------------------------------------------------
hal_ret_t
if_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_args_t            pd_if_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if = NULL;
    if_update_app_ctxt_t        *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-if:{}: update upd cb {}",
                    __FUNCTION__, hal_if->if_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;
    pd_if_args.native_l2seg_change = app_ctxt->native_l2seg_change;
    pd_if_args.native_l2seg = app_ctxt->native_l2seg;

    ret = pd::pd_if_update(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to update if pd, err : {}",
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD if .
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
if_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_args_t            pd_if_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *intf = NULL, *intf_clone;
    if_update_app_ctxt_t        *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    intf = (if_t *)dhl_entry->obj;
    intf_clone = (if_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-if:{}:update commit CB {}",
                    __FUNCTION__, intf->if_id);
    printf("Original: %p, Clone: %p\n", intf, intf_clone);

    // update clone with new attrs
    if (app_ctxt->native_l2seg_change) {
        HAL_TRACE_DEBUG("Setting the clone to new native l2seg: {}", app_ctxt->native_l2seg->seg_id);
        intf_clone->native_l2seg = app_ctxt->native_l2seg->seg_id;
    }

    // Free PD
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    ret = pd::pd_if_mem_free(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to free original if pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    if_free(intf);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
if_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_args_t            pd_if_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *intf = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    intf = (if_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-if:{}:update abort CB {}",
                    __FUNCTION__, intf->if_id);

    // Free PD
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    ret = pd::pd_if_mem_free(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to delete if pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    if_free(intf);
end:

    return ret;
}

hal_ret_t
if_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a interface update request
//------------------------------------------------------------------------------
hal_ret_t
interface_update (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    // pd_if_args_t                pd_if_args;
    const InterfaceKeyHandle    &kh = spec.key_or_handle();
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    if_update_app_ctxt_t        app_ctxt = { 0 };
    bool                        has_changed = false;

    HAL_TRACE_DEBUG("------------ Interface Update API Start ----------------");

    // validate the request message
    ret = validate_if_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:if update validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    hal_if = if_lookup_key_or_handle(kh);
    if (!hal_if) {
        HAL_TRACE_ERR("pi-if:{}:failed to find if, id {}, handle {}",
                      __FUNCTION__, kh.interface_id(), kh.if_handle());
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("pi-if:{}: if update for id {} type: {} enictype: {}",
                    __FUNCTION__, hal_if->if_id, hal_if->if_type,
                    hal_if->enic_type);

    // Check for changes
    ret = if_update_check_for_change(spec, hal_if, &app_ctxt, &has_changed);
    if (ret != HAL_RET_OK || !has_changed) {
        HAL_TRACE_ERR("pi-if:{}:no change in if update: noop", __FUNCTION__);
        goto end;
    }


    if_make_clone(hal_if, (if_t **)&dhl_entry.cloned_obj);

    printf("Cloned object: %p\n", dhl_entry.cloned_obj);

    // form ctxt and call infra update object
    dhl_entry.handle = hal_if->hal_handle;
    dhl_entry.obj = hal_if;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(hal_if->hal_handle, &cfg_ctxt, 
                             if_update_upd_cb,
                             if_update_commit_cb,
                             if_update_abort_cb, 
                             if_update_cleanup_cb);

end:
    if_prepare_rsp(rsp, ret, hal_if->hal_handle);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;

}

//------------------------------------------------------------------------------
// process a interface get request
//------------------------------------------------------------------------------
hal_ret_t
interface_get (InterfaceGetRequest& req, InterfaceGetResponse *rsp)
{
    if_t             *hal_if;
    InterfaceSpec    *spec;
    l2seg_t          *l2seg;

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    hal_if = if_lookup_key_or_handle(req.key_or_handle());
    if (!hal_if) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this interface
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_interface_id(hal_if->if_id);
    spec->set_type(hal_if->if_type);
    spec->set_admin_status(hal_if->if_admin_status);
    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
    {
        l2seg = find_l2seg_by_handle(hal_if->l2seg_handle);
        auto enic_if_info = spec->mutable_if_enic_info();
        enic_if_info->set_enic_type(hal_if->enic_type);
        enic_if_info->mutable_lif_key_or_handle()->set_lif_id(hal_if->if_id);
        enic_if_info->set_l2segment_id(l2seg->seg_id);
        enic_if_info->set_mac_address(MAC_TO_UINT64(hal_if->mac_addr));
        enic_if_info->set_encap_vlan_id(hal_if->encap_vlan);
    }
        break;

    case intf::IF_TYPE_UPLINK:
    {
        auto uplink_if_info = spec->mutable_if_uplink_info();
        uplink_if_info->set_port_num(hal_if->uplink_port_num);
        uplink_if_info->set_native_l2segment_id(hal_if->native_l2seg);
        // TODO: is this populated today ?
        //uplink_if_info->set_l2segment_id();
        // TODO: don't see this info populated in if today
        //uplink_if_info->set_rx_traffic_class_info();
    }
        break;

    case intf::IF_TYPE_UPLINK_PC:
    {
        auto uplink_pc_info = spec->mutable_if_uplink_pc_info();
        uplink_pc_info->set_uplink_pc_num(hal_if->uplink_pc_num);
        uplink_pc_info->set_native_l2segment_id(hal_if->native_l2seg);
        // TODO: is this populated today ???
        //uplink_pc_info->set_l2segment_id();
        // TODO: don't see this info populated in if_t today !!!
        //uplink_pc_info->set_rx_traffic_class_info();
        // TODO: not available in if_t today !!!
        //uplink_pc_info->set_member_if_handle();
    }
        break;

    case intf::IF_TYPE_TUNNEL:
    {
        auto tunnel_if_info = spec->mutable_if_tunnel_info();
        tunnel_if_info->set_encap_type(hal_if->encap_type);
        if (hal_if->encap_type == intf::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
            auto vxlan_info = tunnel_if_info->mutable_vxlan_info();
            ip_addr_to_spec(vxlan_info->mutable_local_tep(),
                            &hal_if->vxlan_ltep);
            ip_addr_to_spec(vxlan_info->mutable_remote_tep(),
                            &hal_if->vxlan_rtep);
        }
    }
        break;

    case intf::IF_TYPE_CPU:     // TODO: why is this exposed in API or only GET is supported ?
        break;

    default:
        break;
    }
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// fetch if from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static if_t *
fetch_if_ifl2seg (InterfaceL2SegmentSpec& spec)
{
    if (spec.if_key_handle().key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId) {
        return find_if_by_id(spec.if_key_handle().interface_id());
    } else {
        return find_if_by_handle(spec.if_key_handle().if_handle());
    }

    return NULL;
}

//------------------------------------------------------------------------------
// fetch L2 segment from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static l2seg_t *
fetch_l2seg_ifl2seg (InterfaceL2SegmentSpec& spec)
{

    if (spec.l2segment_key_or_handle().key_or_handle_case() ==
            l2segment::L2SegmentKeyHandle::kSegmentId) {
        return find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id());
    } else {
        return find_l2seg_by_handle(spec.l2segment_key_or_handle().l2segment_handle());
    }

    return NULL;
}

//------------------------------------------------------------------------------
// validate L2 segment on uplink
//------------------------------------------------------------------------------
static hal_ret_t
validate_l2seg_on_uplink (InterfaceL2SegmentSpec& spec, 
                          InterfaceL2SegmentResponse *rsp)
{
    // L2 segment key/handle is must
    if (!spec.has_l2segment_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // L2 segment has to exist
    if (spec.l2segment_key_or_handle().key_or_handle_case() == 
            l2segment::L2SegmentKeyHandle::kSegmentId && 
            !find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id())) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.l2segment_key_or_handle().key_or_handle_case() == 
            l2segment::L2SegmentKeyHandle::kL2SegmentHandle &&
            !find_l2seg_by_handle(spec.l2segment_key_or_handle().l2segment_handle())) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // uplink key/hanle is must
    if (!spec.has_if_key_handle()) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // uplink has to exist
    if (spec.if_key_handle().key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId && 
            !find_if_by_id(spec.if_key_handle().interface_id())) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.if_key_handle().key_or_handle_case() == 
            intf::InterfaceKeyHandle::kIfHandle &&
            !find_if_by_handle(spec.if_key_handle().if_handle())) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("Validation Passed");
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// add l2segment on Uplink
//------------------------------------------------------------------------------
hal_ret_t
add_l2seg_on_uplink (InterfaceL2SegmentSpec& spec,
                     InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    l2seg_t                     *l2seg = NULL;
    oif_t                       oif;
    pd::pd_l2seg_uplink_args_t  pd_l2seg_uplink_args;

    HAL_TRACE_DEBUG("---------------- Interface API Start -------------------");
    // Validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    HAL_ASSERT((hal_if != NULL) && (l2seg != NULL));

    HAL_TRACE_DEBUG("PI-AddL2SegUplink:{}: if_id:{} l2seg_id:{}", __FUNCTION__, 
            hal_if->if_id, l2seg->seg_id);

    // PD Call
    pd::pd_l2seg_uplinkif_args_init(&pd_l2seg_uplink_args);
    pd_l2seg_uplink_args.l2seg  = l2seg;
    pd_l2seg_uplink_args.intf   = hal_if;
    ret = pd::pd_add_l2seg_uplink(&pd_l2seg_uplink_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Add L2 segment on Uplink If failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // Add Uplink in l2seg
    ret = l2seg_add_if(l2seg, hal_if);
    HAL_ABORT(ret == HAL_RET_OK);

    // Add the uplink to the broadcast list of the l2seg
    // oif.if_id = hal_if->if_id;
    // oif.l2_seg_id = l2seg->seg_id;
    oif.intf = hal_if;
    oif.l2seg = l2seg;
    ret = oif_list_add_oif(l2seg->bcast_oif_list, &oif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Add oif to oif_list failed, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

cleanup:

    HAL_TRACE_DEBUG("----------------------- Interface API End --------------");
    return ret;
}

//------------------------------------------------------------------------------
// del l2segment to Uplink
//------------------------------------------------------------------------------
hal_ret_t
del_l2seg_on_uplink (InterfaceL2SegmentSpec& spec,
                     InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// CPU If Create 
//------------------------------------------------------------------------------
hal_ret_t
cpu_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
               if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif;

    HAL_TRACE_DEBUG("PI-CPUif:{}: CPUif Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    ret = get_lif_handle_for_cpu_if(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-CPUif:{}: Unable to find the lif handle Err: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    lif = find_lif_by_handle(hal_if->lif_handle);
    HAL_TRACE_DEBUG("PI-CPUif:{}: if_id:{} lif_id:{}", __FUNCTION__, 
            hal_if->if_id, lif->lif_id);

    return ret;
}



//------------------------------------------------------------------------------
// Enic If Create 
//------------------------------------------------------------------------------
hal_ret_t
enic_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    // l2seg_id_t          l2seg_id;
    l2seg_t             *l2seg;
    lif_t               *lif;

    HAL_TRACE_DEBUG("pi-enicif:{}:enicif create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    // lif for enic_if ... rsp is updated within the call
    ret = get_lif_handle_for_enic_if(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-enicif:{}:unable to find the lif handle Err: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    auto if_enic_info = spec.if_enic_info();
    hal_if->enic_type = if_enic_info.enic_type();
    hal_if->tid = spec.meta().tenant_id();
    lif = find_lif_by_handle(hal_if->lif_handle);

    if (hal_if->enic_type == intf::IF_ENIC_TYPE_USEG || 
            hal_if->enic_type == intf::IF_ENIC_TYPE_PVLAN ||
            hal_if->enic_type == intf::IF_ENIC_TYPE_DIRECT) {
        l2seg = find_l2seg_by_id(if_enic_info.l2segment_id());
        if (l2seg == NULL) {
            HAL_TRACE_ERR("pi-enicif:{}:failed to find l2seg_id:{}",
                          __FUNCTION__, if_enic_info.l2segment_id());
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        hal_if->l2seg_handle = l2seg->hal_handle;
        MAC_UINT64_TO_ADDR(hal_if->mac_addr,
                if_enic_info.mac_address());
        hal_if->encap_vlan = if_enic_info.encap_vlan_id();

        HAL_TRACE_DEBUG("pi-enicif:{}:l2_seg_id:{}, encap:{}, mac:{}, lif_id:{}", 
                        __FUNCTION__, l2seg->seg_id,
                        hal_if->encap_vlan, macaddr2str(hal_if->mac_addr),
                        lif->lif_id);

    } else {
        HAL_TRACE_ERR("pi-enicif:{}:invalid enic type: {}", __FUNCTION__, 
                        hal_if->enic_type);
        // rsp->set_api_status(types::API_STATUS_IF_ENIC_TYPE_INVALID);
        ret = HAL_RET_IF_ENIC_TYPE_INVALID;

    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Uplink If Create 
//------------------------------------------------------------------------------
hal_ret_t
uplink_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pi-uplinkif:{}:uplinkif create for id {}, "
                    "native_l2seg_id : {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id(),
                    spec.if_uplink_info().native_l2segment_id());

    // TODO: for a member port, we can have valid pc#
    ret = pltfm_get_port_from_front_port_num(spec.if_uplink_info().port_num(),
                                             &hal_if->uplink_port_num);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, HAL_RET_INVALID_ARG);

    hal_if->uplink_pc_num = HAL_PC_INVALID;
    hal_if->native_l2seg = spec.if_uplink_info().native_l2segment_id();

    return ret;
}

//-----------------------------------------------------------------------------
// Adds uplinkif into uplinkpc's member list
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_add_uplinkif (if_t *uppc, if_t *upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (uppc == NULL || upif == NULL) {
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
    entry->handle_id = upif->hal_handle;

    // Insert into the list
    utils::dllist_add(&uppc->mbr_if_list_head, &entry->dllist_ctxt);

end:
    HAL_TRACE_DEBUG("pi-uplinkpc:{}:add uplinkif_id:{} to uplinkpc_id:{}, ret:{}",
                    __FUNCTION__, uppc->if_id, upif->if_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove If from PC's member list
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_del_uplinkif (if_t *uppc, if_t *upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr, *next;


    dllist_for_each_safe(curr, next, &uppc->mbr_if_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == upif->hal_handle) {
            // Remove from list
            utils::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
            goto end;
        }
    }

    ret = HAL_RET_IF_NOT_FOUND;

end:
    HAL_TRACE_DEBUG("pi-uplinkpc:{}:del uplinkif:{} from uplinkpc:{}, reg:{}",
                    __FUNCTION__, upif->if_id, uppc->if_id, ret);
    return ret;
}

//------------------------------------------------------------------------------
// Uplink PC If Create 
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if)
{
    hal_ret_t    ret = HAL_RET_OK;
    uint64_t     mbr_if_handle = 0;
    if_t         *mbr_if = NULL;

    HAL_TRACE_DEBUG("pi-uplinkpc:{}:uplinkpc create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    hal_if->uplink_port_num = HAL_PORT_INVALID;
    hal_if->uplink_pc_num = spec.if_uplink_pc_info().uplink_pc_num();
    hal_if->native_l2seg = spec.if_uplink_pc_info().native_l2segment_id();

    HAL_TRACE_DEBUG("pi-uplinkpc:{}:adding {} no. of members", __FUNCTION__,
                    spec.if_uplink_pc_info().member_if_handle_size());
    // Walk through member uplinks
    utils::dllist_reset(&hal_if->mbr_if_list_head);
    for (int i = 0; i < spec.if_uplink_pc_info().member_if_handle_size(); i++) {
        mbr_if_handle = spec.if_uplink_pc_info().member_if_handle(i);
        mbr_if = find_if_by_handle(mbr_if_handle);
        HAL_ASSERT_RETURN(mbr_if != NULL, HAL_RET_INVALID_ARG);
        if (mbr_if->if_type != intf::IF_TYPE_UPLINK) {
            HAL_TRACE_ERR("pi-uplinkpc:{}:unable to add non-uplinkif. "
                          "Skipping if id: {}", __FUNCTION__, mbr_if->if_id);
            continue;
        }

        uplinkpc_add_uplinkif(hal_if, mbr_if);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Tunnel If Create 
//------------------------------------------------------------------------------
hal_ret_t
tunnel_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pi-tunnelif:{}:tunnelif create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());
    hal_if->tid = spec.meta().tenant_id();
    auto if_tunnel_info = spec.if_tunnel_info();
    hal_if->encap_type = if_tunnel_info.encap_type();
    /* Both Local TEP And remote TEP have to v4 or v6 */
    if ((if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
        !if_tunnel_info.vxlan_info().remote_tep().v4_addr()) || 
        (!if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
          if_tunnel_info.vxlan_info().remote_tep().v4_addr())) {
        ret = HAL_RET_IF_INFO_INVALID;
        // rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
        // rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
        goto end;
    }
    if (hal_if->encap_type ==
            intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        ip_addr_spec_to_ip_addr(&hal_if->vxlan_ltep,
                                if_tunnel_info.vxlan_info().local_tep());
        ip_addr_spec_to_ip_addr(&hal_if->vxlan_rtep,
                                if_tunnel_info.vxlan_info().remote_tep());
    } else {
        ret = HAL_RET_IF_INFO_INVALID;
        HAL_TRACE_ERR("pi-tunnelif:{}:unsupported encap type:{}",
                      __FUNCTION__, hal_if->encap_type);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// validate if delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_if_delete (InterfaceDeleteRequest& req, InterfaceDeleteResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-if:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
if_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_args_t            pd_if_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *intf = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the if. 
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    intf = (if_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-if:{}:delete del CB {}",
                    __FUNCTION__, intf->if_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    ret = pd::pd_if_delete(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to delete if pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as if_delete_del_cb() was a succcess
//      a. Delete from if id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI if
//------------------------------------------------------------------------------
hal_ret_t
if_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *intf = NULL;
    l2seg_t                     *l2seg = NULL;
    hal_handle_t                hal_handle = 0;
    oif_t                       oif = { 0 };

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    intf = (if_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-if:{}:delete commit CB {}",
                    __FUNCTION__, intf->if_id);

    // Remove from l2seg
    if (intf->if_type == intf::IF_TYPE_ENIC) {
        l2seg = find_l2seg_by_handle(intf->l2seg_handle);
        ret = l2seg_del_if(l2seg, intf);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-enicif:{}:unable to remove if from l2seg",
                          __FUNCTION__);
            goto end;
        }

        // delete oif from bcast oif list
        oif.intf = intf;
        oif.l2seg = l2seg;
        ret = oif_list_remove_oif(l2seg->bcast_oif_list, &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-enicif:{}:unable to remove if from l2seg bcast list.ret:{}",
                          __FUNCTION__, ret);
            // goto end;
        }
    }

    // a. Remove from if id hash table
    ret = if_del_from_db(intf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to del if {} from db, err : {}", 
                      __FUNCTION__, intf->if_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI if
    if_free(intf);

    // TODO: Decrement the ref counts of dependent objects
    //  - Have to decrement ref count for nwsec profile

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
if_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
if_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a if delete request
//------------------------------------------------------------------------------
hal_ret_t
interface_delete (InterfaceDeleteRequest& req, InterfaceDeleteResponseMsg *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const InterfaceKeyHandle    &kh = req.key_or_handle();

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

    // validate the request message
    ret = validate_if_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:if delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }


    hal_if = if_lookup_key_or_handle(kh);
    if (hal_if == NULL) {
        HAL_TRACE_ERR("pi-if:{}:failed to find if, id {}, handle {}",
                      __FUNCTION__, kh.interface_id(), kh.if_handle());
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-if:{}:deleting if {}", 
                    __FUNCTION__, hal_if->if_id);

    // form ctxt and call infra add
    dhl_entry.handle = hal_if->hal_handle;
    dhl_entry.obj = hal_if;
    cfg_ctxt.app_ctxt = NULL;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(hal_if->hal_handle, &cfg_ctxt, 
                             if_delete_del_cb,
                             if_delete_commit_cb,
                             if_delete_abort_cb, 
                             if_delete_cleanup_cb);

end:
    rsp->add_api_status(hal_prepare_rsp(ret));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// Get lif handle
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_for_enic_if (InterfaceSpec& spec, InterfaceResponse *rsp, 
                            if_t *hal_if)
{
    lif_id_t            lif_id = 0;
    hal_handle_t        lif_handle = 0;
    lif_t               *lif = NULL;
    hal_ret_t           ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_enic_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == intf::LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("PI-Enicif:{}: LIF handle not found for ID:{} HDL:{}",
                      __FUNCTION__, lif_id, lif_handle);
        rsp->set_api_status(types::API_STATUS_LIF_NOT_FOUND);
         ret = HAL_RET_LIF_NOT_FOUND;
         goto end;
    } else {
        hal_if->lif_handle = lif->hal_handle;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// Get lif handle for CPU If
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_for_cpu_if (InterfaceSpec& spec, InterfaceResponse *rsp, 
                          if_t *hal_if)
{
    lif_id_t        lif_id = 0;
    hal_handle_t    lif_handle = 0;
    lif_t           *lif = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_cpu_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == intf::LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("PI-CPUif:{}: LIF handle not found for ID:{} HDL:{}",
                      __FUNCTION__, lif_id, lif_handle);
        rsp->set_api_status(types::API_STATUS_LIF_NOT_FOUND);
         ret = HAL_RET_LIF_NOT_FOUND;
         goto end;
    } else {
        hal_if->lif_handle = lif->hal_handle;
    }

end:

    return ret;
}

hal_ret_t
if_handle_nwsec_update (l2seg_t *l2seg, if_t *hal_if, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_if_nwsec_upd_args_t  args;

    HAL_TRACE_DEBUG("{}: if_id: {}", __FUNCTION__, hal_if->if_id);
    pd::pd_if_nwsec_upd_args_init(&args);
    args.l2seg = l2seg;
    args.intf = hal_if;
    args.nwsec_prof = nwsec_prof;

    ret = pd::pd_if_nwsec_update(&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: PD call for nwsec update on if failed. ret: {}", 
                __FUNCTION__, ret);
        goto end;
    }

end:
    return ret;
}





}    // namespace hal
