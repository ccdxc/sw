#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/if_utils.hpp"
#include "nic/hal/src/rdma.hpp"
#include "nic/include/oif_list_api.hpp"

#define TNNL_ENC_TYPE intf::IfTunnelEncapType

namespace hal {

//------------------------------------------------------------------------------
// Get key function for if id hash table
//------------------------------------------------------------------------------
void *
if_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry = NULL;
    if_t                     *hal_if   = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    hal_if = (if_t *)hal_handle_get_obj(ht_entry->handle_id);
    HAL_ASSERT(hal_if != NULL);
    return (void *)&(hal_if->if_id);
}

//------------------------------------------------------------------------------
// Compute hash function for if id hash table
//------------------------------------------------------------------------------
uint32_t
if_id_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(if_id_t)) % ht_size;
}

//------------------------------------------------------------------------------
// Compare key function for if id hash table
//------------------------------------------------------------------------------
bool
if_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(if_id_t *)key1 == *(if_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a if to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
if_add_to_db (if_t *hal_if, hal_handle_t handle)
{
    hal_ret_t                ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t *entry;

    HAL_TRACE_DEBUG("pi-hal_if:{}:adding to hal_if id hash table", 
                    __FUNCTION__);
    // allocate hash table entry
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add if_id -> handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->if_id_ht()->insert_with_key(&hal_if->if_id,
                                                       entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to add if id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a if from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
if_del_from_db (if_t *hal_if)
{
    hal_handle_id_ht_entry_t *entry;

    HAL_TRACE_DEBUG("pi-if:{}:removing from if id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->if_id_ht()->
        remove(&hal_if->if_id);
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// get if from either id or handle
//------------------------------------------------------------------------------
if_t *
if_lookup_key_or_handle (const kh::InterfaceKeyHandle& key_handle)
{
    if (key_handle.key_or_handle_case() == 
            kh::InterfaceKeyHandle::kInterfaceId) {
        return find_if_by_id(key_handle.interface_id());
    }
    if (key_handle.key_or_handle_case() == 
            kh::InterfaceKeyHandle::kIfHandle) {
        return find_if_by_handle(key_handle.if_handle());
    }

    return NULL;

}

//------------------------------------------------------------------------------
// get if from either id or handle
//------------------------------------------------------------------------------
const char *
if_lookup_key_or_handle_to_str (const kh::InterfaceKeyHandle& key_handle)
{
	static thread_local char       if_str[4][50];
	static thread_local uint8_t    if_str_next = 0;
	char                           *buf;

	buf = if_str[if_str_next++ & 0x3];
	memset(buf, 0, 50);

    if (key_handle.key_or_handle_case() == 
            kh::InterfaceKeyHandle::kInterfaceId) {
		snprintf(buf, 50, "if_id: %lu", key_handle.interface_id());
    }
    if (key_handle.key_or_handle_case() == 
            kh::InterfaceKeyHandle::kIfHandle) {
		snprintf(buf, 50, "if_handle: 0x%lx", key_handle.if_handle());
    }

	return buf;

}

//------------------------------------------------------------------------------
// validate an incoming interface create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_interface_create (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    intf::IfType if_type;
    hal_ret_t ret = HAL_RET_OK;

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
        // if classic 
        if (spec.if_enic_info().enic_type() == intf::IF_ENIC_TYPE_CLASSIC) {
            // enic type info has to be classic
            if (spec.if_enic_info().enic_type_info_case() != 
                    intf::IfEnicInfo::ENIC_TYPE_INFO_NOT_SET &&   
                    spec.if_enic_info().enic_type_info_case() != 
                    intf::IfEnicInfo::kClassicEnicInfo) {
                // info is set but its not valid
                HAL_TRACE_ERR("pi-enicif:{}: wrong enic info being passed for "
                              "classic enic err:{}",
                              __FUNCTION__, HAL_RET_INVALID_ARG);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        } else{
            // enic type info has to be non classic
            if (spec.if_enic_info().enic_type_info_case() !=
                    intf::IfEnicInfo::ENIC_TYPE_INFO_NOT_SET &&
                    spec.if_enic_info().enic_type_info_case() != 
                    intf::IfEnicInfo::kEnicInfo) {
                // info is set but its not valid
                HAL_TRACE_ERR("pi-enicif:{}: wrong enic info being passed "
                              "for non-classic enic err:{}",
                              __FUNCTION__, HAL_RET_INVALID_ARG);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
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
        // While create we dont have to get any pc info.
        // Everything can come as update
#if 0
        // uplink PC specific validation
        if (!spec.has_if_uplink_pc_info()) {
            HAL_TRACE_ERR("pi-uplinkpc:{}: no uplinkpc info. err:{} ",
                          __FUNCTION__, HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
#endif
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
    } else if (if_type == intf::IF_TYPE_APP_REDIR) {
        // App Redirect specific validation
        if (!spec.has_if_app_redir_info() ||
            !spec.if_app_redir_info().has_lif_key_or_handle()) {
            HAL_TRACE_ERR("{}: no app redir if info. err:{} ",
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

end:
    return ret;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
if_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    pd::pd_if_create_args_t     pd_if_args = { 0 };
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if    = NULL;
    if_create_app_ctxt_t        *app_ctxt  = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-if:{}:if_id:{}:create add CB",
                    __FUNCTION__, hal_if->if_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_if_create_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;
    pd_if_args.lif = app_ctxt->lif;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_CREATE, (void *)&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to create if pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

hal_ret_t
enicif_classic_add_to_oif_lists (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    oif_t                       oif = { 0 };
    lif_t                       *lif = NULL;

    HAL_ASSERT(l2seg && hal_if);
    lif = find_lif_by_handle(hal_if->lif_handle);
    HAL_ASSERT(lif != NULL);

    oif.intf = hal_if;
    oif.l2seg = l2seg;

    if (lif->packet_filters.receive_broadcast) {
        ret = oif_list_add_oif(l2seg->bcast_oif_list, &oif);
        HAL_ASSERT(ret == HAL_RET_OK);
    }
    if (lif->packet_filters.receive_all_multicast) {
        ret = oif_list_add_oif(l2seg->bcast_oif_list + 1, &oif);
        HAL_ASSERT(ret == HAL_RET_OK);
    }
    if (lif->packet_filters.receive_promiscuous) {
        ret = oif_list_add_oif(l2seg->bcast_oif_list + 2, &oif);
        HAL_ASSERT(ret == HAL_RET_OK);
    }

    return HAL_RET_OK;
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
    if_t                        *hal_if = NULL, *uplink = NULL;
    hal_handle_t                hal_handle = 0;
    if_create_app_ctxt_t        *app_ctxt = NULL; 
    l2seg_t                     *l2seg = NULL, *nat_l2seg = NULL;
    oif_t                       oif = { 0 };

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

    HAL_TRACE_DEBUG("pi-if:{}:if_id:{}:create commit CB",
                    __FUNCTION__, hal_if->if_id);

    // Add to if id hash table
    ret = if_add_to_db(hal_if, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to add if {} to db, err : {}", 
                __FUNCTION__, hal_if->if_id, ret);
        goto end;
    }

    // If its enic, add to l2seg and lif
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        if (hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            l2seg = l2seg_lookup_by_handle(hal_if->l2seg_handle);
            // add to l2seg
            ret = l2seg_add_if(l2seg, hal_if);
            HAL_ABORT(ret == HAL_RET_OK);

            // add to lif
            ret = lif_add_if(app_ctxt->lif, hal_if);
            HAL_ABORT(ret == HAL_RET_OK);

            // Add classic nic/RxQ to bcast list only when RDMA is not enabled for this LIF
            if (l2seg && !app_ctxt->lif->enable_rdma) {
                // TODO: Clean this as l2seg should not have list of oifs.
                // It should be handles.
                // Add the new interface to the broadcast list of the associated
                // l2seg. This applies to enicifs only. Its here because the
                // multicast oif call requires the pi_if to have been created fully.
                oif.intf = hal_if;
                oif.l2seg = l2seg;
                ret = oif_list_add_oif(l2seg->bcast_oif_list, &oif);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Add oif to oif_list failed, err : {}", ret);
                    goto end;
                }
            }
        } else {

            // add to lif
            ret = lif_add_if(app_ctxt->lif, hal_if);
            HAL_ABORT(ret == HAL_RET_OK);

            // Add to uplink's back refs
            if (hal_if->pinned_uplink != HAL_HANDLE_INVALID) {
                uplink = find_if_by_handle(hal_if->pinned_uplink);
                if (uplink == NULL) {
                    HAL_TRACE_ERR("pi-enicif:{}:unable to find uplink_hdl:{}",
                                  __FUNCTION__, hal_if->pinned_uplink);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
                ret = uplink_add_enicif(uplink, hal_if);
                HAL_ASSERT(ret == HAL_RET_OK);
            }
            // Add to native l2seg's back ref
            if (hal_if->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                nat_l2seg = l2seg_lookup_by_handle(hal_if->native_l2seg_clsc);
                if (nat_l2seg == NULL) {
                    HAL_TRACE_ERR("pi-enicif:{}:unable to find native_l2seg_hdl:{}",
                                  __FUNCTION__, hal_if->native_l2seg_clsc);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
                ret = l2seg_add_if(nat_l2seg, hal_if);
                HAL_ASSERT(ret == HAL_RET_OK);
                ret = enicif_classic_add_to_oif_lists(nat_l2seg, hal_if);
                HAL_ASSERT(ret == HAL_RET_OK);
            }

            //  - Add back refs to all l2segs 
            ret = enicif_update_l2segs_relation(&hal_if->l2seg_list_clsc_head,
                                                hal_if, true);
            HAL_ASSERT(ret == HAL_RET_OK);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-enicif:{}:failed to add l2seg -> enicif "
                              "relation ret:{}", 
                              __FUNCTION__,  ret);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        }
    }

    if (hal_if->if_type == intf::IF_TYPE_UPLINK_PC) {
        // Add relation from mbr uplink if to PC
        ret = uplinkpc_update_mbrs_relation(&hal_if->mbr_if_list_head,
                                            hal_if, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-if:{}:failed to add uplinkif -> uplinkpc "
                          "relation ret:{}", 
                          __FUNCTION__,  ret);
            goto end;
        }

    }

    if (hal_if->if_type == intf::IF_TYPE_APP_REDIR) {
        g_hal_state->set_app_redir_if_id(hal_if->if_id);
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
// 3. Free PI vrf 
//------------------------------------------------------------------------------
hal_ret_t
if_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    pd::pd_if_delete_args_t     pd_if_args = { 0 };
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if    = NULL;
    hal_handle_t                hal_handle = 0;
    dllist_ctxt_t               *lnode     = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    hal_if = (if_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-if:{}:if_id:{}:create abort CB",
                    __FUNCTION__, hal_if->if_id);

    // delete call to PD
    if (hal_if->pd_if) {
        pd::pd_if_delete_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_DELETE, (void *)&pd_if_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-if:{}:failed to delete if pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // members are populated before commit_cb itself. So if it fails, we have to clean
    if (hal_if->if_type == intf::IF_TYPE_UPLINK_PC) {
        HAL_TRACE_DEBUG("pi-uplinkpc:{}:freeing up mbr and l2seg lists", 
                        __FUNCTION__);
        hal_free_handles_list(&hal_if->mbr_if_list_head);
        hal_free_handles_list(&hal_if->l2seg_list_head);

#if 0
    // dllist_ctxt_t               *curr, *next;
    // hal_handle_id_list_entry_t  *entry = NULL;
        // if uplinkpc, clean up the member ports
        dllist_for_each_safe(curr, next, &hal_if->mbr_if_list_head) {
            entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            // uplinkpc_del_uplinkif(hal_if, find_if_by_handle(entry->handle_id));
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
        }
        dllist_for_each_safe(curr, next, &hal_if->l2seg_list_head) {
            entry = dllist_entry(curr, hal_handle_id_list_entry_t,
                        dllist_ctxt);
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
        }
#endif
    }

    if (hal_if->if_type == intf::IF_TYPE_ENIC && 
            hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
        enicif_free_l2seg_entry_list(&hal_if->l2seg_list_clsc_head);
    }

    // remove the object
    hal_handle_free(hal_handle);

    // free PI if
    // if_free(hal_if);
end:
    return ret;
}

//----------------------------------------------------------------------------
// Dummy create cleanup callback
//----------------------------------------------------------------------------
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
    if (ret == HAL_RET_OK && hal_handle != 0) {
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
    // l2seg_t                     *l2seg = NULL;
    lif_t                       *lif = NULL;
    if_t                        *hal_if = NULL;
    if_create_app_ctxt_t        app_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };

    hal_api_trace(" API Begin: interface create ");
    HAL_TRACE_DEBUG("pi-if:{}:if create for id {} type: {} enictype: {}",
                    __FUNCTION__, spec.key_or_handle().interface_id(),
                    IfType_Name(spec.type()), 
                    IfEnicType_Name(spec.if_enic_info().enic_type()));

    // do basic validations on interface
    ret = validate_interface_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}: validation failed. err: {} apistatus: {}",
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

    // allocate and initialize interface instance
    hal_if = if_alloc_init();
    if (hal_if == NULL) {
        HAL_TRACE_ERR("pi-if:{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

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
        lif = find_lif_by_handle(hal_if->lif_handle);
        HAL_ASSERT(lif != NULL);
        break;

    case intf::IF_TYPE_UPLINK:
        ret = uplink_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        // will be added to broadcast list through add_l2seg_on_uplink() call
        break;

    case intf::IF_TYPE_UPLINK_PC:
        ret = uplink_pc_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_l2seg_on_uplink() call
        break;

    case intf::IF_TYPE_TUNNEL:
        ret = tunnel_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_l2seg_on_uplink() call
        break;

    case intf::IF_TYPE_CPU:
        ret = cpu_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_APP_REDIR:
        ret = app_redir_if_create(spec, rsp, hal_if);
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

    if ((hal_if->if_type == intf::IF_TYPE_TUNNEL) && 
            (hal_if->encap_type == TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_GRE)) {
        ep_t *ep;
        switch (hal_if->gre_dest.af) {
            case IP_AF_IPV4:
                ep = find_ep_by_v4_key(hal_if->tid, hal_if->gre_dest.addr.v4_addr);
                break;
            case IP_AF_IPV6:
                ep = find_ep_by_v6_key(hal_if->tid, &hal_if->gre_dest);
                break;
            default:
                HAL_TRACE_ERR("pi-tunnelif:{}:GRE tunnelif {} create Invalid AF {}", __FUNCTION__,
                    spec.key_or_handle().interface_id(), hal_if->gre_dest.af);
                ret = HAL_RET_IF_INFO_INVALID;
                goto end;
        }
        if (ep == NULL) {
            HAL_TRACE_ERR("pi-tunnelif:{}:GRE tunnelif create did not find EP {}", __FUNCTION__,
                spec.key_or_handle().interface_id());
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        ep->gre_if_handle = hal_if->hal_handle;
        HAL_TRACE_DEBUG("pi-tunnelif:{}:GRE tunnelif {} added to EP vrfId {}", __FUNCTION__,
                spec.key_or_handle().interface_id(), hal_if->tid);
    }
    // form ctxt and call infra add
    // app_ctxt.l2seg = l2seg;
    app_ctxt.lif = lif;
    dhl_entry.handle = hal_if->hal_handle;
    dhl_entry.obj = hal_if;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = HAL_RET_OK;
    ret = hal_handle_add_obj(hal_if->hal_handle, &cfg_ctxt,
                             if_create_add_cb,
                             if_create_commit_cb,
                             if_create_abort_cb, 
                             if_create_cleanup_cb);

end:
    if (ret != HAL_RET_OK && hal_if != NULL) {
        // if there is an error, if will be freed in abort CB
        if_free(hal_if);
        hal_if = NULL;
    }
    if_prepare_rsp(rsp, ret, hal_if ? hal_if->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: interface create ");
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
    pd::pd_if_make_clone_args_t args;
    

    *if_clone = if_alloc_init();

    memcpy(*if_clone, hal_if, sizeof(if_t));

    dllist_reset(&(*if_clone)->mbr_if_list_head);
    dllist_reset(&(*if_clone)->l2seg_list_clsc_head);
    dllist_reset(&(*if_clone)->l2seg_list_head);
    dllist_reset(&(*if_clone)->enicif_list_head);

    args.hal_if = hal_if;
    args.clone = *if_clone;
    pd::hal_pd_call(pd::PD_FUNC_ID_IF_MAKE_CLONE, (void *)&args);

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
    hal_ret_t   ret = HAL_RET_OK;

    auto if_enic_info = spec.if_enic_info();

    HAL_TRACE_DEBUG("pi-enicif:{}: update for if_id:{}", __FUNCTION__,
                    spec.key_or_handle().interface_id());

    HAL_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC) {
        auto clsc_enic_info = if_enic_info.mutable_classic_enic_info();
        // check of native l2seg change
        if (hal_if->native_l2seg_clsc != 
                clsc_enic_info->native_l2segment_handle()) {

            app_ctxt->new_native_l2seg_clsc = 
                clsc_enic_info->native_l2segment_handle();

            HAL_TRACE_DEBUG("pi-enicif:{}: updating native_l2seg_hdl {} => {}", 
                            __FUNCTION__, hal_if->native_l2seg_clsc, 
                            app_ctxt->new_native_l2seg_clsc);


            if (app_ctxt->new_native_l2seg_clsc != HAL_HANDLE_INVALID) {
                if (l2seg_lookup_by_handle(app_ctxt->new_native_l2seg_clsc) 
                        == NULL) {
                    HAL_TRACE_ERR("pi-enicif:{}:unable to find new "
                            "l2seg_handle:{}",
                            __FUNCTION__, app_ctxt->new_native_l2seg_clsc);
                    ret = HAL_RET_L2SEG_NOT_FOUND;
                    goto end;
                }
            } else {
                HAL_TRACE_DEBUG("pi-enicif:{}:removing native l2seg",
                                __FUNCTION__);
            }

            app_ctxt->native_l2seg_clsc_change = true;
            *has_changed = true;
        }

        // check of pinned uplink change
        if (hal_if->pinned_uplink != if_enic_info.pinned_uplink_if_handle()) {
            app_ctxt->new_pinned_uplink = if_enic_info.pinned_uplink_if_handle();
            HAL_TRACE_DEBUG("pi-enicif:{}: updating pinned uplink hdl {} => {}",
                            __FUNCTION__, hal_if->pinned_uplink,
                            app_ctxt->new_pinned_uplink);

            if (find_if_by_handle(app_ctxt->new_pinned_uplink) == NULL) {
                HAL_TRACE_ERR("pi-enicif:{}:unable to find new uplinkif_hdl:{}",
                              __FUNCTION__, app_ctxt->new_pinned_uplink);
                ret = HAL_RET_IF_NOT_FOUND;
                goto end;
            }

            app_ctxt->pinned_uplink_change = true;
            *has_changed = true;
        }

        // check for l2seg list change
        ret = enic_if_upd_l2seg_list_update(spec, hal_if, 
                                            &app_ctxt->l2segclsclist_change,
                                            &app_ctxt->add_l2segclsclist,
                                            &app_ctxt->del_l2segclsclist);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-enicif:{}:failed to check classic l2seg "
                          "list change. ret:{}",
                          __FUNCTION__, ret);
            goto end;
        }
        if (app_ctxt->l2segclsclist_change) {
            *has_changed = true;
        }
    }

end:
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
// App Redirect If Update
//------------------------------------------------------------------------------
hal_ret_t
app_redir_if_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
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
        HAL_TRACE_DEBUG("pi-uplinkif:{}: updating native_l2seg_id {} => {}", 
                        __FUNCTION__, hal_if->native_l2seg, new_seg_id);


        if (new_seg_id != HAL_L2SEGMENT_ID_INVALID) {
            app_ctxt->native_l2seg = find_l2seg_by_id(new_seg_id);
            if (!app_ctxt->native_l2seg) {
                HAL_TRACE_ERR("pi-uplinkif:{}:unable to find new l2seg:{}",
                              __FUNCTION__, new_seg_id);
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
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
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_update_check_for_change (InterfaceSpec& spec, if_t *hal_if,
                                   if_update_app_ctxt_t *app_ctxt,
                                   bool *has_changed)
{
    hal_ret_t   ret = HAL_RET_OK;
    l2seg_id_t  new_seg_id = 0;
    // uint64_t    l2seg_id = 0;
    // l2seg_t     *l2seg = NULL;

    HAL_TRACE_DEBUG("pi-uplinkpc:{}: update for if_id:{}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    HAL_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    // check of native l2seg change
    if (hal_if->native_l2seg != spec.if_uplink_pc_info().native_l2segment_id()) {
        new_seg_id = spec.if_uplink_pc_info().native_l2segment_id();
        HAL_TRACE_DEBUG("pi-uplinkpc:{}: updating native_l2seg_id {} => {}", 
                        __FUNCTION__, hal_if->native_l2seg, new_seg_id);

        if (new_seg_id != HAL_L2SEGMENT_ID_INVALID) {
            app_ctxt->native_l2seg = find_l2seg_by_id(new_seg_id);
            if (!app_ctxt->native_l2seg) {
                HAL_TRACE_ERR("pi-uplinkpc:{}:unable to find new l2seg:{}",
                              __FUNCTION__, new_seg_id);
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
        }

        app_ctxt->native_l2seg_change = true;
        *has_changed = true;
    }

    // check for mbr list change
    ret = uplinkpc_mbr_list_update(spec, hal_if, &app_ctxt->mbrlist_change,
                                   &app_ctxt->add_mbrlist,
                                   &app_ctxt->del_mbrlist,
                                   &app_ctxt->aggr_mbrlist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-uplinkpc:{}:failed to check mbr list change. ret:{}",
                      __FUNCTION__, ret);
        goto end;
    }
    if (app_ctxt->mbrlist_change) {
        *has_changed = true;
    }

#if 0
    /*
     *TODO: We should ignore the ones which are already added.
     */
    sdk::lib::dllist_reset(&hal_if->l2seg_list_head);
    for (int i = 0; i < spec.if_uplink_pc_info().l2segment_id_size(); i++) {
        l2seg_id = spec.if_uplink_pc_info().l2segment_id(i);
        l2seg = find_l2seg_by_id(l2seg_id);
        HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_INVALID_ARG);
        uplinkpc_add_l2segment(hal_if, l2seg);
        app_ctxt->l2segids_change = true;
    }
#endif

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

    if (hal_if->if_type != spec.type()) {
        HAL_TRACE_ERR("pi-if:{} Cannot change if type from {} to {} as part of "
                      "if update", __FUNCTION__, hal_if->if_type, spec.type());
        return HAL_RET_INVALID_ARG;
    }

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

        case intf::IF_TYPE_APP_REDIR:
            ret = app_redir_if_update_check_for_change(spec, hal_if, app_ctxt,
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
    hal_ret_t                   ret        = HAL_RET_OK;
    pd::pd_if_update_args_t     pd_if_args = { 0 };
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if    = NULL;
    if_update_app_ctxt_t        *app_ctxt  = NULL;

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

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        pd::pd_if_update_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        pd_if_args.native_l2seg_clsc_change = app_ctxt->native_l2seg_clsc_change;
        pd_if_args.new_native_l2seg_clsc = app_ctxt->new_native_l2seg_clsc;
        pd_if_args.pinned_uplink_change = app_ctxt->pinned_uplink_change;
        pd_if_args.new_pinned_uplink = app_ctxt->new_pinned_uplink;
        pd_if_args.l2seg_clsc_change = app_ctxt->l2segclsclist_change;
        pd_if_args.add_l2seg_clsclist = app_ctxt->add_l2segclsclist;
        pd_if_args.del_l2seg_clsclist = app_ctxt->del_l2segclsclist;
        break;
    case intf::IF_TYPE_UPLINK:
    case intf::IF_TYPE_UPLINK_PC:
        pd::pd_if_update_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        pd_if_args.native_l2seg_change = app_ctxt->native_l2seg_change;
        pd_if_args.native_l2seg = app_ctxt->native_l2seg;

        pd_if_args.mbrlist_change = app_ctxt->mbrlist_change;
        pd_if_args.add_mbrlist = app_ctxt->add_mbrlist;
        pd_if_args.del_mbrlist = app_ctxt->del_mbrlist;
        pd_if_args.aggr_mbrlist = app_ctxt->aggr_mbrlist;

        break;
    case intf::IF_TYPE_TUNNEL:
        break;
    case intf::IF_TYPE_CPU:
        break;
    case intf::IF_TYPE_APP_REDIR:
        break;
    default:
        HAL_TRACE_ERR("pi-if:{}:invalid if type: {}", __FUNCTION__, 
                      hal_if->if_type);
        ret = HAL_RET_INVALID_ARG;
    }

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_UPDATE, (void *)&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to update if pd, err : {}",
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// Update l2segs with classic enic PI
//----------------------------------------------------------------------------
hal_ret_t
enicif_update_pi_with_l2seg_list (if_t *hal_if, if_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    dllist_ctxt_t                   *curr, *next;
    if_l2seg_entry_t                *entry = NULL, *del_l2seg_entry = NULL;
    l2seg_t                         *l2seg = NULL;

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);

    dllist_for_each_safe(curr, next, app_ctxt->add_l2segclsclist) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);
        if (!l2seg) {
            HAL_TRACE_ERR("{}:unable to find l2seg with handle:{}",
                          __FUNCTION__, entry->l2seg_handle);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        // Remove entry from temp. list
        sdk::lib::dllist_del(&entry->lentry);

        // Add entry in the main list
        sdk::lib::dllist_add(&hal_if->l2seg_list_clsc_head, 
                          &entry->lentry);

        // Add the back reference in l2seg
        ret = l2seg_add_if(l2seg, hal_if);
        HAL_ASSERT(ret == HAL_RET_OK);
    }
    
    dllist_for_each_safe(curr, next, app_ctxt->del_l2segclsclist) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);
        HAL_ASSERT(l2seg != NULL);

        // Remove entry from temp. list
        sdk::lib::dllist_del(&entry->lentry);
        if (l2seg_in_classic_enicif(hal_if, entry->l2seg_handle, 
                                    &del_l2seg_entry)) {
            // Remove entry from main list
            sdk::lib::dllist_del(&del_l2seg_entry->lentry);

            // Del the back reference from l2seg
            ret = l2seg_del_if(l2seg, hal_if);
            HAL_ASSERT(ret == HAL_RET_OK);

            // free entry from temp list
            hal::delay_delete_to_slab(HAL_SLAB_ENIC_L2SEG_ENTRY,
                                      del_l2seg_entry);

            // free entry of main list
            hal::delay_delete_to_slab(HAL_SLAB_ENIC_L2SEG_ENTRY, entry);
        }
    }

end:

    // Free add & del list
    enicif_cleanup_l2seg_entry_list(&app_ctxt->add_l2segclsclist);
    enicif_cleanup_l2seg_entry_list(&app_ctxt->del_l2segclsclist);

    // Unlock if
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);

    return ret;
}

//----------------------------------------------------------------------------
// Updates Uplink PC's Pi with member list
//----------------------------------------------------------------------------
hal_ret_t
if_update_pi_with_mbr_list (if_t *hal_if, if_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    // dllist_ctxt_t                   *curr, *next;
    // hal_handle_id_list_entry_t      *entry = NULL;

    // lock if. 
    // Revisit: this is a clone and may be we dont have to take the lock
    if_lock(hal_if, __FILENAME__, __LINE__, __func__);

    // Free list in clone
    hal_free_handles_list(&hal_if->mbr_if_list_head);

    // Move aggregated list to clone
    dllist_move(&hal_if->mbr_if_list_head, app_ctxt->aggr_mbrlist);

    // add/del relations from member ports.
    ret = uplinkpc_update_mbrs_relation(app_ctxt->add_mbrlist,
                                        hal_if, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to add uplinkif -> uplinkpc "
                "relation ret:{}", 
                __FUNCTION__,  ret);
        goto end;
    }

    ret = uplinkpc_update_mbrs_relation(app_ctxt->del_mbrlist,
                                        hal_if, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to del uplinkif -/-> uplinkpc "
                "relation ret:{}", 
                __FUNCTION__,  ret);
        goto end;
    }

end:
    // Free add & del list
    interface_cleanup_handle_list(&app_ctxt->add_mbrlist);
    interface_cleanup_handle_list(&app_ctxt->del_mbrlist);
    interface_cleanup_handle_list(&app_ctxt->aggr_mbrlist);

    // Unlock if
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);

    return ret;
}


//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD if .
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
if_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret            = HAL_RET_OK;
    pd::pd_if_mem_free_args_t   pd_if_args     = { 0 };
    dllist_ctxt_t               *lnode         = NULL;
    dhl_entry_t                 *dhl_entry     = NULL;
    if_t                        *intf          = NULL, *intf_clone = NULL,
                                *old_uplink    = NULL, *new_uplink = NULL;
    if_update_app_ctxt_t        *app_ctxt      = NULL;
    // if_t                     *mbr_if        = NULL;
    l2seg_t                     *old_nat_l2seg = NULL, *new_nat_l2seg = NULL;
    uint32_t                    seg_id         = HAL_L2SEGMENT_ID_INVALID;


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


    switch (intf->if_type) {
        case intf::IF_TYPE_ENIC:
            // move lists
            dllist_move(&intf_clone->l2seg_list_clsc_head, 
                        &intf->l2seg_list_clsc_head);

            // Update clone with attrs
            if (app_ctxt->native_l2seg_clsc_change) {
                HAL_TRACE_DEBUG("Setting the classic enicif clone to new "
                                "native l2seg_hdl: {}", 
                                app_ctxt->new_native_l2seg_clsc);
                intf_clone->native_l2seg_clsc = app_ctxt->new_native_l2seg_clsc;
            }

            if (app_ctxt->pinned_uplink_change) {
                HAL_TRACE_DEBUG("Setting the classic enicif clone to new "
                                "pinned uplink_hdl: {}", 
                                app_ctxt->new_pinned_uplink);
                intf_clone->pinned_uplink = app_ctxt->new_pinned_uplink;
                // Update uplink's relation
                old_uplink = find_if_by_handle(intf->pinned_uplink);
                new_uplink = find_if_by_handle(app_ctxt->new_pinned_uplink);
                HAL_ASSERT(old_uplink != NULL && new_uplink != NULL);

                // Remove from older uplink
                ret = uplink_del_enicif(old_uplink, intf);
                HAL_ASSERT(ret == HAL_RET_OK);
                // Add to new uplink
                ret = uplink_add_enicif(new_uplink, intf);
                HAL_ASSERT(ret == HAL_RET_OK);
            }

            if (app_ctxt->native_l2seg_clsc_change) {
                HAL_TRACE_DEBUG("Setting the classic enicif clone to new "
                                "l2seg : {}",
                                app_ctxt->new_native_l2seg_clsc);
                intf_clone->native_l2seg_clsc = app_ctxt->new_native_l2seg_clsc;
                // Update native l2seg's relation
                if (intf->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                    old_nat_l2seg = l2seg_lookup_by_handle(intf->native_l2seg_clsc);
                    HAL_ASSERT(old_nat_l2seg != NULL);
                    // Remove from older nat l2seg
                    ret = l2seg_del_if(old_nat_l2seg, intf);
                    HAL_ASSERT(ret == HAL_RET_OK);
                }
                // Add to new nat l2seg
                if (app_ctxt->new_native_l2seg_clsc != HAL_HANDLE_INVALID) {
                    new_nat_l2seg = l2seg_lookup_by_handle(app_ctxt->new_native_l2seg_clsc);
                    HAL_ASSERT(new_nat_l2seg != NULL);
                    ret = l2seg_add_if(new_nat_l2seg, intf);
                    HAL_ASSERT(ret == HAL_RET_OK);
                }
            }

            if (app_ctxt->l2segclsclist_change) {
                ret = enicif_update_pi_with_l2seg_list(intf_clone, app_ctxt);
            }
            break;

        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            // move lists
            dllist_move(&intf_clone->l2seg_list_head, &intf->l2seg_list_head);
            dllist_move(&intf_clone->mbr_if_list_head, &intf->mbr_if_list_head);

            // update clone with new attrs
            if (app_ctxt->native_l2seg_change) {
                
                if (app_ctxt->native_l2seg) {
                    seg_id = app_ctxt->native_l2seg->seg_id;
                }
                HAL_TRACE_DEBUG("Setting the clone to new native l2seg: {}", 
                                seg_id);
                intf_clone->native_l2seg = seg_id;
            }

            // update mbr list, valid only for uplink pc 
            if (app_ctxt->mbrlist_change) {
                ret = if_update_pi_with_mbr_list(intf_clone, app_ctxt);
            }

            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        case intf::IF_TYPE_CPU:
            break;
        case intf::IF_TYPE_APP_REDIR:
            break;
        default:
            HAL_TRACE_ERR("pi-if:{}:invalid if type: {}", __FUNCTION__, 
                          intf->if_type);
            ret = HAL_RET_INVALID_ARG;
    }

    // Free PD
    pd::pd_if_mem_free_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_MEM_FREE, (void *)&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to free original if pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    if_free(intf);

end:
    return ret;
}

//----------------------------------------------------------------------------
// Clean up list
//----------------------------------------------------------------------------
hal_ret_t
interface_cleanup_handle_list (dllist_ctxt_t **list)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (*list == NULL) {
        return ret;
    }
    hal_free_handles_list(*list);
    HAL_FREE(HAL_MEM_ALLOC_DLLIST, *list);
    *list = NULL;

    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
if_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    pd::pd_if_mem_free_args_t   pd_if_args = { 0 };
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *intf      = NULL;
    if_update_app_ctxt_t        *app_ctxt  = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-if{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    intf = (if_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-if:{}:update abort CB {}",
                    __FUNCTION__, intf->if_id);

    // Free PD
    pd::pd_if_mem_free_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_MEM_FREE, (void *)&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:failed to delete if pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free mbr lists
    interface_cleanup_handle_list(&app_ctxt->add_mbrlist);
    interface_cleanup_handle_list(&app_ctxt->del_mbrlist);
    interface_cleanup_handle_list(&app_ctxt->aggr_mbrlist);

    // Free l2segs lists for classic enic if
    enicif_cleanup_l2seg_entry_list(&app_ctxt->add_l2segclsclist);
    enicif_cleanup_l2seg_entry_list(&app_ctxt->del_l2segclsclist);

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

    hal_api_trace(" API Begin: interface update ");

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

    HAL_TRACE_DEBUG("pi-if:{}: if update for id {} type:{} enictype:{}",
                    __FUNCTION__, hal_if->if_id, 
                    IfType_Name(hal_if->if_type), 
                    (hal_if->if_type == intf::IF_TYPE_ENIC) ?
                    IfEnicType_Name(hal_if->enic_type) : "IF_ENIC_TYPE_NONE");

    // Check for changes
    ret = if_update_check_for_change(spec, hal_if, &app_ctxt, &has_changed);
    if (ret != HAL_RET_OK || !has_changed) {
        HAL_TRACE_ERR("pi-if:{}:no change in if update: noop", __FUNCTION__);
        goto end;
    }

    if_make_clone(hal_if, (if_t **)&dhl_entry.cloned_obj);

    // form ctxt and call infra update object
    dhl_entry.handle = hal_if->hal_handle;
    dhl_entry.obj = hal_if;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(hal_if->hal_handle, &cfg_ctxt, 
                             if_update_upd_cb,
                             if_update_commit_cb,
                             if_update_abort_cb, 
                             if_update_cleanup_cb);

end:
    if_prepare_rsp(rsp, ret, hal_if ? hal_if->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: interface update ");
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

    hal_api_trace(" API Begin: interface get ");
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
    rsp->mutable_status()->set_if_handle(hal_if->hal_handle);
    spec->set_admin_status(hal_if->if_admin_status);
    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
    {
        l2seg = l2seg_lookup_by_handle(hal_if->l2seg_handle);
        auto enic_if_info = spec->mutable_if_enic_info();
        enic_if_info->set_enic_type(hal_if->enic_type);
        enic_if_info->mutable_lif_key_or_handle()->set_lif_id(hal_if->if_id);
        if (hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            enic_if_info->mutable_enic_info()->set_l2segment_id(l2seg->seg_id);
            enic_if_info->mutable_enic_info()->set_mac_address(MAC_TO_UINT64(hal_if->mac_addr));
            enic_if_info->mutable_enic_info()->set_encap_vlan_id(hal_if->encap_vlan);
        } else {
            // enic_if_info->mutable_classic_enic_info()->
        }

    }
        break;

    case intf::IF_TYPE_UPLINK:
    {
        auto uplink_if_info = spec->mutable_if_uplink_info();
        //Port number is 0 based.
        uplink_if_info->set_port_num(hal_if->uplink_port_num + 1);
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
        // uplink_pc_info->set_uplink_pc_num(hal_if->uplink_pc_num);
        uplink_pc_info->set_native_l2segment_id(hal_if->native_l2seg);
        dllist_ctxt_t *curr, *next;
        hal_handle_id_list_entry_t *entry;
        dllist_for_each_safe(curr, next, &hal_if->mbr_if_list_head) {
            entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_ERR("pi-uplinkpc:{}:READ ..unable to add non-uplinkif. "
                          "Skipping if id: {}", __FUNCTION__, entry->handle_id);
            auto mif_key_handle = uplink_pc_info->add_member_if_key_handle();
            mif_key_handle->set_interface_id(entry->handle_id);
        }
#if 0
        dllist_for_each_safe(curr, next, &hal_if->l2seg_list_head) {
            entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_ERR("pi-uplinkpc:{}:READ ..unable to add segment id "
                          "Skipping segment ID: {}", __FUNCTION__, entry->handle_id);
            l2seg_t *l2seg = l2seg_lookup_by_handle(entry->handle_id);
            if (l2seg != NULL) {
                uplink_pc_info->add_l2segment_id(l2seg->seg_id);
            }
        }
#endif
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
        } else if (hal_if->encap_type == intf::IF_TUNNEL_ENCAP_TYPE_GRE) {
            auto gre_info = tunnel_if_info->mutable_gre_info();
            ip_addr_to_spec(gre_info->mutable_source(),
                            &hal_if->gre_source);
            ip_addr_to_spec(gre_info->mutable_destination(),
                            &hal_if->gre_dest);
        }
    }
        break;

    case intf::IF_TYPE_CPU:     // TODO: why is this exposed in API or only GET is supported ?
        break;

    case intf::IF_TYPE_APP_REDIR:
        break;

    default:
        break;
    }
    hal_api_trace(" API End: interface get ");
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// fetch if from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static if_t *
fetch_if_ifl2seg (InterfaceL2SegmentSpec& spec)
{
    if (spec.if_key_handle().key_or_handle_case() == 
            kh::InterfaceKeyHandle::kInterfaceId) {
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
            kh::L2SegmentKeyHandle::kSegmentId) {
        return find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id());
    } else {
        return l2seg_lookup_by_handle(spec.l2segment_key_or_handle().l2segment_handle());
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
        HAL_TRACE_ERR("{}:no l2segment key or handle", __FUNCTION__);
        return HAL_RET_L2SEG_ID_INVALID;
    }

    // L2 segment has to exist
    if (spec.l2segment_key_or_handle().key_or_handle_case() == 
            kh::L2SegmentKeyHandle::kSegmentId &&
            !find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id())) {
        HAL_TRACE_ERR("{}:failed to find l2seg with id:{}", 
                      __FUNCTION__, 
                      spec.l2segment_key_or_handle().segment_id());
        return HAL_RET_L2SEG_NOT_FOUND;
    }
    if (spec.l2segment_key_or_handle().key_or_handle_case() == 
            kh::L2SegmentKeyHandle::kL2SegmentHandle &&
            !l2seg_lookup_by_handle(spec.l2segment_key_or_handle().l2segment_handle())) {
        HAL_TRACE_ERR("{}:failed to find l2seg with handle:{}", 
                      __FUNCTION__, 
                      spec.l2segment_key_or_handle().l2segment_handle());
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    // uplink key/hanle is must
    if (!spec.has_if_key_handle()) {
        HAL_TRACE_ERR("{}:no if key or handle", __FUNCTION__);
        return HAL_RET_INTERFACE_ID_INVALID;
    }

    // uplink has to exist
    if (spec.if_key_handle().key_or_handle_case() == 
            kh::InterfaceKeyHandle::kInterfaceId && 
            !find_if_by_id(spec.if_key_handle().interface_id())) {
        HAL_TRACE_ERR("{}:failed to find interface with id:{}", 
                      __FUNCTION__, 
                      spec.if_key_handle().interface_id());
        return HAL_RET_IF_NOT_FOUND;
    }
    if (spec.if_key_handle().key_or_handle_case() == 
            kh::InterfaceKeyHandle::kIfHandle &&
            !find_if_by_handle(spec.if_key_handle().if_handle())) {
        HAL_TRACE_ERR("{}:failed to find interface with handle:{}", 
                      __FUNCTION__, 
                      spec.if_key_handle().if_handle());
        return HAL_RET_IF_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("{}:validation passed", __FUNCTION__);
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// add l2segment on Uplink
//------------------------------------------------------------------------------
hal_ret_t
add_l2seg_on_uplink (InterfaceL2SegmentSpec& spec,
                     InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    if_t                            *hal_if = NULL;
    l2seg_t                         *l2seg = NULL;
    oif_t                           oif;
    pd::pd_add_l2seg_uplink_args_t  pd_l2seg_uplink_args;

    hal_api_trace(" API Begin: addl2seguplink ");
    // Validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    HAL_ASSERT((hal_if != NULL) && (l2seg != NULL));

    HAL_TRACE_DEBUG("pi-l2segup<->link:{}: if_id:{} <=> l2seg_id:{}", 
                    __FUNCTION__, 
                    hal_if->if_id, l2seg->seg_id);

    // PD Call
    pd::pd_add_l2seg_uplink_args_init(&pd_l2seg_uplink_args);
    pd_l2seg_uplink_args.l2seg  = l2seg;
    pd_l2seg_uplink_args.intf   = hal_if;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ADD_L2SEG_UPLINK, (void *)&pd_l2seg_uplink_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}: failed in pd, ret:{}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // Add Uplink in l2seg
    ret = l2seg_add_if(l2seg, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}:failed to add if to "
                      "l2seg's if list. ret:{}", 
                      __FUNCTION__, ret);
        goto end;
    }

    // Add l2seg in uplink
    ret = if_add_l2seg(hal_if, l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}:failed to add l2seg to "
                      "if's seg list. ret:{}", 
                      __FUNCTION__, ret);
        goto end;
    }

    // Add the uplink to the broadcast list of the l2seg
    if (is_forwarding_mode_smart_nic()) {
        oif.intf = hal_if;
        oif.l2seg = l2seg;
        ret = oif_list_add_oif(l2seg->bcast_oif_list, &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-l2segup<->link:{}:bcast oiflist failed. ret:{}",
                          __FUNCTION__, ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
    }

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: add l2seg on uplink ");
    return ret;
}

//------------------------------------------------------------------------------
// del l2segment to Uplink
//------------------------------------------------------------------------------
hal_ret_t
del_l2seg_on_uplink (InterfaceL2SegmentSpec& spec,
                     InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    l2seg_t                         *l2seg = NULL;
    if_t                            *hal_if = NULL;
    pd::pd_del_l2seg_uplink_args_t  pd_l2seg_uplink_args;

    hal_api_trace(" API Begin: delete l2seg on uplink ");

    // Validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:validation failed", __FUNCTION__);
        return ret;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    HAL_ASSERT((hal_if != NULL) && (l2seg != NULL));

    HAL_TRACE_DEBUG("pi-dell2seguplink:{}: if_id:{} <=> l2seg_id:{}", __FUNCTION__, 
            hal_if->if_id, l2seg->seg_id);

    // PD Call
    pd::pd_del_l2seg_uplink_args_init(&pd_l2seg_uplink_args);
    pd_l2seg_uplink_args.l2seg  = l2seg;
    pd_l2seg_uplink_args.intf   = hal_if;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DEL_L2SEG_UPLINK, 
                          (void *)&pd_l2seg_uplink_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}: failed in pd, ret:{}", ret);
        goto end;
    }

    // Del Uplink in l2seg
    ret = l2seg_del_if(l2seg, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}:failed to del if to "
                      "l2seg's if list. ret:{}", 
                      __FUNCTION__, ret);
        goto end;
    } 

    // Del l2seg in uplink
    ret = if_del_l2seg(hal_if, l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}:failed to del l2seg to "
                      "if's seg list. ret:{}", 
                      __FUNCTION__, ret);
        goto end;
    }

    // TODO: Del from bcast list
#if 0
    // Add the uplink to the broadcast list of the l2seg
    oif.intf = hal_if;
    oif.l2seg = l2seg;
    ret = oif_list_del_oif(l2seg->bcast_oif_list, &oif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-l2segup<->link:{}:bcast oiflist failed. ret:{}",
                      __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }
#endif

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: delete l2seg on uplink ");
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
// App Redir If Create 
//------------------------------------------------------------------------------
hal_ret_t
app_redir_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                     if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif;

    HAL_TRACE_DEBUG("{}: Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    ret = get_lif_handle_for_app_redir_if(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: Unable to find the lif handle Err: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    lif = find_lif_by_handle(hal_if->lif_handle);
    HAL_TRACE_DEBUG("{}: if_id:{} lif_id:{}", __FUNCTION__, 
                    hal_if->if_id, lif->lif_id);

    return ret;
}


//------------------------------------------------------------------------------
// Adds l2seg to the list for classic enicif
//------------------------------------------------------------------------------
hal_ret_t
enicif_classic_add_l2seg(if_t *hal_if, l2seg_t *l2seg)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_l2seg_entry_t        *l2seg_entry = NULL;

    l2seg_entry = (if_l2seg_entry_t *)g_hal_state->
                  enic_l2seg_entry_slab()->alloc();
    if (l2seg_entry  == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("pi-enicif:{}:unable to alloc memory",
                      __FUNCTION__);
        goto end;
    }
    l2seg_entry->l2seg_handle = l2seg->hal_handle;
    sdk::lib::dllist_reset(&l2seg_entry->lentry);
    sdk::lib::dllist_add(&hal_if->l2seg_list_clsc_head, &l2seg_entry->lentry);

    HAL_TRACE_DEBUG("pi-enicif:{}:L2segs:", __FUNCTION__);
    enicif_print_l2seg_entry_list(&hal_if->l2seg_list_clsc_head);

end:
    return ret;
}

//----------------------------------------------------------------------------
// Checks if l2seg is present in classic enicif
//----------------------------------------------------------------------------
bool 
l2seg_in_classic_enicif(if_t *hal_if, hal_handle_t l2seg_handle, 
                        if_l2seg_entry_t **l2seg_entry)
{
    dllist_ctxt_t                   *lnode = NULL;
    if_l2seg_entry_t                *entry = NULL;

    dllist_for_each(lnode, &(hal_if->l2seg_list_clsc_head)) {
        entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
        if (entry->l2seg_handle == l2seg_handle) {
            if (l2seg_entry) {
                *l2seg_entry = entry;
            }
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
// Prints l2seg entries handles from the list
//----------------------------------------------------------------------------
void
enicif_print_l2seg_entry_list(dllist_ctxt_t  *list)
{
    dllist_ctxt_t                   *lnode = NULL;
    if_l2seg_entry_t                *entry = NULL;

    dllist_for_each(lnode, list) {
        entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
        HAL_TRACE_DEBUG("l2seg_handle: {}", entry->l2seg_handle);
    }
}

//----------------------------------------------------------------------------
// Adds l2seg handle to if_l2seg list
//----------------------------------------------------------------------------
hal_ret_t
enicif_add_to_l2seg_entry_list(dllist_ctxt_t *handle_list, hal_handle_t handle)
{
    hal_ret_t                       ret = HAL_RET_OK;
    if_l2seg_entry_t                *entry = NULL;

    // Allocate the entry
    entry = (if_l2seg_entry_t *)g_hal_state->
            enic_l2seg_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->l2seg_handle = handle;
    // Insert into the list
    sdk::lib::dllist_add(handle_list, &entry->lentry);

end:
    return ret;
}

//----------------------------------------------------------------------------
// Free l2seg handle entries in a list. 
// - Please take locks if necessary outside this call.
//----------------------------------------------------------------------------
void
enicif_free_l2seg_entry_list(dllist_ctxt_t *list)
{
    dllist_ctxt_t                   *curr, *next;
    if_l2seg_entry_t                *entry = NULL;

    dllist_for_each_safe(curr, next, list) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        HAL_TRACE_DEBUG("{}: freeing l2seg handle: {}", __FUNCTION__, 
                        entry->l2seg_handle);
        // Remove from list
        sdk::lib::dllist_del(&entry->lentry);
        // Free the entry
        hal::delay_delete_to_slab(HAL_SLAB_ENIC_L2SEG_ENTRY, entry);
    }
}

hal_ret_t
enicif_cleanup_l2seg_entry_list(dllist_ctxt_t **list)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (*list == NULL) {
        return ret;
    }
    enicif_free_l2seg_entry_list(*list);
    HAL_FREE(HAL_MEM_ALLOC_DLLIST, *list);
    *list = NULL;

    return ret;
}

//------------------------------------------------------------------------------
// Enic If Create 
//------------------------------------------------------------------------------
hal_ret_t
enic_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    l2seg_t             *l2seg;
    lif_t               *lif;
    vrf_t               *vrf = NULL;
    L2SegmentKeyHandle  l2seg_clsc_key_handle;

    HAL_TRACE_DEBUG("pi-enicif:{}:enicif create for id {} type:{}",
                    __FUNCTION__, 
                    spec.key_or_handle().interface_id(),
                    spec.if_enic_info().enic_type());

    // lif for enic_if ... rsp is updated within the call
    ret = get_lif_handle_for_enic_if(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-enicif:{}:unable to find the lif handle Err: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    auto if_enic_info = spec.if_enic_info();
    hal_if->enic_type = if_enic_info.enic_type();
    hal_if->pinned_uplink = if_enic_info.pinned_uplink_if_handle();
    lif = find_lif_by_handle(hal_if->lif_handle);

    if (hal_if->enic_type == intf::IF_ENIC_TYPE_USEG || 
            hal_if->enic_type == intf::IF_ENIC_TYPE_PVLAN ||
            hal_if->enic_type == intf::IF_ENIC_TYPE_DIRECT) {
        l2seg = find_l2seg_by_id(if_enic_info.mutable_enic_info()->l2segment_id());
        if (l2seg == NULL) {
            HAL_TRACE_ERR("pi-enicif:{}:failed to find l2seg_id:{}",
                          __FUNCTION__, 
                          if_enic_info.mutable_enic_info()->l2segment_id());
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        hal_if->l2seg_handle = l2seg->hal_handle;

        // Fetch the vrf information from l2seg
        vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
        if(vrf == NULL ) {
            HAL_TRACE_ERR("pi-enicif:{}:failed to find vrf using handle:{}",
                           __FUNCTION__,
                           l2seg->hal_handle);
            ret = HAL_RET_VRF_NOT_FOUND;
            goto end;
        }

        hal_if->tid = vrf->vrf_id;
        MAC_UINT64_TO_ADDR(hal_if->mac_addr,
                if_enic_info.mutable_enic_info()->mac_address());
        hal_if->encap_vlan = if_enic_info.mutable_enic_info()->encap_vlan_id();

        HAL_TRACE_DEBUG("pi-enicif:{}:l2_seg_id:{}, encap:{}, mac:{}, lif_id:{}", 
                        __FUNCTION__, l2seg->seg_id,
                        hal_if->encap_vlan, macaddr2str(hal_if->mac_addr),
                        lif->lif_id);

    } else if (hal_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC) {
        auto clsc_enic_info = if_enic_info.mutable_classic_enic_info();
        if (if_enic_info.mutable_classic_enic_info()->
                native_l2segment_handle() != HAL_HANDLE_INVALID) {
            // Processing native l2seg
            l2seg = l2seg_lookup_by_handle(if_enic_info.
                    mutable_classic_enic_info()->native_l2segment_handle());
            if (l2seg == NULL) {
                HAL_TRACE_ERR("pi-enicif:{}:failed to find l2seg_handle:{}",
                              __FUNCTION__, 
                              if_enic_info.mutable_classic_enic_info()->
                              native_l2segment_handle());
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
            HAL_TRACE_DEBUG("pi-enicif:{}:Adding l2seg_id:{} as native",
                            __FUNCTION__, l2seg->seg_id);
            hal_if->native_l2seg_clsc = l2seg->hal_handle;
        }
        // Processing l2segments
        HAL_TRACE_DEBUG("pi-enicif:{}:Received {} number of l2segs",
                        __FUNCTION__, 
                        clsc_enic_info->l2segment_key_handle_size());
        sdk::lib::dllist_reset(&hal_if->l2seg_list_clsc_head);
        for (int i = 0; i < clsc_enic_info->l2segment_key_handle_size();
                i++) {
            l2seg_clsc_key_handle = clsc_enic_info->l2segment_key_handle(i);
            l2seg = l2seg_lookup_key_or_handle(l2seg_clsc_key_handle);
            if (l2seg == NULL) {
                HAL_TRACE_ERR("pi-enicif:{}:failed to find l2seg_handle:{}",
                              __FUNCTION__, 
                              l2seg_clsc_key_handle.l2segment_handle());
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
            enicif_classic_add_l2seg(hal_if, l2seg);
        }
    } else {
        HAL_TRACE_ERR("pi-enicif:{}:invalid enic type: {}", __FUNCTION__, 
                        hal_if->enic_type);
        // rsp->set_api_status(types::API_STATUS_IF_ENIC_TYPE_INVALID);
        ret = HAL_RET_IF_ENIC_TYPE_INVALID;

    }

end:
    if (ret != HAL_RET_OK) {
        // TODO: Clean up l2seg list in classic
    }
    return ret;
}


//------------------------------------------------------------------------------
// Uplink If Create 
//------------------------------------------------------------------------------
hal_ret_t
uplink_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pi-uplinkif:{}:native_l2seg_id : {}", __FUNCTION__, 
                    spec.if_uplink_info().native_l2segment_id());

    // TODO: for a member port, we can have valid pc#
    ret = pltfm_get_port_from_front_port_num(spec.if_uplink_info().port_num(),
                                             &hal_if->uplink_port_num);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, HAL_RET_INVALID_ARG);

    // hal_if->uplink_pc_num = HAL_PC_INVALID;
    hal_if->native_l2seg = spec.if_uplink_info().native_l2segment_id();

    return ret;
}

#if 0
//-----------------------------------------------------------------------------
// Adds l2segments into uplinkpc's member list
//-----------------------------------------------------------------------------
static hal_ret_t
uplinkpc_add_l2segment (if_t *uppc, l2seg_t *seg)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (uppc == NULL || seg == NULL) {
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
    entry->handle_id = seg->hal_handle;

    if_lock(uppc);      // lock
    // Insert into the list
    sdk::lib::dllist_add(&uppc->l2seg_list_head, &entry->dllist_ctxt);
    if_unlock(uppc);      // unlock

end:
    HAL_TRACE_DEBUG("pi-uplinkpc:{}:add Segment ID :{} to uplinkpc_id:{}, ret:{}",
                    __FUNCTION__, seg->seg_id, uppc->if_id, ret);
    return ret;
}
#endif

//------------------------------------------------------------------------------
// Uplink PC If Create 
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if)
{
    hal_ret_t    ret = HAL_RET_OK;
    InterfaceKeyHandle mbr_if_key_handle;
    // uint64_t     l2seg_id = 0;
    if_t         *mbr_if = NULL;
    // l2seg_t      *l2seg = NULL;

    if (!spec.has_if_uplink_pc_info()) {
        HAL_TRACE_DEBUG("pi-uplinkpc:{}: no uplinkpcinfo. not much to process",
                        __FUNCTION__);
        goto end;
    }

    HAL_TRACE_DEBUG("pi-uplinkpc:{}:native_l2seg_id : {}", __FUNCTION__, 
                    spec.if_uplink_pc_info().native_l2segment_id());

    hal_if->uplink_port_num = HAL_PORT_INVALID;
    // hal_if->uplink_pc_num = spec.if_uplink_pc_info().uplink_pc_num();
    hal_if->native_l2seg = spec.if_uplink_pc_info().native_l2segment_id();

    HAL_TRACE_DEBUG("pi-uplinkpc:{}:adding {} no. of members", __FUNCTION__,
                    spec.if_uplink_pc_info().member_if_key_handle_size());
    // Walk through member uplinks
    sdk::lib::dllist_reset(&hal_if->mbr_if_list_head);
    for (int i = 0; i < spec.if_uplink_pc_info().member_if_key_handle_size(); i++) {
        mbr_if_key_handle = spec.if_uplink_pc_info().member_if_key_handle(i);
        mbr_if = if_lookup_key_or_handle(mbr_if_key_handle);
        if (mbr_if == NULL || mbr_if->if_type != intf::IF_TYPE_UPLINK) {
            HAL_TRACE_ERR("pi-uplinkpc:{}:unable to add non-uplinkif. "
                          "Skipping if : {} , {}", __FUNCTION__, 
                          if_lookup_key_or_handle_to_str(mbr_if_key_handle), 
						  (mbr_if == NULL) ? "Not Present" :
                          "Not Uplink If");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        uplinkpc_add_uplinkif(hal_if, mbr_if);
    }

#if 0
    // Walk through l2segments.
    sdk::lib::dllist_reset(&hal_if->l2seg_list_head);
    for (int i = 0; i < spec.if_uplink_pc_info().l2segment_id_size(); i++) {
        l2seg_id = spec.if_uplink_pc_info().l2segment_id(i);
        l2seg = find_l2seg_by_id(l2seg_id);
        HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_INVALID_ARG);
        uplinkpc_add_l2segment(hal_if, l2seg);
    }
#endif

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Given a tunnel, get the remote TEP EP
// ----------------------------------------------------------------------------
ep_t *
tunnel_if_get_remote_tep_ep(if_t *pi_if)
{
    ep_t *remote_tep_ep = NULL;

    if (pi_if->encap_type == 
            TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        if (pi_if->vxlan_rtep.af == IP_AF_IPV4) {
            remote_tep_ep = find_ep_by_v4_key(pi_if->tid,
                                          pi_if->vxlan_rtep.addr.v4_addr);
        } else {
            remote_tep_ep = find_ep_by_v6_key(pi_if->tid, &pi_if->vxlan_rtep);
        }
    } else if (pi_if->encap_type == 
            TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_GRE) {
        if (pi_if->gre_dest.af == IP_AF_IPV4) {
            remote_tep_ep = find_ep_by_v4_key(pi_if->tid,
                                          pi_if->gre_dest.addr.v4_addr);
        } else {
            remote_tep_ep = find_ep_by_v6_key(pi_if->tid, &pi_if->gre_dest);
        }
    }

    return remote_tep_ep;
}



//------------------------------------------------------------------------------
// Tunnel If Create 
//------------------------------------------------------------------------------
hal_ret_t
tunnel_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if)
{
    hal_ret_t  ret      = HAL_RET_OK;
    ep_t       *rtep_ep = NULL;

    HAL_TRACE_DEBUG("pi-tunnelif:{}:tunnelif create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());
    auto if_tunnel_info = spec.if_tunnel_info();
    hal_if->tid = if_tunnel_info.vrf_key_handle().vrf_id();
    hal_if->encap_type = if_tunnel_info.encap_type();
    /* Both source addr and dest addr have to be v4 or v6 */
    if ((if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
        !if_tunnel_info.vxlan_info().remote_tep().v4_addr()) || 
        (!if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
          if_tunnel_info.vxlan_info().remote_tep().v4_addr())) {
        ret = HAL_RET_IF_INFO_INVALID;
        // rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
        // rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
        goto end;
    } else if ((if_tunnel_info.gre_info().source().v4_addr() &&
        !if_tunnel_info.gre_info().destination().v4_addr()) || 
        (!if_tunnel_info.gre_info().source().v4_addr() &&
          if_tunnel_info.gre_info().destination().v4_addr())) {
        ret = HAL_RET_IF_INFO_INVALID;
        goto end;
    }
    if (hal_if->encap_type ==
            TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        ip_addr_spec_to_ip_addr(&hal_if->vxlan_ltep,
                                if_tunnel_info.vxlan_info().local_tep());
        ip_addr_spec_to_ip_addr(&hal_if->vxlan_rtep,
                                if_tunnel_info.vxlan_info().remote_tep());
    } else if (hal_if->encap_type ==
            TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_GRE) {
        ip_addr_spec_to_ip_addr(&hal_if->gre_source,
                                if_tunnel_info.gre_info().source());
        ip_addr_spec_to_ip_addr(&hal_if->gre_dest,
                                if_tunnel_info.gre_info().destination());
        hal_if->gre_mtu = if_tunnel_info.gre_info().mtu();
        hal_if->gre_ttl = if_tunnel_info.gre_info().ttl();
    } else {
        ret = HAL_RET_IF_INFO_INVALID;
        HAL_TRACE_ERR("pi-tunnelif:{}:unsupported encap type:{}",
                      __FUNCTION__, hal_if->encap_type);
        goto end;
    }

    // Get remote tep EP
    rtep_ep = tunnel_if_get_remote_tep_ep(hal_if);
    if (!rtep_ep) {
        ret = HAL_RET_IF_INFO_INVALID;
        HAL_TRACE_ERR("pi-tunnelif:{}: unable to find rtep ep for IP: {}."
                      "ret:{}",
                      __FUNCTION__, 
                      hal_if->encap_type == 
                      TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_VXLAN ? 
                      ipaddr2str(&hal_if->vxlan_rtep) : 
                      ipaddr2str(&hal_if->gre_dest), ret);
        goto end;
    }
    hal_if->rtep_ep_handle = rtep_ep->hal_handle;

end:
    return ret;
}

//------------------------------------------------------------------------------
// validate if delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_if_delete_req (InterfaceDeleteRequest& req, InterfaceDeleteResponse *rsp)
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
// validate uplink if delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_uplinkif_delete (if_t *hal_if)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of l2segs
    if (dllist_count(&hal_if->l2seg_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("{}:l2segs still referring:", __FUNCTION__);
        hal_print_handles_list(&hal_if->l2seg_list_head);
    }

    // check if the uplink is not a member of PC
    if (hal_if->is_pc_mbr) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("{}:PC is still referring. PC's handle:{}", 
                      __FUNCTION__, hal_if->uplinkpc_handle);
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate uplink pc delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_uplinkpc_delete (if_t *hal_if)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of l2segs
    if (dllist_count(&hal_if->l2seg_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("{}:l2segs still referring:", __FUNCTION__);
        hal_print_handles_list(&hal_if->l2seg_list_head);
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate if delete 
// - check if there are any references
//------------------------------------------------------------------------------
hal_ret_t
validate_if_delete (if_t *hal_if)
{
    hal_ret_t   ret = HAL_RET_OK;

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        break;
    case intf::IF_TYPE_UPLINK:
        ret = validate_uplinkif_delete(hal_if);
        break;
    case intf::IF_TYPE_UPLINK_PC:
        ret = validate_uplinkpc_delete(hal_if);
        break;
    case intf::IF_TYPE_TUNNEL:
        break;
    case intf::IF_TYPE_CPU:
        break;
    case intf::IF_TYPE_APP_REDIR:
        break;
    default:
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("{}:invalid if type", __FUNCTION__);
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
    pd::pd_if_delete_args_t     pd_if_args = { 0 };
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
    pd::pd_if_delete_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_DELETE, (void *)&pd_if_args);
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
    if_t                        *intf = NULL, *uplink = NULL;
    l2seg_t                     *l2seg = NULL, *nat_l2seg = NULL;
    lif_t                       *lif = NULL;
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

    if (intf->if_type == intf::IF_TYPE_ENIC) {
        if (intf->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            // Remove from l2seg
            l2seg = l2seg_lookup_by_handle(intf->l2seg_handle);
            ret = l2seg_del_if(l2seg, intf);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-enicif:{}:unable to remove if from l2seg",
                              __FUNCTION__);
                goto end;
            }

            // Remove from lif
            lif = find_lif_by_handle(intf->lif_handle);
            ret = lif_del_if(lif, intf);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-enicif:{}:unable to remove if from lif",
                              __FUNCTION__);
                goto end;
            }

            // delete oif from bcast oif list
            oif.intf = intf;
            oif.l2seg = l2seg;
            ret = oif_list_remove_oif(l2seg->bcast_oif_list, &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-enicif:{}:unable to remove if from "
                              "l2seg bcast list.ret:{}",
                              __FUNCTION__, ret);
                // goto end;
            }
        } else {
            // Remove from lif
            lif = find_lif_by_handle(intf->lif_handle);
            ret = lif_del_if(lif, intf);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-enicif:{}:unable to remove if from lif",
                              __FUNCTION__);
                goto end;
            }

            // Del to uplink's back refs
            if (intf->pinned_uplink != HAL_HANDLE_INVALID) {
                uplink = find_if_by_handle(intf->pinned_uplink);
                if (uplink == NULL) {
                    HAL_TRACE_ERR("pi-enicif:{}:unable to find uplink_hdl:{}",
                                  __FUNCTION__, intf->pinned_uplink);
                    goto end;
                }
                ret = uplink_del_enicif(uplink, intf);
                HAL_ASSERT(ret == HAL_RET_OK);
            }
            // Del from native l2seg's back ref
            if (intf->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                nat_l2seg = l2seg_lookup_by_handle(intf->native_l2seg_clsc);
                if (nat_l2seg == NULL) {
                    HAL_TRACE_ERR("pi-enicif:{}:unable to find native_l2seg_hdl:{}",
                                  __FUNCTION__, intf->native_l2seg_clsc);
                    goto end;
                }
                ret = l2seg_del_if(nat_l2seg, intf);
                HAL_ASSERT(ret == HAL_RET_OK);
            }

            //  - Del back refs to all l2segs 
            ret = enicif_update_l2segs_relation(&intf->l2seg_list_clsc_head,
                                                intf, false);
            HAL_ASSERT(ret == HAL_RET_OK);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-enicif:{}:failed to del l2seg -/-> enicif "
                              "relation ret:{}", 
                              __FUNCTION__,  ret);
                goto end;
            }

            // Free up the l2seg list
            enicif_free_l2seg_entry_list(&intf->l2seg_list_clsc_head);
        }
    }

    // Uplink PC: Remove relations from mbrs
    if (intf->if_type == intf::IF_TYPE_UPLINK_PC) {
        // Del relation from mbr uplink if to PC
        ret = uplinkpc_update_mbrs_relation(&intf->mbr_if_list_head,
                                            intf, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-if:{}:failed to del uplinkif -/-> uplinkpc "
                          "relation ret:{}", 
                          __FUNCTION__,  ret);
            goto end;
        }

        // clean up mbr if list
        HAL_TRACE_DEBUG("{}:cleaning up mbr list", __FUNCTION__);
        hal_free_handles_list(&intf->mbr_if_list_head);
    }

    if (intf->if_type == intf::IF_TYPE_ENIC) {
        // Del relation from l2seg to enicifs
        ret = enicif_update_l2segs_relation(&intf->l2seg_list_clsc_head, 
                                            intf, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-if:{}:failed to del l2seg -/-> enicif "
                          "relation ret:{}", 
                          __FUNCTION__,  ret);
            goto end;
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
interface_delete (InterfaceDeleteRequest& req, InterfaceDeleteResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const InterfaceKeyHandle    &kh = req.key_or_handle();

    hal_api_trace(" API Begin: interface delete ");

    // validate the request message
    ret = validate_if_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:if delete request validation failed, ret : {}",
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
    HAL_TRACE_DEBUG("pi-if:{}: if delete for id {} type:{} enictype:{}",
                    __FUNCTION__, hal_if->if_id, 
                    IfType_Name(hal_if->if_type), 
                    (hal_if->if_type == intf::IF_TYPE_ENIC) ?
                    IfEnicType_Name(hal_if->enic_type) : "IF_ENIC_TYPE_NONE");

    ret = validate_if_delete(hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-if:{}:if delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = hal_if->hal_handle;
    dhl_entry.obj = hal_if;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(hal_if->hal_handle, &cfg_ctxt, 
                             if_delete_del_cb,
                             if_delete_commit_cb,
                             if_delete_abort_cb, 
                             if_delete_cleanup_cb);

end:
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: interface delete ");
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
    if (lif_kh.key_or_handle_case() == kh::LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("pi-enicif:{}: lif handle not found for id:{} hdl:{}",
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
    if (lif_kh.key_or_handle_case() == kh::LifKeyHandle::kLifId) {
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

//------------------------------------------------------------------------------
// Get lif handle for App Redirect If
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_for_app_redir_if (InterfaceSpec& spec, InterfaceResponse *rsp, 
                                 if_t *hal_if)
{
    lif_id_t        lif_id = 0;
    hal_handle_t    lif_handle = 0;
    lif_t           *lif = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_app_redir_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == kh::LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("{}: LIF handle not found for ID:{} HDL:{}",
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
    pd::pd_if_nwsec_update_args_t  args;

    HAL_TRACE_DEBUG("{}: if_id: {}", __FUNCTION__, hal_if->if_id);
    pd::pd_if_nwsec_update_args_init(&args);
    args.l2seg = l2seg;
    args.intf = hal_if;
    args.nwsec_prof = nwsec_prof;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_NWSEC_UPDATE, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: PD call for nwsec update on if failed. ret: {}", 
                __FUNCTION__, ret);
        goto end;
    }

end:
    return ret;
}

hal_ret_t
if_handle_lif_update (pd::pd_if_lif_update_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (args == NULL) {
        HAL_TRACE_ERR("{}:args is NULL", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("{}: if_id: {}", __FUNCTION__, args->intf->if_id);

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_LIF_UPDATE, (void *)args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: PD call for lif update on if failed. ret: {}", 
                __FUNCTION__, ret);
        goto end;
    }

end:
    return ret;
}

bool 
mbrif_in_pc (if_t *up_pc, hal_handle_t mbr_handle, hal_handle_id_list_entry_t **handle_entry)
{
    dllist_ctxt_t                   *lnode = NULL;
    hal_handle_id_list_entry_t      *entry = NULL;

    dllist_for_each(lnode, &(up_pc->mbr_if_list_head)) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == mbr_handle) {
            if (handle_entry) {
                *handle_entry = entry;
            }
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
// Handle classic enicif l2seg list change
//----------------------------------------------------------------------------
hal_ret_t
enic_if_upd_l2seg_list_update(InterfaceSpec& spec, if_t *hal_if,
                              bool *l2seglist_change,
                              dllist_ctxt_t **add_l2seglist, 
                              dllist_ctxt_t **del_l2seglist)
{
    hal_ret_t                       ret = HAL_RET_OK;
    uint16_t                        num_l2segs = 0, i = 0;
    dllist_ctxt_t                   *lnode = NULL;
    // ep_ip_entry_t                   *pi_ip_entry = NULL;
    bool                            l2seg_exists = false;
    L2SegmentKeyHandle              l2seg_key_handle;
    l2seg_t                         *l2seg = NULL;
    if_l2seg_entry_t                *entry = NULL, *lentry = NULL;

    *l2seglist_change = false;

    auto if_enic_info = spec.if_enic_info();
    auto clsc_enic_info = if_enic_info.mutable_classic_enic_info();

    *add_l2seglist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST, 
                                               sizeof(dllist_ctxt_t));
    HAL_ABORT(*add_l2seglist != NULL);
    *del_l2seglist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST, 
                                               sizeof(dllist_ctxt_t));
    HAL_ABORT(*del_l2seglist != NULL);

    sdk::lib::dllist_reset(*add_l2seglist);
    sdk::lib::dllist_reset(*del_l2seglist);

    num_l2segs = clsc_enic_info->l2segment_key_handle_size();
    HAL_TRACE_DEBUG("pi-enicif:{}:number of l2segs:{}", 
                    __FUNCTION__, num_l2segs);
    for (i = 0; i < num_l2segs; i++) {
        l2seg_key_handle = clsc_enic_info->l2segment_key_handle(i);
        l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);
        HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_INVALID_ARG);

        if (l2seg_in_classic_enicif(hal_if, l2seg->hal_handle, NULL)) {
            continue;
        } else {
            // Add to added list
            enicif_add_to_l2seg_entry_list(*add_l2seglist, l2seg->hal_handle);
            *l2seglist_change = true;
            HAL_TRACE_DEBUG("pi-enicif:{}: added to add list hdl: {}", 
                    __FUNCTION__, l2seg->hal_handle);
        }
    }

    HAL_TRACE_DEBUG("{}:Existing l2segs:", __FUNCTION__);
    enicif_print_l2seg_entry_list(&hal_if->l2seg_list_head);
    HAL_TRACE_DEBUG("{}:added l2segs:", __FUNCTION__);
    enicif_print_l2seg_entry_list(*add_l2seglist);

    dllist_for_each(lnode, &(hal_if->l2seg_list_clsc_head)) {
        entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
        HAL_TRACE_DEBUG("pi-enicif:{}: Checking for l2seg: {}", 
                __FUNCTION__, entry->l2seg_handle);
        for (i = 0; i < num_l2segs; i++) {
            l2seg_key_handle = clsc_enic_info->l2segment_key_handle(i);
            HAL_TRACE_DEBUG("{}:grpc l2seg handle: {}", __FUNCTION__, l2seg->hal_handle);
            if (entry->l2seg_handle == l2seg_key_handle.l2segment_handle()) {
                l2seg_exists = true;
                break;
            } else {
                continue;
            }
        }
        if (!l2seg_exists) {
            // Have to delet the mbr
            lentry = (if_l2seg_entry_t *)g_hal_state->
                enic_l2seg_entry_slab()->alloc();
            if (lentry == NULL) {
                ret = HAL_RET_OOM;
                goto end;
            }
            lentry->l2seg_handle = entry->l2seg_handle;
            lentry->pd = entry->pd;

            // Insert into the list
            sdk::lib::dllist_add(*del_l2seglist, &lentry->lentry);
            *l2seglist_change = true;
            HAL_TRACE_DEBUG("pi-enicif:{}: added to delete list hdl: {}", 
                    __FUNCTION__, lentry->l2seg_handle);
        }
        l2seg_exists = false;
    }

    HAL_TRACE_DEBUG("{}:deleted l2segs:", __FUNCTION__);
    enicif_print_l2seg_entry_list(*del_l2seglist);

    if (!*l2seglist_change) {
        // Got same mbrs as existing
        enicif_cleanup_l2seg_entry_list(add_l2seglist);
        enicif_cleanup_l2seg_entry_list(del_l2seglist);
    }
end:
    return ret;
}

//----------------------------------------------------------------------------
// Handle uplink pc mbr list update
//----------------------------------------------------------------------------
hal_ret_t
uplinkpc_mbr_list_update(InterfaceSpec& spec, if_t *hal_if,
                         bool *mbrlist_change,
                         dllist_ctxt_t **add_mbrlist, 
                         dllist_ctxt_t **del_mbrlist,
                         dllist_ctxt_t **aggr_mbrlist)
{
    hal_ret_t                       ret = HAL_RET_OK;
    uint16_t                        num_mbrs = 0, i = 0;
    dllist_ctxt_t                   *lnode = NULL;
    // ep_ip_entry_t                   *pi_ip_entry = NULL;
    bool                            mbr_exists = false;
    InterfaceKeyHandle              mbr_if_key_handle;
    if_t                            *mbr_if = NULL;
    hal_handle_id_list_entry_t      *entry = NULL, *lentry = NULL;

    *mbrlist_change = false;

    *add_mbrlist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST, 
                                               sizeof(dllist_ctxt_t));
    HAL_ABORT(*add_mbrlist != NULL);
    *del_mbrlist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST, 
                                               sizeof(dllist_ctxt_t));
    HAL_ABORT(*del_mbrlist != NULL);
    *aggr_mbrlist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST,
                                                sizeof(dllist_ctxt_t));
    HAL_ABORT(*aggr_mbrlist != NULL);

    sdk::lib::dllist_reset(*add_mbrlist);
    sdk::lib::dllist_reset(*del_mbrlist);
    sdk::lib::dllist_reset(*aggr_mbrlist);

    num_mbrs = spec.if_uplink_pc_info().member_if_key_handle_size();
    HAL_TRACE_DEBUG("pi-if:{}:pc mbrs:{}", 
                    __FUNCTION__, num_mbrs);
    for (i = 0; i < num_mbrs; i++) {
        mbr_if_key_handle = spec.if_uplink_pc_info().member_if_key_handle(i);
        mbr_if = if_lookup_key_or_handle(mbr_if_key_handle);
        HAL_ASSERT_RETURN(mbr_if != NULL, HAL_RET_INVALID_ARG);

        // Add to aggregated list
        hal_add_to_handle_list(*aggr_mbrlist, mbr_if->hal_handle);

        if (mbr_if->if_type != intf::IF_TYPE_UPLINK) {
            HAL_TRACE_ERR("pi-uplinkpc:{}:unable to add non-uplinkif. "
                          "Skipping if id: {}", __FUNCTION__, mbr_if->if_id);
            continue;
        }
        if (mbrif_in_pc(hal_if, mbr_if->hal_handle, NULL)) {
            continue;
        } else {
            // Add to added list
            hal_add_to_handle_list(*add_mbrlist, mbr_if->hal_handle);
            *mbrlist_change = true;
            HAL_TRACE_DEBUG("pi-uplinkpc:{}: added to add list hdl: {}", 
                    __FUNCTION__, mbr_if->hal_handle);
        }
    }

    HAL_TRACE_DEBUG("{}:Existing mbrs:", __FUNCTION__);
    hal_print_handles_list(&hal_if->mbr_if_list_head);
    HAL_TRACE_DEBUG("{}:New Aggregated mbrs:", __FUNCTION__);
    hal_print_handles_list(*aggr_mbrlist);
    HAL_TRACE_DEBUG("{}:added mbrs:", __FUNCTION__);
    hal_print_handles_list(*add_mbrlist);

    dllist_for_each(lnode, &(hal_if->mbr_if_list_head)) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        HAL_TRACE_DEBUG("pi-uplinkpc:{}: Checking for mbr: {}", 
                __FUNCTION__, entry->handle_id);
        for (i = 0; i < num_mbrs; i++) {
            mbr_if_key_handle = spec.if_uplink_pc_info().member_if_key_handle(i);
            mbr_if = if_lookup_key_or_handle(mbr_if_key_handle);
            HAL_TRACE_DEBUG("{}:grpc mbr handle: {}", __FUNCTION__, mbr_if->hal_handle);
            if (entry->handle_id == mbr_if->hal_handle) {
                mbr_exists = true;
                break;
            } else {
                continue;
            }
        }
        if (!mbr_exists) {
            // Have to delete the mbr
            lentry = (hal_handle_id_list_entry_t *)g_hal_state->
                hal_handle_id_list_entry_slab()->alloc();
            if (lentry == NULL) {
                ret = HAL_RET_OOM;
                goto end;
            }
            lentry->handle_id = entry->handle_id;

            // Insert into the list
            sdk::lib::dllist_add(*del_mbrlist, &lentry->dllist_ctxt);
            *mbrlist_change = true;
            HAL_TRACE_DEBUG("pi-uplinkpc:{}: added to delete list hdl: {}", 
                    __FUNCTION__, lentry->handle_id);
        }
        mbr_exists = false;
    }

    HAL_TRACE_DEBUG("{}:deleted mbrs:", __FUNCTION__);
    hal_print_handles_list(*del_mbrlist);

    if (!*mbrlist_change) {
        // Got same mbrs as existing
        interface_cleanup_handle_list(add_mbrlist);
        interface_cleanup_handle_list(del_mbrlist);
        interface_cleanup_handle_list(aggr_mbrlist);
    }
end:
    return ret;
}

//----------------------------------------------------------------------------
// Add/Del relation l2seg -> enicif for all l2segs in the list
//----------------------------------------------------------------------------
hal_ret_t
enicif_update_l2segs_relation (dllist_ctxt_t *l2segs_list, if_t *hal_if, bool add)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *curr, *next;
    if_l2seg_entry_t            *entry = NULL;
    l2seg_t                     *l2seg = NULL;

    dllist_for_each_safe(curr, next, l2segs_list) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);
        if (!l2seg) {
            HAL_TRACE_ERR("{}:unable to find l2seg with handle:{}",
                          __FUNCTION__, entry->l2seg_handle);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }
        if (add) {
            ret = l2seg_add_if(l2seg, hal_if);
            if (ret == HAL_RET_OK) {
                ret = enicif_classic_add_to_oif_lists(l2seg, hal_if);
            }
        } else {
            ret = l2seg_del_if(l2seg, hal_if);
        }
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// Add/Del relation uplinkif -> uplinkpc for all mbrs in the list
//----------------------------------------------------------------------------
hal_ret_t
uplinkpc_update_mbrs_relation (dllist_ctxt_t *mbr_list, if_t *uppc, bool add)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *curr, *next;
    hal_handle_id_list_entry_t  *entry = NULL;
    if_t                        *up_if = NULL;

    dllist_for_each_safe(curr, next, mbr_list) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        up_if = find_if_by_handle(entry->handle_id);
        if (!up_if) {
            HAL_TRACE_ERR("{}:unable to find uplinkif with handle:{}",
                          __FUNCTION__, entry->handle_id);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }
        if (add) {
            ret = uplinkif_add_uplinkpc(up_if, uppc);
        } else {
            ret = uplinkif_del_uplinkpc(up_if, uppc);
        }
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// Add relation uplinkif -> uplinkpc
//----------------------------------------------------------------------------
hal_ret_t
uplinkif_add_uplinkpc (if_t *upif, if_t *uppc)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (upif == NULL || uppc == NULL) {
        HAL_TRACE_ERR("{}: invalid args", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(upif, __FILENAME__, __LINE__, __func__);

    upif->is_pc_mbr = true;
    upif->uplinkpc_handle = uppc->hal_handle;

    if_unlock(upif, __FILENAME__, __LINE__, __func__);

end:

    HAL_TRACE_DEBUG("{}: add uplinkif => uplinkpc , {} => {}",
                    upif->if_id, uppc->if_id);
    return ret;
}

//----------------------------------------------------------------------------
// Del relation uplinkif -/-> uplinkpc
//----------------------------------------------------------------------------
hal_ret_t
uplinkif_del_uplinkpc (if_t *upif, if_t *uppc)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (upif == NULL || uppc == NULL) {
        HAL_TRACE_ERR("{}: invalid args", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(upif, __FILENAME__, __LINE__, __func__);

    upif->is_pc_mbr = false;
    upif->uplinkpc_handle = HAL_HANDLE_INVALID;

    if_unlock(upif, __FILENAME__, __LINE__, __func__);


end:
    HAL_TRACE_DEBUG("{}: del uplinkif =/=> uplinkpc , {} =/=> {}",
                    __FUNCTION__, upif->if_id, uppc->if_id);
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

    if_lock(uppc, __FILENAME__, __LINE__, __func__);          // lock
    // Insert into the list
    sdk::lib::dllist_add(&uppc->mbr_if_list_head, &entry->dllist_ctxt);
    if_unlock(uppc, __FILENAME__, __LINE__, __func__);      // unlock

end:
    HAL_TRACE_DEBUG("pi-uplinkpc:{}: add uplinkpc => uplinkif, {} => {}, ret:{}",
                    __FUNCTION__, uppc->if_id, upif->if_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove UplinkIf from Uplink PC's member list
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_del_uplinkif (if_t *uppc, if_t *upif)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr, *next;

    if_lock(uppc, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &uppc->mbr_if_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == upif->hal_handle) {
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);
            ret = HAL_RET_OK;
        }
    }
    if_unlock(uppc, __FILENAME__, __LINE__, __func__);    // unlock

    HAL_TRACE_DEBUG("pi-uplinkpc:{}: del uplinkpc =/=> uplinkif, {} =/=> {}, ret:{}",
                    __FUNCTION__, uppc->if_id, upif->if_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Adds l2segs into if list
//-----------------------------------------------------------------------------
hal_ret_t
if_add_l2seg (if_t *hal_if, l2seg_t *l2seg)
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
    entry->handle_id = l2seg->hal_handle;

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    // Insert into the list
    sdk::lib::dllist_add(&hal_if->l2seg_list_head, &entry->dllist_ctxt);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock

end:
    HAL_TRACE_DEBUG("pi-if:{}: add if => l2seg, {} => {}, ret:{}",
                    __FUNCTION__, hal_if->if_id, l2seg->seg_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove l2seg from if list
//-----------------------------------------------------------------------------
hal_ret_t
if_del_l2seg (if_t *hal_if, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;


    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &hal_if->l2seg_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == l2seg->hal_handle) {
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);
            ret = HAL_RET_OK;
        }
    }
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock

    HAL_TRACE_DEBUG("pi-if:{}: del if =/=> l2seg, {} =/=> {}, ret:{}",
                    __FUNCTION__, hal_if->if_id, l2seg->seg_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Adds enics into if list of uplinks
//-----------------------------------------------------------------------------
hal_ret_t
uplink_add_enicif (if_t *uplink, if_t *enic_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (uplink == NULL || enic_if == NULL) {
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
    entry->handle_id = enic_if->hal_handle;

    if_lock(enic_if, __FILENAME__, __LINE__, __func__);      // lock
    // Insert into the list
    sdk::lib::dllist_add(&uplink->enicif_list_head, &entry->dllist_ctxt);
    if_unlock(enic_if, __FILENAME__, __LINE__, __func__);    // unlock

end:
    HAL_TRACE_DEBUG("pi-if:{}: add uplink => enic_if, {} => {}, ret:{}",
                    __FUNCTION__, uplink->if_id, enic_if->if_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove enicif from if list of uplink
//-----------------------------------------------------------------------------
hal_ret_t
uplink_del_enicif (if_t *uplink, if_t *enic_if)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;

    if_lock(uplink, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &uplink->enicif_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == enic_if->hal_handle) {
            // remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);
            ret = HAL_RET_OK;
        }
    }
    if_unlock(uplink, __FILENAME__, __LINE__, __func__);    // unlock

    HAL_TRACE_DEBUG("pi-if:{}: del uplink =/=> enic_if, {} =/=> {}, ret:{}",
                    __FUNCTION__, uplink->if_id, enic_if->if_id, ret);
    return ret;
}

}    // namespace hal
