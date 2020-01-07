//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for all interfaces
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"
#include "nic/hal/plugins/cfg/nw/nh.hpp"
#include "nic/hal/plugins/cfg/nw/filter.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/port.grpc.pb.h"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/plugins/cfg/mcast/oif_list_api.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/linkmgr/linkmgr_ipc.hpp"
#include "nic/sdk/linkmgr/linkmgr_internal.hpp"
#include "nic/include/fte.hpp"
#include "nic/linkmgr/linkmgr_utils.hpp"
#include "nic/sdk/include/sdk/if.hpp"

#define TNNL_ENC_TYPE intf::IfTunnelEncapType

namespace hal {

static inline hal_ret_t
hal_stream_port_status_update (port_event_info_t port_event) {
    
    auto walk_cb = [](uint32_t event_id, void *entry, void *ctxt) {
        grpc::ServerWriter<EventResponse> *stream =
                 (grpc::ServerWriter<EventResponse> *)ctxt;
        port_event_info_t *port_event = (port_event_info_t *)entry;
        EventResponse   evtresponse;
        auto status = evtresponse.mutable_port_event()->mutable_status();
        auto link_status = status->mutable_link_status();

        status->mutable_key_or_handle()->set_port_id(port_event->logical_port);
        // Set link status
        link_status->set_oper_state(
                     linkmgr::sdk_port_oper_st_to_port_oper_st_spec(port_event->oper_status));
        link_status->set_port_speed(
                     linkmgr::sdk_port_speed_to_port_speed_spec(port_event->speed));
        link_status->set_auto_neg_enable(port_event->auto_neg_enable);
        link_status->set_num_lanes(port_event->num_lanes); 
        evtresponse.set_event_id(::event::EVENT_ID_PORT_STATE);
        
        evtresponse.set_api_status(::types::ApiStatus::API_STATUS_OK);
        stream->Write(evtresponse);
        return true;
    };
    g_hal_state->event_mgr()->notify_event(::event::EVENT_ID_PORT_STATE, 
                                          (void *)&port_event, walk_cb);

    return HAL_RET_OK;
}

uint8_t g_num_uplink_ifs = 0;
std::vector<uint8_t > g_uplink_if_ids;

hal_ret_t
if_prepare_rsp (InterfaceResponse *rsp, hal_ret_t ret,
                hal_handle_t hal_handle, hal_handle_t uplink_handle = HAL_HANDLE_INVALID);

//------------------------------------------------------------------------------
// Get key function for if id hash table
//------------------------------------------------------------------------------
void *
if_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry = NULL;
    if_t                     *hal_if   = NULL;

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    hal_if = (if_t *)hal_handle_get_obj(ht_entry->handle_id);
    return hal_if ? (void *)&(hal_if->if_id) : NULL;
}


//------------------------------------------------------------------------------
// Compute hash table key size
//------------------------------------------------------------------------------
uint32_t
if_id_key_size ()
{
    return sizeof(if_id_t);
}

// allocate a interface instance
static inline if_t *
if_alloc (void)
{
    if_t    *hal_if;

    hal_if = (if_t *)g_hal_state->if_slab()->alloc();
    if (hal_if == NULL) {
        return NULL;
    }
    return hal_if;
}

// initialize a interface instance
static inline if_t *
if_init (if_t *hal_if)
{
    if (!hal_if) {
        return NULL;
    }
    memset(hal_if, 0, sizeof(if_t));
    SDK_SPINLOCK_INIT(&hal_if->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    hal_if->num_ep            = 0;
    hal_if->enic_type         = intf::IF_ENIC_TYPE_NONE;
    hal_if->pd_if             = NULL;
    hal_if->hal_handle        = HAL_HANDLE_INVALID;
    hal_if->lif_handle        = HAL_HANDLE_INVALID;
    hal_if->l2seg_handle      = HAL_HANDLE_INVALID;
    hal_if->native_l2seg_clsc = HAL_HANDLE_INVALID;
    hal_if->pinned_uplink     = HAL_HANDLE_INVALID;
    hal_if->is_pc_mbr         = false;
    hal_if->uplinkpc_handle   = HAL_HANDLE_INVALID;

    hal_if->mbr_if_list     = block_list::factory(sizeof(hal_handle_t));
    hal_if->l2seg_list      = block_list::factory(sizeof(hal_handle_t));
    hal_if->enicif_list     = block_list::factory(sizeof(hal_handle_t));
    hal_if->nh_list         = block_list::factory(sizeof(hal_handle_t));
    hal_if->ep_list         = block_list::factory(sizeof(hal_handle_t));
    sdk::lib::dllist_reset(&hal_if->l2seg_list_clsc_head);
    sdk::lib::dllist_reset(&hal_if->mc_entry_list_head);

    for (unsigned i = 0; i < SDK_ARRAY_SIZE(hal_if->acl_list); i++) {
        hal_if->acl_list[i]= block_list::factory(sizeof(hal_handle_t));
    }

    return hal_if;
}

// allocate and initialize a interface instance
static inline if_t *
if_alloc_init (void)
{
    return if_init(if_alloc());
}

static inline hal_ret_t
if_free (if_t *hal_if)
{
    hal::delay_delete_to_slab(HAL_SLAB_IF, hal_if);
    return HAL_RET_OK;
}

static inline hal_ret_t
if_cleanup (if_t *hal_if)
{
    block_list::destroy(hal_if->mbr_if_list);
    block_list::destroy(hal_if->l2seg_list);
    block_list::destroy(hal_if->enicif_list);
    block_list::destroy(hal_if->nh_list);
    block_list::destroy(hal_if->ep_list);
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(hal_if->acl_list); i++) {
        block_list::destroy(hal_if->acl_list[i]);
    }
    SDK_SPINLOCK_DESTROY(&hal_if->slock);
    return if_free(hal_if);
}

hal_handle_id_ht_entry_t *
find_handle_obj_by_if_id (if_id_t if_id)
{
    hal_handle_id_ht_entry_t    *entry = NULL;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        if_id_ht()->lookup(&if_id);

    return entry;
}

hal_handle_t
find_hal_handle_from_if_id (if_id_t if_id)
{
    hal_handle_id_ht_entry_t    *entry = NULL;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        if_id_ht()->lookup(&if_id);
    if (entry) {
        return entry->handle_id;
    }

    return HAL_HANDLE_INVALID;
}

if_id_t
find_if_id_from_hal_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return 0;
    }

    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle : {}", handle);
        return 0;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_INTERFACE) {
        HAL_TRACE_DEBUG("Failed to find if with handle : {}", handle);
        return 0;
    }

    return ((if_t *)hal_handle_get_obj(handle))->if_id;
}

if_t *
find_if_by_id (if_id_t if_id)
{
    hal_handle_id_ht_entry_t    *entry;
    if_t                        *hal_if;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        if_id_ht()->lookup(&if_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {

        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_INTERFACE);

        hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);
        return hal_if;
    }
    return NULL;
}

if_t *
find_if_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle : {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_INTERFACE) {
        HAL_TRACE_DEBUG("Failed to find if with handle : {}", handle);
        return NULL;
    }

    return (if_t *)hal_handle_get_obj(handle);

#if 0
    // TODO: hal_handle can be NULL if there is no if with handle.
    //       Print proper msg.

    // check for object type
    SDK_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() ==
               HAL_OBJ_ID_INTERFACE);
    return (if_t *)hal_handle_get_obj(handle);
#endif
}

lif_t *
find_lif_by_if_handle (hal_handle_t if_handle)
{
    if_t    *if_p;

    if_p = find_if_by_handle(if_handle);
    if (!if_p) {
        return NULL;
    }

    if (if_p->if_type == intf::IF_TYPE_ENIC) {
        return find_lif_by_handle(if_p->lif_handle);
    } else {
        return NULL;
    }
}

//------------------------------------------------------------------------------
// insert a if to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
if_add_to_db (if_t *hal_if, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("adding to hal_if id hash table");
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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add if id to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return ret;
}

//------------------------------------------------------------------------------
// delete a if from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
if_del_from_db (if_t *hal_if)
{
    hal_handle_id_ht_entry_t *entry;

    HAL_TRACE_DEBUG("removing from if id hash table");
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
if_lookup_key_or_handle (const InterfaceKeyHandle& key_handle)
{
    if (key_handle.key_or_handle_case() ==
            InterfaceKeyHandle::kInterfaceId) {
        return find_if_by_id(key_handle.interface_id());
    }
    if (key_handle.key_or_handle_case() ==
            InterfaceKeyHandle::kIfHandle) {
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
    IfType if_type;
    hal_ret_t ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("if id and handle not set in request");
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            InterfaceKeyHandle::kInterfaceId) {
        HAL_TRACE_ERR("if id not set in request");
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // interface type must be valid
    if_type = spec.type();
    if (if_type == intf::IF_TYPE_NONE) {
        HAL_TRACE_ERR("if type not set in request");
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (if_type == intf::IF_TYPE_ENIC) {
        // make sure ENIC info is provided
        if (!spec.has_if_enic_info()) {
            HAL_TRACE_ERR("no enic info. err : {} ",
                          HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

        // For Classic Enics, lif has to be present when enic gets created.
        if (spec.if_enic_info().enic_type() == intf::IF_ENIC_TYPE_CLASSIC) {
            // check if lif is provided
            if (!spec.if_enic_info().has_lif_key_or_handle()) {
                HAL_TRACE_ERR("no lif for enic. err : {} ",
                              HAL_RET_INVALID_ARG);
                rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
                return HAL_RET_INVALID_ARG;
            }
            // enic type info has to be classic
            if (spec.if_enic_info().enic_type_info_case() !=
                    IfEnicInfo::ENIC_TYPE_INFO_NOT_SET &&
                    spec.if_enic_info().enic_type_info_case() !=
                    IfEnicInfo::kClassicEnicInfo) {
                // info is set but its not valid
                HAL_TRACE_ERR(" wrong enic info being passed for "
                              "classic enic err : {}",
                              HAL_RET_INVALID_ARG);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        } else{
            // enic type info has to be non classic
            if (spec.if_enic_info().enic_type_info_case() !=
                    IfEnicInfo::ENIC_TYPE_INFO_NOT_SET &&
                    spec.if_enic_info().enic_type_info_case() !=
                    IfEnicInfo::kEnicInfo) {
                // info is set but its not valid
                HAL_TRACE_ERR("wrong enic info being passed "
                              "for non-classic enic err : {}",
                              HAL_RET_INVALID_ARG);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        }
    } else if (if_type == intf::IF_TYPE_UPLINK) {
        // uplink specific validation
        if (!spec.has_if_uplink_info()) {
            HAL_TRACE_ERR("no uplink info. err : {} ",
                          HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_UPLINK_PC) {
        // While create we dont have to get any pc info.
        // Everything can come as update
#if 0
        // uplink PC specific validation
        if (!spec.has_if_uplink_pc_info()) {
            HAL_TRACE_ERR("no uplinkpc info. err : {} ",
                          HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
#endif
    } else if (if_type == intf::IF_TYPE_TUNNEL) {
        // tunnel specification validation
        if (!spec.has_if_tunnel_info()) {
            HAL_TRACE_ERR("no tunnel info. err : {} ",
                          HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_CPU) {
        // CPU specific validation
        if (!spec.has_if_cpu_info()) {
            HAL_TRACE_ERR("no cpu if info. err : {} ",
                          HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_APP_REDIR) {
        // App Redirect specific validation
        if (!spec.has_if_app_redir_info() ||
            !spec.if_app_redir_info().has_lif_key_or_handle()) {
            HAL_TRACE_ERR("no app redir if info. err : {} ",
                          HAL_RET_INVALID_ARG);
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        HAL_TRACE_ERR("invalid type err : {} ",
                HAL_RET_INVALID_ARG);
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
    pd::pd_func_args_t          pd_func_args = {0};

    // if_create_app_ctxt_t        *app_ctxt  = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (if_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("if_id : {}:create add cb", hal_if->if_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_if_create_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;
    if (hal_if->if_type != intf::IF_TYPE_UPLINK &&
        hal_if->if_type != intf::IF_TYPE_UPLINK_PC) {
        pd_if_args.lif = find_lif_by_handle(hal_if->lif_handle);
    }
    pd_func_args.pd_if_create = &pd_if_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_CREATE,  &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create if pd, err : {}", ret);
    }

end:
    return ret;
}

hal_ret_t
enicif_classic_update_oif_lists(if_t *hal_if, l2seg_t *l2seg,
                                lif_t *lif, bool add)
{
    hal_ret_t                   ret = HAL_RET_OK;
    // oif_t                       oif = {};

    SDK_ASSERT(l2seg && hal_if && lif);

#if 0
    oif.intf = hal_if;
    oif.l2seg = l2seg;
#endif

    ret = l2seg_update_oiflist_oif(l2seg, hal_if, add, false,
                                  lif->packet_filters.receive_broadcast,
                                  lif->packet_filters.receive_all_multicast,
                                  lif->packet_filters.receive_promiscuous);

#if 0
    if (add) {
        if (lif->packet_filters.receive_broadcast) {
            ret = oif_list_add_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_add_oif(l2seg_get_shared_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
        if (lif->packet_filters.receive_all_multicast) {
            ret = oif_list_add_oif(l2seg_get_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_add_oif(l2seg_get_shared_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
        if (lif->packet_filters.receive_promiscuous) {
            ret = oif_list_add_oif(l2seg_get_prmsc_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
    } else {
        if (lif->packet_filters.receive_broadcast) {
            ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_remove_oif(l2seg_get_shared_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
        if (lif->packet_filters.receive_all_multicast) {
            ret = oif_list_remove_oif(l2seg_get_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_remove_oif(l2seg_get_shared_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
        if (lif->packet_filters.receive_promiscuous) {
            ret = oif_list_remove_oif(l2seg_get_prmsc_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
    }
#endif

    return ret;
}

hal_ret_t
enicif_classic_update_l2seg_oiflist(if_t *hal_if, l2seg_t *l2seg,
                                    lif_update_app_ctxt_t *lif_upd)
{
    hal_ret_t                   ret = HAL_RET_OK;
    oif_t                       oif = {};

    oif.intf = hal_if;
    oif.l2seg = l2seg;

    if (lif_upd->pkt_filter_bcast_changed) {
        ret = l2seg_update_oiflist_oif(l2seg, hal_if, lif_upd->receive_broadcast, false,
                                       true, false, false); 
#if 0
        if (lif_upd->receive_broadcast) {
            ret = oif_list_add_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_add_oif(l2seg_get_shared_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        } else {
            ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_remove_oif(l2seg_get_shared_bcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
#endif
    }

    if (lif_upd->pkt_filter_allmc_changed) {
        ret = l2seg_update_oiflist_oif(l2seg, hal_if, lif_upd->receive_all_multicast, 
                                       false,
                                       false, true, false); 
#if 0
        if (lif_upd->receive_all_multicast) {
            ret = oif_list_add_oif(l2seg_get_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_add_oif(l2seg_get_shared_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        } else {
            ret = oif_list_remove_oif(l2seg_get_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
            ret = oif_list_remove_oif(l2seg_get_shared_mcast_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
#endif
    }

    if (lif_upd->pkt_filter_prom_changed) {
        ret = l2seg_update_oiflist_oif(l2seg, hal_if, lif_upd->receive_promiscous, 
                                       false,
                                       false, false, true); 
#if 0
        if (lif_upd->receive_promiscous) {
            ret = oif_list_add_oif(l2seg_get_prmsc_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        } else {
            ret = oif_list_remove_oif(l2seg_get_prmsc_oif_list(l2seg), &oif);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
#endif
    }

    return ret;
}

hal_ret_t
if_update_classic_oif_lists(if_t *hal_if, lif_update_app_ctxt_t *lif_upd)
{
    hal_ret_t        ret = HAL_RET_OK;
    dllist_ctxt_t    *curr, *next;
    if_l2seg_entry_t *entry = NULL;
    l2seg_t          *l2seg = NULL, *nat_l2seg = NULL;
    dllist_ctxt_t    *l2segs_list = &hal_if->l2seg_list_clsc_head;

    // Add native L2Segment if valid
    if (hal_if->native_l2seg_clsc != HAL_HANDLE_INVALID) {
        nat_l2seg = l2seg_lookup_by_handle(hal_if->native_l2seg_clsc);
        SDK_ASSERT(nat_l2seg);
        ret = enicif_classic_update_l2seg_oiflist(hal_if, nat_l2seg,
                                                  lif_upd);
    }

    dllist_for_each_safe(curr, next, l2segs_list) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);
        ret = enicif_classic_update_l2seg_oiflist(hal_if, l2seg,
                                                  lif_upd);
    }

    return ret;
}


hal_ret_t
if_update_oif_lists (if_t *hal_if, bool add)
{
    oif_t oif = {};
    hal_ret_t  ret = HAL_RET_OK;
    l2seg_t    *l2seg = NULL, *nat_l2seg = NULL;

    SDK_ASSERT(hal_if);
    HAL_TRACE_DEBUG("if_id : {}", hal_if->if_id);

    // If its enic, add to l2seg and lif
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        lif_t *lif = find_lif_by_handle(hal_if->lif_handle);
        if (!lif) {
            HAL_TRACE_DEBUG("No lif. skipping adding to mcast oiflists");
            goto end;
        }
        if (hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            l2seg = l2seg_lookup_by_handle(hal_if->l2seg_handle);
            if (l2seg) {
                oif.intf = hal_if;
                oif.l2seg = l2seg;

                ret = l2seg_update_oiflist_oif(l2seg, hal_if, add,
                                               false, 
                                               true,    // BC 
                                               true,    // MC
                                               false);  // Prom
#if 0
                // Add the new interface to the broadcast list of the associated
                // l2seg.
                if (add) {
                    // ret = oif_list_add_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
                } else {
                    // ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
                }
#endif

                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("{} oif to oif_list failed, err : {}",
                                  add ? "Add": "Del", ret);
                    goto end;
                }
            }
        } else {
            // Add native L2Segment if valid
            if (hal_if->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                nat_l2seg = l2seg_lookup_by_handle(hal_if->native_l2seg_clsc);
                SDK_ASSERT(nat_l2seg);
                ret = enicif_classic_update_oif_lists(hal_if, nat_l2seg, lif, add);
                SDK_ASSERT(ret == HAL_RET_OK);
            }

            ret = enicif_update_l2segs_oif_lists(hal_if, lif, add);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
    }

//    if (hal_if->if_type == intf::IF_TYPE_UPLINK_PC) {
//        Nothing to be done here since OIFs will be added when
//        the l2seg is added to the uplink interface
//
//    }

end:
    return ret;
}

hal_ret_t
if_add_to_db_and_refs (if_t *hal_if)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    hal_handle = hal_if->hal_handle;
    l2seg_t         *l2seg = NULL, *nat_l2seg = NULL;
    lif_t           *lif = NULL;
    if_t            *uplink = NULL;

    // Add to if id hash table
    ret = if_add_to_db(hal_if, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add if {} to db, err : {}",
                      hal_if->if_id, ret);
        goto end;
    }

    // If its enic, add to l2seg and lif
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        lif = find_lif_by_handle(hal_if->lif_handle);

        if (hal_if->enic_type == intf::IF_ENIC_TYPE_GFT) {
            // add to lif
            ret = lif_add_if(lif, hal_if);
            HAL_ABORT(ret == HAL_RET_OK);
        } else if (hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            l2seg = l2seg_lookup_by_handle(hal_if->l2seg_handle);
            // add to l2seg
            ret = l2seg_add_back_if(l2seg, hal_if);
            HAL_ABORT(ret == HAL_RET_OK);

            // add to lif
            if (lif) {
                ret = lif_add_if(lif, hal_if);
                HAL_ABORT(ret == HAL_RET_OK);
            }
        } else {
            if (lif) {
                // add to lif
                ret = lif_add_if(lif, hal_if);
                HAL_ABORT(ret == HAL_RET_OK);
            }

            // Add to uplink's back refs
            if (hal_if->pinned_uplink != HAL_HANDLE_INVALID) {
                uplink = find_if_by_handle(hal_if->pinned_uplink);
                if (uplink == NULL) {
                    HAL_TRACE_ERR("Unable to find uplink_hdl : {}",
                                  hal_if->pinned_uplink);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
                ret = uplink_add_enicif(uplink, hal_if);
                SDK_ASSERT(ret == HAL_RET_OK);
            }
            // Add to native l2seg's back ref
            if (hal_if->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                nat_l2seg = l2seg_lookup_by_handle(hal_if->native_l2seg_clsc);
                if (nat_l2seg == NULL) {
                    HAL_TRACE_ERR("Unable to find native_l2seg_hdl : {}",
                                  hal_if->native_l2seg_clsc);
                    ret = HAL_RET_INVALID_ARG;
                    goto end;
                }
                ret = l2seg_add_back_if(nat_l2seg, hal_if);
                SDK_ASSERT(ret == HAL_RET_OK);
            }

            //  - Add back refs to all l2segs
            ret = enicif_update_l2segs_relation(&hal_if->l2seg_list_clsc_head,
                                                hal_if, true);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to add l2seg -> enicif "
                              "relation ret : {}", ret);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        }
    }

    if (hal_if->if_type == intf::IF_TYPE_UPLINK_PC) {
        // Add relation from mbr uplink if to PC
        ret = uplinkpc_update_mbrs_relation(hal_if->mbr_if_list,
                                            hal_if, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add uplinkif -> uplinkpc "
                          "relation ret : {}", ret);
            goto end;
        }

    }

    if ((hal_if->if_type == intf::IF_TYPE_UPLINK) ||
        (hal_if->if_type == intf::IF_TYPE_UPLINK_PC)) {
        g_num_uplink_ifs++;
        g_uplink_if_ids.push_back(hal_if->if_id);
    }

    if (hal_if->if_type == intf::IF_TYPE_APP_REDIR) {
        g_hal_state->set_app_redir_if_id(hal_if->if_id);
    }

    // TODO: Increment the ref counts of dependent objects

    // Finally add the new interface to all the relevant OIF lists.
    // The call is here because the multicast oif call requires the pi_if to
    // have been created completely.
    ret = if_update_oif_lists(hal_if, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to add if to OIFs");
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
    // hal_handle_t                hal_handle = 0;
    // if_create_app_ctxt_t        *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (if_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;
    // hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("if_id : {}:create commit cb", hal_if->if_id);

    ret = if_add_to_db_and_refs(hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to add to DB or refs: if:{}, err:{}",
                      hal_if->if_id, ret);
        goto end;
    }

end:
    return ret;
}

hal_ret_t
if_create_abort_cleanup (if_t *hal_if, hal_handle_t hal_handle)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_delete_args_t     pd_if_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    // delete call to PD
    if (hal_if->pd_if) {
        pd::pd_if_delete_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        pd_func_args.pd_if_delete = &pd_if_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete if pd, err : {}", ret);
        }
    }

    // members are populated before commit_cb itself. So if it fails, we have to clean
    if (hal_if->if_type == intf::IF_TYPE_UPLINK_PC) {
        hal_remove_all_handles_block_list(hal_if->mbr_if_list);
        // hal_free_handles_list(&hal_if->l2seg_list_head);
    }

    if (hal_if->if_type == intf::IF_TYPE_ENIC &&
            hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
        enicif_free_l2seg_entry_list(&hal_if->l2seg_list_clsc_head);
    }

    // remove the object
    hal_handle_free(hal_handle);

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
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if    = NULL;
    hal_handle_t                hal_handle = 0;
    dllist_ctxt_t               *lnode     = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    hal_if = (if_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("if_id : {}:create abort cb",
                    hal_if->if_id);

    if_create_abort_cleanup(hal_if, hal_handle);

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
if_prepare_rsp (InterfaceResponse *rsp, hal_ret_t ret,
                hal_handle_t hal_handle, hal_handle_t pinned_uplink_handle)
{
    if (ret == HAL_RET_OK && hal_handle != 0) {
        rsp->mutable_status()->set_if_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    if (pinned_uplink_handle != HAL_HANDLE_INVALID) {
        rsp->mutable_status()->mutable_enic_info()->set_uplink_if_handle(pinned_uplink_handle);
        HAL_TRACE_DEBUG("Enic {} Uplink interface {}", hal_handle, pinned_uplink_handle);
    }

    return HAL_RET_OK;
}

static void
if_set_rsp_status (if_t *hal_if, InterfaceResponse *rsp)
{
    rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
    rsp->mutable_status()->set_if_handle(hal_if->hal_handle);
    // Rest of the status need to be filled in from PD
}

IfStatus
uplink_if_get_oper_state(uint32_t fp_port_num)
{
    hal_ret_t   ret = HAL_RET_OK;
    port_args_t args = {0};

    args.port_num = fp_port_num;
    ret = linkmgr::port_get(&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to get port oper status: port: {}. err: {}",
                      fp_port_num, ret);
        return intf::IF_STATUS_NONE;
    }
    switch(args.oper_status) {
        case port_oper_status_t::PORT_OPER_STATUS_NONE: return intf::IF_STATUS_NONE;
        case port_oper_status_t::PORT_OPER_STATUS_UP:   return intf::IF_STATUS_UP;
        case port_oper_status_t::PORT_OPER_STATUS_DOWN: return intf::IF_STATUS_DOWN;
        default:                                        return intf::IF_STATUS_NONE;
    }
}

uint32_t
uplink_if_get_idx (if_t *hal_if)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_if_get_args_t    args   = {0};
    InterfaceGetResponse    rsp;
    pd::pd_func_args_t      pd_func_args = {0};

    // Getting PD information
    args.hal_if = hal_if;
    args.rsp = &rsp;
    pd_func_args.pd_if_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for If id : {}. ret : {}",
                      hal_if->if_id, ret);
    }
    return rsp.status().uplink_info().uplink_idx();
}

hal_ret_t
if_init_from_spec(if_t *hal_if, const InterfaceSpec& spec)
{
    hal_ret_t       ret = HAL_RET_OK;

    // consume the config
    hal_if->if_id = spec.key_or_handle().interface_id();
    hal_if->if_type = spec.type();
    hal_if->if_admin_status = spec.admin_status();
    hal_if->if_op_status = intf::IF_STATUS_NONE;      // TODO: set this later !!

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        ret = enic_if_create(spec, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_UPLINK:
        ret = uplink_if_create(spec, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        // will be added to broadcast list through add_l2seg_on_uplink() call
        break;

    case intf::IF_TYPE_UPLINK_PC:
        ret = uplink_pc_create(spec, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_l2seg_on_uplink() call
        break;

    case intf::IF_TYPE_TUNNEL:
        ret = tunnel_if_create(spec, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_l2seg_on_uplink() call
        break;

    case intf::IF_TYPE_CPU:
        ret = cpu_if_create(spec, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_APP_REDIR:
        ret = app_redir_if_create(spec, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;
    default:
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

end:
    return ret;
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
    // lif_t                       *lif = NULL;
    if_t                        *hal_if = NULL;
    if_create_app_ctxt_t        app_ctxt;
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    hal_handle_t                pinned_uplink_handle = HAL_HANDLE_INVALID;

    hal_api_trace(" API Begin: Interface create ");
    proto_msg_dump(spec);

    // do basic validations on interface
    ret = validate_interface_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("validation failed. err : {} apistatus : {}",
                      ret, rsp->api_status());
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return ret;
    }

    // check if intf exists already, and reject if one is found
    hal_if = find_if_by_id(spec.key_or_handle().interface_id());
    if (hal_if) {
        HAL_TRACE_ERR("Failed to create an if, "
                      "if {} exists already",
                      spec.key_or_handle().interface_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        if_set_rsp_status(hal_if, rsp);
        return HAL_RET_ENTRY_EXISTS;
    }

    // allocate and initialize interface instance
    hal_if = if_alloc_init();
    if (hal_if == NULL) {
        HAL_TRACE_ERR("Unable to allocate handle/memory ret : {}",
                      ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    // initialize if attributes from its spec
    ret = if_init_from_spec(hal_if, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("If Create Failed: Unable to init from spec. err:{}",
                      ret);
        goto end;
    }

    // allocate hal handle id
    hal_if->hal_handle = hal_handle_alloc(HAL_OBJ_ID_INTERFACE);
    if (hal_if->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle {}",
                      hal_if->if_id);
        if_cleanup(hal_if);
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
                HAL_TRACE_ERR("GRE tunnelif {} create Invalid AF {}",
                    spec.key_or_handle().interface_id(), hal_if->gre_dest.af);
                ret = HAL_RET_IF_INFO_INVALID;
                goto end;
        }
        if (ep == NULL) {
            HAL_TRACE_ERR("GRE tunnelif create did not find EP {}",
                spec.key_or_handle().interface_id());
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        ep->gre_if_handle = hal_if->hal_handle;
        HAL_TRACE_DEBUG("GRE tunnelif {} added to EP vrfId {}",
                spec.key_or_handle().interface_id(), hal_if->tid);
    }
    // form ctxt and call infra add
    // app_ctxt.l2seg = l2seg;
    // app_ctxt.lif = lif;
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
    if (ret != HAL_RET_OK && ret != HAL_RET_ENTRY_EXISTS) {
        if (hal_if) {
            // if there is an error, if will be freed in abort cb
            if_cleanup(hal_if);
            hal_if = NULL;
        }
    }
    if (hal_if && hal_if->if_type == intf::IF_TYPE_ENIC) {
        pinned_uplink_handle = hal_if->pinned_uplink;
    }
    if_prepare_rsp(rsp, ret, hal_if ? hal_if->hal_handle : HAL_HANDLE_INVALID, pinned_uplink_handle);
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
        HAL_TRACE_ERR("spec has no key or handle");
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
    pd::pd_func_args_t          pd_func_args = {0};

    // Just alloc, no need to init. We dont want new block lists
    *if_clone = if_alloc();

    memcpy(*if_clone, hal_if, sizeof(if_t));

    // After copy, always reset dllists as the prev and next after the copy
    // will have older prev and next.
    dllist_reset(&(*if_clone)->mc_entry_list_head);
    dllist_reset(&(*if_clone)->l2seg_list_clsc_head);

    args.hal_if = hal_if;
    args.clone = *if_clone;
    pd_func_args.pd_if_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_IF_MAKE_CLONE, &pd_func_args);

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
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    lif_handle = HAL_HANDLE_INVALID;
    hal_handle_t    spec_pinned_uplink = HAL_HANDLE_INVALID;
    l2seg_t         *native_l2seg = NULL;
    hal_handle_t    native_l2seg_handle = HAL_HANDLE_INVALID;

    auto if_enic_info = spec.if_enic_info();

    HAL_TRACE_DEBUG("update for if_id : {}",
                    spec.key_or_handle().interface_id());

    SDK_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC) {
        auto clsc_enic_info = if_enic_info.mutable_classic_enic_info();
        if (clsc_enic_info->native_l2segment_id() != 0) {
            native_l2seg = find_l2seg_by_id(clsc_enic_info->native_l2segment_id());
            if (native_l2seg != NULL) {
                native_l2seg_handle = native_l2seg->hal_handle;
            }
        }

        // check of native l2seg change
        if (hal_if->native_l2seg_clsc != native_l2seg_handle) {

            app_ctxt->new_native_l2seg_clsc = native_l2seg_handle;
                // clsc_enic_info->native_l2segment_handle();

            HAL_TRACE_DEBUG("updating native_l2seg_hdl {} => {}",
                            hal_if->native_l2seg_clsc,
                            app_ctxt->new_native_l2seg_clsc);


            if (app_ctxt->new_native_l2seg_clsc != HAL_HANDLE_INVALID) {
                if (l2seg_lookup_by_handle(app_ctxt->new_native_l2seg_clsc)
                        == NULL) {
                    HAL_TRACE_ERR("Unable to find new "
                            "l2seg_handle : {}",
                            app_ctxt->new_native_l2seg_clsc);
                    ret = HAL_RET_L2SEG_NOT_FOUND;
                    goto end;
                }
            } else {
                HAL_TRACE_DEBUG("removing native l2seg");
            }

            app_ctxt->native_l2seg_clsc_change = true;
            *has_changed = true;
        }

        // check of pinned uplink change
        if (if_enic_info.pinned_uplink_if_key_handle().key_or_handle_case() == InterfaceKeyHandle::kInterfaceId) {
            spec_pinned_uplink = find_hal_handle_from_if_id(if_enic_info.pinned_uplink_if_key_handle().interface_id());
        } else {
            spec_pinned_uplink = if_enic_info.pinned_uplink_if_key_handle().if_handle();
        }

        if (hal_if->pinned_uplink != spec_pinned_uplink) {
            app_ctxt->new_pinned_uplink = spec_pinned_uplink;
            HAL_TRACE_DEBUG("updating pinned uplink hdl {} => {}",
                            hal_if->pinned_uplink,
                            app_ctxt->new_pinned_uplink);

            if (find_if_by_handle(app_ctxt->new_pinned_uplink) == NULL) {
                HAL_TRACE_ERR("Unable to find new uplinkif_hdl : {}",
                              app_ctxt->new_pinned_uplink);
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
            HAL_TRACE_ERR("Failed to check classic l2seg "
                          "list change. ret : {}",
                          ret);
            goto end;
        }
        if (app_ctxt->l2segclsclist_change) {
            *has_changed = true;
        }
    } else {
        if (hal_if->lif_learned) {
            HAL_TRACE_DEBUG("Lif already learnt. Skipping lif change check");
            goto end;
        }
        ret = get_lif_handle_from_spec(spec, &lif_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to find lif. ret: {}", ret);
            goto end;
        }
        if (hal_if->lif_handle != lif_handle) {
            app_ctxt->lif_change = true;
            app_ctxt->lif = find_lif_by_handle(lif_handle);
            *has_changed = true;
            HAL_TRACE_DEBUG("updating lif hdl from {} -> {}",
                            hal_if->lif_handle, lif_handle);
        }

        if (hal_if->encap_vlan != if_enic_info.mutable_enic_info()->encap_vlan_id()) {
            app_ctxt->new_encap_vlan = if_enic_info.mutable_enic_info()->encap_vlan_id();
            app_ctxt->encap_vlan_change = true;
            *has_changed = true;
            HAL_TRACE_DEBUG("updating encap vlan {} => {}",
                            hal_if->encap_vlan, app_ctxt->new_encap_vlan);
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

    HAL_TRACE_DEBUG("update for if_id : {}",
                    spec.key_or_handle().interface_id());

    SDK_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->native_l2seg != spec.if_uplink_info().native_l2segment_id()) {
        new_seg_id = spec.if_uplink_info().native_l2segment_id();
        HAL_TRACE_DEBUG("updating native_l2seg_id {} => {}",
                        hal_if->native_l2seg, new_seg_id);


        if (new_seg_id != HAL_L2SEGMENT_ID_INVALID) {
            app_ctxt->native_l2seg = find_l2seg_by_id(new_seg_id);
            if (!app_ctxt->native_l2seg) {
                HAL_TRACE_ERR("Unable to find new l2seg : {}",
                              new_seg_id);
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
        }

        app_ctxt->native_l2seg_change = true;
        *has_changed = true;
    }

    if (hal_if->is_oob_management != spec.if_uplink_info().is_oob_management()) {
        app_ctxt->is_oob = spec.if_uplink_info().is_oob_management();
        app_ctxt->is_oob_change = true;
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

    HAL_TRACE_DEBUG("update for if_id : {}",
                    spec.key_or_handle().interface_id());

    SDK_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    // check of native l2seg change
    if (hal_if->native_l2seg != spec.if_uplink_pc_info().native_l2segment_id()) {
        new_seg_id = spec.if_uplink_pc_info().native_l2segment_id();
        HAL_TRACE_DEBUG("updating native_l2seg_id {} => {}",
                        hal_if->native_l2seg, new_seg_id);

        if (new_seg_id != HAL_L2SEGMENT_ID_INVALID) {
            app_ctxt->native_l2seg = find_l2seg_by_id(new_seg_id);
            if (!app_ctxt->native_l2seg) {
                HAL_TRACE_ERR("Unable to find new l2seg : {}",
                              new_seg_id);
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
        HAL_TRACE_ERR("Failed to check mbr list change. ret : {}",
                      ret);
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
        SDK_ASSERT_RETURN(l2seg != NULL, HAL_RET_INVALID_ARG);
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
        HAL_TRACE_ERR("Cannot change if type from {} to {} as part of "
                      "if update", hal_if->if_type, spec.type());
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
        HAL_TRACE_ERR("invalid if type : {}",
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
    pd::pd_ep_if_update_args_t  pd_ep_if_args = {0};
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    if_t                        *hal_if    = NULL, *hal_if_clone = NULL;
    if_update_app_ctxt_t        *app_ctxt  = NULL;
    pd::pd_func_args_t          pd_func_args = {0};
    l2seg_t                     *native_l2seg_old;
    oif_t                       oif = {};
    hal_handle_t                *p_hdl = NULL;
    ep_t                        *ep = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    hal_if = (if_t *)dhl_entry->obj;
    hal_if_clone = (if_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb {}", hal_if->if_id);

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        pd::pd_if_update_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        pd_if_args.intf_clone = hal_if_clone;
        pd_if_args.native_l2seg_clsc_change = app_ctxt->native_l2seg_clsc_change;
        pd_if_args.new_native_l2seg_clsc = app_ctxt->new_native_l2seg_clsc;
        pd_if_args.pinned_uplink_change = app_ctxt->pinned_uplink_change;
        pd_if_args.new_pinned_uplink = app_ctxt->new_pinned_uplink;
        pd_if_args.l2seg_clsc_change = app_ctxt->l2segclsclist_change;
        pd_if_args.add_l2seg_clsclist = app_ctxt->add_l2segclsclist;
        pd_if_args.del_l2seg_clsclist = app_ctxt->del_l2segclsclist;
        pd_if_args.lif_change = app_ctxt->lif_change;
        pd_if_args.new_lif = app_ctxt->lif;
        pd_if_args.encap_vlan_change = app_ctxt->encap_vlan_change;
        pd_if_args.new_encap_vlan = app_ctxt->new_encap_vlan;
        if (app_ctxt->encap_vlan_change) {
            hal_if_clone->encap_vlan = app_ctxt->new_encap_vlan;
        }
        break;
    case intf::IF_TYPE_UPLINK:
    case intf::IF_TYPE_UPLINK_PC:
        pd::pd_if_update_args_init(&pd_if_args);
        pd_if_args.intf = hal_if;
        pd_if_args.intf_clone = hal_if_clone;
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
        HAL_TRACE_ERR("invalid if type : {}", hal_if->if_type);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    pd_func_args.pd_if_update = &pd_if_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update if pd, err  : {}", ret);
    }

    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        // If l2seg list changes for classic enic
        if (pd_if_args.l2seg_clsc_change) {
            ret = enicif_clsc_l2seglist_change_update_oiflists(hal_if,
                                                               pd_if_args.add_l2seg_clsclist,
                                                               true);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Not able to add new oifs to oiflists. ret: {}",
                              ret);
                return ret;
            }
            ret = enicif_clsc_l2seglist_change_update_oiflists(hal_if,
                                                               pd_if_args.del_l2seg_clsclist,
                                                               false);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Not able to del old oifs from oiflists. ret: {}",
                              ret);
                return ret;
            }
        }
        // If lif changes on enic, we should trigger EP to reprogram ipsg entries & reg_mac entries.
        if (pd_if_args.lif_change && hal_if->if_type == intf::IF_TYPE_ENIC) {
            pd_ep_if_args.lif_change = pd_if_args.lif_change;
            pd_ep_if_args.new_lif = pd_if_args.new_lif;
            for (const void *ptr : *(hal_if->ep_list)) {
                p_hdl = (hal_handle_t *)ptr;
                ep = find_ep_by_handle(*p_hdl);
                if (!ep) {
                    HAL_TRACE_ERR("Unable to find ep with handle  : {}",
                                  *p_hdl);
                    continue;
                }
                HAL_TRACE_DEBUG("Updating EP {} b'coz of lif_change",
                                ep_l2_key_to_str(ep));
                pd_ep_if_args.ep = ep;
                pd_func_args.pd_ep_if_update = &pd_ep_if_args;
                ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_IF_UPDATE, &pd_func_args);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Failed to update EP with if update, err  : {}",
                                  ret);
                }
            }

            // Update mcast oiflist
            if (hal_if->lif_handle == HAL_HANDLE_INVALID) {
                if (pd_if_args.new_lif) {
                    // Add oif
                    ret = if_update_oif_lists(hal_if_clone, true);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Unable to add Enic to oiflist. ret: {}",
                                      ret);
                        goto end;
                    }
                }
            } else {
                if (!pd_if_args.new_lif) {
                    // Delete oif
                    ret = if_update_oif_lists(hal_if, false);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Unable to del Enic from oiflist. ret: {}",
                                      ret);
                        goto end;
                    }
                } else {
                    // Delete
                    ret = if_update_oif_lists(hal_if, false);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Unable to del Enic from oiflist. ret: {}",
                                      ret);
                        goto end;
                    }

                    // Add
                    ret = if_update_oif_lists(hal_if_clone, true);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Unable to add Enic to oiflist. ret: {}",
                                      ret);
                        goto end;
                    }
                }
            }
        }
    }

    // Deprecated. Not support smart switch mode. 
#if 0
    // Change Flood replication entry for change of native l2seg
    if (is_forwarding_mode_smart_switch() && pd_if_args.native_l2seg_change &&
        hal::g_hal_cfg.features != hal::HAL_FEATURE_SET_GFT) {
        // Remove and add replication entry
        if (hal_if->native_l2seg != 0) {
            native_l2seg_old = find_l2seg_by_id(hal_if->native_l2seg);

            oif.intf = hal_if;
            oif.l2seg = native_l2seg_old;

            // remove replication entry for old native l2seg
            ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(native_l2seg_old), &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Native l2seg change: Remove repl entry of old l2seg: {}",
                              l2seg_keyhandle_to_str(native_l2seg_old));
                goto end;
            }

            // add replication entry for old native l2seg
            ret = oif_list_add_oif(l2seg_get_bcast_oif_list(native_l2seg_old), &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Native l2seg change: Add repl entry of old l2seg: {}",
                              l2seg_keyhandle_to_str(native_l2seg_old));
                goto end;
            }
        }

        if (pd_if_args.native_l2seg) {

            // TODO: Have to check if new native l2seg is already part of bcast oiflist.
            //       If not skip this.

            oif.intf = hal_if;
            oif.l2seg = pd_if_args.native_l2seg;

            // remove replication entry for new native l2seg
            ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(pd_if_args.native_l2seg), &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Native l2seg change: Remove repl entry of new l2seg: {}",
                              l2seg_keyhandle_to_str(pd_if_args.native_l2seg));
                goto end;
            }

            hal_if_clone->native_l2seg = pd_if_args.native_l2seg->seg_id;
            oif.intf = hal_if_clone;
            oif.l2seg = pd_if_args.native_l2seg;
            // add replication entry for old native l2seg
            ret = oif_list_add_oif(l2seg_get_bcast_oif_list(pd_if_args.native_l2seg), &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Native l2seg change: Add repl entry of new l2seg: {}",
                              l2seg_keyhandle_to_str(pd_if_args.native_l2seg));
                goto end;
            }
        }
    }
#endif

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
            HAL_TRACE_ERR("unable to find l2seg with handle:{}",
                          entry->l2seg_handle);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        // Remove entry from temp. list
        sdk::lib::dllist_del(&entry->lentry);

        // Add entry in the main list
        sdk::lib::dllist_add(&hal_if->l2seg_list_clsc_head,
                          &entry->lentry);

        // Add the back reference in l2seg
        ret = l2seg_add_back_if(l2seg, hal_if);
        SDK_ASSERT(ret == HAL_RET_OK);
    }

    dllist_for_each_safe(curr, next, app_ctxt->del_l2segclsclist) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);
        SDK_ASSERT(l2seg != NULL);

        // Remove entry from temp. list
        sdk::lib::dllist_del(&entry->lentry);
        if (l2seg_in_classic_enicif(hal_if, entry->l2seg_handle,
                                    &del_l2seg_entry)) {
            // Remove entry from main list
            sdk::lib::dllist_del(&del_l2seg_entry->lentry);

            // Del the back reference from l2seg
            ret = l2seg_del_back_if(l2seg, hal_if);
            SDK_ASSERT(ret == HAL_RET_OK);

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
#if 0
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
    hal_handle_t                    *p_hdl_id = NULL;

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);

    for (const void *ptr : *app_ctxt->add_l2segclsclist) {
        p_hdl_id = (hal_handle_t *)ptr;
        l2seg = l2seg_lookup_by_handle(*p_hdl_id);
        if (!l2seg) {
            HAL_TRACE_ERR("Unable to find l2seg with handle : {}", *p_hdl_id);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        // Remove entry from temp. list
        hal_del_from_handle_block_list(app_ctxt->add_l2segclsclist, *p_hdl_id);

        // Add entry in the main list
        hal_add_to_handle_block_list(app_ctxt->add_l2segclsclist, *p_hdl_id);

        // Add the back reference in l2seg
        ret = l2seg_add_if(l2seg, hal_if);
        SDK_ASSERT(ret == HAL_RET_OK);
    }

    for (const void *ptr : *app_ctxt->del_l2segclsclist) {
        p_hdl_id = (hal_handle_t *)ptr;
        l2seg = l2seg_lookup_by_handle(*p_hdl_id);
        SDK_ASSERT(l2seg != NULL);

        // Remove entry from temp. list
        hal_del_from_handle_block_list(app_ctxt->del_l2segclsclist, *p_hdl_id);

        if (l2seg_in_classic_enicif(hal_if, entry->l2seg_handle)) {
            // Remove entry from main list
            hal_del_from_handle_block_list(hal_if->l2seg_list_clsc, *p_hdl_id);

            // Del the back reference from l2seg
            ret = l2seg_del_if(l2seg, hal_if);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
    }

end:

    // Free add & del list
    // enicif_cleanup_l2seg_entry_list(&app_ctxt->add_l2segclsclist);
    // enicif_cleanup_l2seg_entry_list(&app_ctxt->del_l2segclsclist);

    hal_cleanup_handle_block_list(&app_ctxt->add_l2segclsclist);
    hal_cleanup_handle_block_list(&app_ctxt->del_l2segclsclist);

    // Unlock if
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);

    return ret;
}
#endif

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

    // Destroy clone's list which is original's list as well
    hal_cleanup_handle_block_list(&hal_if->mbr_if_list);
    // Assign aggr list to clone's list
    hal_if->mbr_if_list = app_ctxt->aggr_mbrlist;

    // add/del relations from member ports.
    ret = uplinkpc_update_mbrs_relation(app_ctxt->add_mbrlist,
                                        hal_if, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add uplinkif -> uplinkpc "
                "relation ret : {}", ret);
        goto end;
    }

    ret = uplinkpc_update_mbrs_relation(app_ctxt->del_mbrlist,
                                        hal_if, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del uplinkif -/-> uplinkpc "
                "relation ret : {}", ret);
        goto end;
    }

end:
    // Free add & del list
    hal_cleanup_handle_block_list(&app_ctxt->add_mbrlist);
    hal_cleanup_handle_block_list(&app_ctxt->del_mbrlist);

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
    pd::pd_func_args_t          pd_func_args = {0};
    lif_t                       *old_lif       = NULL;


    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    intf = (if_t *)dhl_entry->obj;
    intf_clone = (if_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}",
                    intf->if_id);
    //printf("Original: %p, Clone: %p\n", intf, intf_clone);

    dllist_move(&intf_clone->mc_entry_list_head, &intf->mc_entry_list_head);

    switch (intf->if_type) {
        case intf::IF_TYPE_ENIC:
            // move lists
            dllist_move(&intf_clone->l2seg_list_clsc_head,
                        &intf->l2seg_list_clsc_head);

            // Update clone with attrs
            if (app_ctxt->native_l2seg_clsc_change) {
                HAL_TRACE_DEBUG("Setting the classic enicif clone to new "
                                "native l2seg_hdl : {}",
                                app_ctxt->new_native_l2seg_clsc);
                intf_clone->native_l2seg_clsc = app_ctxt->new_native_l2seg_clsc;
            }

            if (app_ctxt->pinned_uplink_change) {
                HAL_TRACE_DEBUG("Setting the classic enicif clone to new "
                                "pinned uplink_hdl : {}",
                                app_ctxt->new_pinned_uplink);
                intf_clone->pinned_uplink = app_ctxt->new_pinned_uplink;
                // Update uplink's relation
                old_uplink = find_if_by_handle(intf->pinned_uplink);
                new_uplink = find_if_by_handle(app_ctxt->new_pinned_uplink);
                SDK_ASSERT(old_uplink != NULL && new_uplink != NULL);

                // Remove from older uplink
                ret = uplink_del_enicif(old_uplink, intf);
                SDK_ASSERT(ret == HAL_RET_OK);
                // Add to new uplink
                ret = uplink_add_enicif(new_uplink, intf);
                SDK_ASSERT(ret == HAL_RET_OK);
            }

            if (app_ctxt->native_l2seg_clsc_change) {
                HAL_TRACE_DEBUG("Setting the classic enicif clone to new "
                                "l2seg : {}",
                                app_ctxt->new_native_l2seg_clsc);
                intf_clone->native_l2seg_clsc = app_ctxt->new_native_l2seg_clsc;
                // Update native l2seg's relation
                if (intf->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                    old_nat_l2seg = l2seg_lookup_by_handle(intf->native_l2seg_clsc);
                    SDK_ASSERT(old_nat_l2seg != NULL);
                    // Remove from older nat l2seg
                    ret = l2seg_del_back_if(old_nat_l2seg, intf);
                    SDK_ASSERT(ret == HAL_RET_OK);
                }
                // Add to new nat l2seg
                if (app_ctxt->new_native_l2seg_clsc != HAL_HANDLE_INVALID) {
                    new_nat_l2seg = l2seg_lookup_by_handle(app_ctxt->new_native_l2seg_clsc);
                    SDK_ASSERT(new_nat_l2seg != NULL);
                    ret = l2seg_add_back_if(new_nat_l2seg, intf);
                    SDK_ASSERT(ret == HAL_RET_OK);
                }
            }

            if (app_ctxt->l2segclsclist_change) {
                ret = enicif_update_pi_with_l2seg_list(intf_clone, app_ctxt);
            }

            if (app_ctxt->lif_change) {
                intf_clone->lif_handle = app_ctxt->lif ? app_ctxt->lif->hal_handle :
                    HAL_HANDLE_INVALID;

                // Remove from older lif
                old_lif = find_lif_by_handle(intf->lif_handle);

                if (old_lif) {
                    ret = lif_del_if(old_lif, intf);
                    HAL_ABORT(ret == HAL_RET_OK);
                }

                // Add to new lif
                if (app_ctxt->lif) {
                    ret = lif_add_if(app_ctxt->lif, intf_clone);
                    HAL_ABORT(ret == HAL_RET_OK);
                }
            }
            break;

        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            // move lists
            // hal_copy_block_lists(intf_clone->l2seg_list, intf->l2seg_list);
            // hal_copy_block_lists(intf_clone->mbr_if_list, intf->mbr_if_list);

            // update clone with new attrs
            if (app_ctxt->native_l2seg_change) {

                if (app_ctxt->native_l2seg) {
                    seg_id = app_ctxt->native_l2seg->seg_id;
                }
                HAL_TRACE_DEBUG("Setting the clone to new native l2seg : {}",
                                seg_id);
                intf_clone->native_l2seg = seg_id;
            }

            if (intf->if_type == intf::IF_TYPE_UPLINK_PC) {
                // update mbr list, valid only for uplink pc
                ret = if_update_pi_with_mbr_list(intf_clone, app_ctxt);
            }
#if 0
            if (app_ctxt->mbrlist_change) {
                ret = if_update_pi_with_mbr_list(intf_clone, app_ctxt);
            }
#endif

            if (app_ctxt->is_oob_change) {
                intf_clone->is_oob_management = app_ctxt->is_oob;
            }

            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        case intf::IF_TYPE_CPU:
            break;
        case intf::IF_TYPE_APP_REDIR:
            break;
        default:
            HAL_TRACE_ERR("invalid if type : {}",
                          intf->if_type);
            ret = HAL_RET_INVALID_ARG;
    }

    // Free PD
    pd::pd_if_mem_free_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    pd_func_args.pd_if_mem_free = &pd_if_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_MEM_FREE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to free original if pd, err : {}", ret);
    }

    // Free PI
    if_free(intf);

end:

    return ret;
}

// TODO: Deprecated
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
    pd::pd_func_args_t          pd_func_args = {0};

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (if_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    intf = (if_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort cb {}",
                    intf->if_id);

    // Free PD
    pd::pd_if_mem_free_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    pd_func_args.pd_if_mem_free = &pd_if_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_MEM_FREE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete if pd, err : {}",
                      ret);
    }

    if (intf->if_type == intf::IF_TYPE_UPLINK_PC) {
        // Free mbr lists
        // interface_cleanup_handle_list(&app_ctxt->add_mbrlist);
        // interface_cleanup_handle_list(&app_ctxt->del_mbrlist);
        // interface_cleanup_handle_list(&app_ctxt->aggr_mbrlist);

        hal_cleanup_handle_block_list(&app_ctxt->add_mbrlist);
        hal_cleanup_handle_block_list(&app_ctxt->del_mbrlist);
        hal_cleanup_handle_block_list(&app_ctxt->aggr_mbrlist);
    }

    if (intf->if_type == intf::IF_TYPE_ENIC) {
        // Free l2segs lists for classic enic if
        enicif_cleanup_l2seg_entry_list(&app_ctxt->add_l2segclsclist);
        enicif_cleanup_l2seg_entry_list(&app_ctxt->del_l2segclsclist);

    }

    if_free(intf);

end:

    return ret;
}

hal_ret_t
if_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
enic_update_lif (if_t *hal_if,
                 lif_t *new_lif,
                 if_t **new_hal_if)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_update_app_ctxt_t    app_ctxt = {0};
    cfg_op_ctxt_t           cfg_ctxt = { 0 };
    dhl_entry_t             dhl_entry = { 0 };


    hal_api_trace(" API Begin: Interface update ");

    if (!hal_if || !new_hal_if) {
        HAL_TRACE_ERR("Invalid args. ");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    HAL_TRACE_DEBUG("Enic update of lif. if_id: {}, new_lif_id: {}",
                    hal_if->if_id, new_lif ? new_lif->lif_id : 0);

    if (hal_if->if_type != intf::IF_TYPE_ENIC) {
        HAL_TRACE_ERR("Can't update lif on non-enic if. type: {}",
                      IfType_Name(hal_if->if_type));
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Check if lif has chnanged.
    if ((!new_lif && hal_if->lif_handle == HAL_HANDLE_INVALID) ||
        (new_lif && hal_if->lif_handle == new_lif->hal_handle)) {
        HAL_TRACE_DEBUG("No change in lif. noop. handle: {}",
                        hal_if->lif_handle);
        goto end;
    }

    HAL_TRACE_DEBUG("Updating lif handle: {} -> {}", hal_if->lif_handle,
                    new_lif ? new_lif->hal_handle : HAL_HANDLE_INVALID);

    app_ctxt.lif_change = true;
    app_ctxt.lif = new_lif;

    if_make_clone(hal_if, (if_t **)&dhl_entry.cloned_obj);
    *new_hal_if = (if_t *)dhl_entry.cloned_obj;

    if (new_lif) {
        (*new_hal_if)->lif_handle = new_lif->hal_handle;
        (*new_hal_if)->lif_learned = true;
    } else {
        (*new_hal_if)->lif_learned = false;
    }

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

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update Enic. err: {}", ret);
    }

end:
    return ret;
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

    hal_api_trace(" API Begin: Interface update ");
    proto_msg_dump(spec);

    // validate the request message
    ret = validate_if_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("if update validation failed, ret : {}", ret);
        goto end;
    }

    hal_if = if_lookup_key_or_handle(kh);
    if (!hal_if) {
        HAL_TRACE_ERR("Failed to find if, id {}, handle {}",
                      kh.interface_id(), kh.if_handle());
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }

    // check if this is a CPU type interface
    if (hal_if->if_type == intf::IF_TYPE_CPU) {
        // updates not allowed on CPU interface as this is internal
        ret = HAL_RET_INVALID_OP;
        goto end;
    }

    HAL_TRACE_DEBUG("if update for id {} type : {} enictype : {}",
                    hal_if->if_id, IfType_Name(hal_if->if_type),
                    (hal_if->if_type == intf::IF_TYPE_ENIC) ?
                    IfEnicType_Name(hal_if->enic_type) : "IF_ENIC_TYPE_NONE");

    // Check for changes
    ret = if_update_check_for_change(spec, hal_if, &app_ctxt, &has_changed);
    if (ret != HAL_RET_OK || !has_changed) {
        HAL_TRACE_WARN("no change/error in if update");
        goto end;
    }

    if_make_clone(hal_if, (if_t **)&dhl_entry.cloned_obj);
    if (app_ctxt.lif) {
        ((if_t *)(dhl_entry.cloned_obj))->lif_handle = app_ctxt.lif->hal_handle;
    }

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
    return ret;

}

static void
if_process_get (if_t *hal_if, InterfaceGetResponse *rsp)
{
    hal_ret_t               ret    = HAL_RET_OK;
    InterfaceSpec           *spec  = NULL;
    l2seg_t                 *l2seg = NULL;
    if_t                    *up_if = NULL;
    pd::pd_if_get_args_t    args   = {0};
    lif_t                   *lif;
    hal_handle_t            *p_hdl_id = NULL;
    pd::pd_func_args_t      pd_func_args = {0};

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
        lif = find_lif_by_handle(hal_if->lif_handle);
        if (lif) {
            enic_if_info->mutable_lif_key_or_handle()->set_lif_id(lif->lif_id);
        }
        if (hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            enic_if_info->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(l2seg->seg_id);
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
        uplink_if_info->set_port_num(hal_if->fp_port_num);
        uplink_if_info->set_native_l2segment_id(hal_if->native_l2seg);
        rsp->mutable_status()->mutable_uplink_info()->
            set_num_l2segs(hal_if->l2seg_list->num_elems());
        rsp->mutable_status()->mutable_uplink_info()->
            set_hw_port_num(hal_if->uplink_port_num);
        rsp->mutable_status()->set_if_status(hal_if->if_op_status);
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
        for (const void *ptr : *hal_if->mbr_if_list) {
            p_hdl_id = (hal_handle_t *)ptr;
            up_if = find_if_by_handle(*p_hdl_id);
            if (!up_if) {
                HAL_TRACE_ERR("Unable to find uplinkif with handle  : {}",
                              *p_hdl_id);
                continue;
            }
            auto mif_key_handle = uplink_pc_info->add_member_if_key_handle();
            mif_key_handle->set_interface_id(up_if->if_id);
        }
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
        } else if (hal_if->encap_type == intf::IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS) {
            auto mpls_info = tunnel_if_info->mutable_prop_mpls_info();

            auto substrate_ip = mpls_info->mutable_substrate_ip();
            substrate_ip->set_ip_af(types::IP_AF_INET);
            substrate_ip->set_v4_addr(hal_if->substrate_ip);

            for (int i = 0; i < hal_if->num_overlay_ip; i ++) {
                auto overlay_ip = mpls_info->add_overlay_ip();
                overlay_ip->set_ip_af(types::IP_AF_INET);
                overlay_ip->set_v4_addr(hal_if->overlay_ip[i]);
            }

            auto tunnel_dest_ip = mpls_info->mutable_tunnel_dest_ip();
            tunnel_dest_ip->set_ip_af(types::IP_AF_INET);
            tunnel_dest_ip->set_v4_addr(hal_if->tun_dst_ip);

            auto source_gw_prefix = mpls_info->mutable_source_gw()->mutable_prefix()->mutable_ipv4_subnet();
            auto source_gw_address = source_gw_prefix->mutable_address();
            source_gw_address->set_ip_af(types::IP_AF_INET);
            source_gw_address->set_v4_addr(hal_if->source_gw.v4_addr);
            source_gw_prefix->set_prefix_len(hal_if->source_gw.len);

            mpls_info->set_gw_mac_da(MAC_TO_UINT64(hal_if->gw_mac_da));
            mpls_info->set_overlay_mac(MAC_TO_UINT64(hal_if->overlay_mac));
            mpls_info->set_pf_mac(MAC_TO_UINT64(hal_if->pf_mac));

            for (int i = 0; i < hal_if->num_mpls_if; i ++) {
                auto mpls_if = mpls_info->add_mpls_if();
                mpls_if->set_label(hal_if->mpls_if[i].label);
                mpls_if->set_exp(hal_if->mpls_if[i].exp);
                mpls_if->set_ttl(hal_if->mpls_if[i].ttl);
            }

            auto mpls_tag = mpls_info->mutable_mpls_tag();
            mpls_tag->set_label(hal_if->mpls_tag.label);
            mpls_tag->set_exp(hal_if->mpls_tag.exp);
            mpls_tag->set_ttl(hal_if->mpls_tag.ttl);

            mpls_info->set_ingress_bw(hal_if->ingress_bw);
            mpls_info->set_egress_bw(hal_if->egress_bw);

            auto lif = find_lif_by_id(hal_if->lif_id);
            mpls_info->set_lif_name(lif->name);
        }
    }
        break;

    case intf::IF_TYPE_CPU:
    {
        spec->mutable_if_cpu_info()->mutable_lif_key_or_handle()->set_lif_handle(hal_if->lif_handle);
    }
        break;

    case intf::IF_TYPE_APP_REDIR:
        break;

    default:
        break;
    }

    // Getting PD information
    args.hal_if = hal_if;
    args.rsp = rsp;
    pd_func_args.pd_if_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for If id : {}. ret : {}",
                      hal_if->if_id, ret);
    }

}

static bool
if_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    InterfaceGetResponseMsg *rsp    = (InterfaceGetResponseMsg *)ctxt;
    InterfaceGetResponse *response  = rsp->add_response();
    if_t          *hal_if           = NULL;

    hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);
    if_process_get(hal_if, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

//------------------------------------------------------------------------------
// process a interface get request
//------------------------------------------------------------------------------
hal_ret_t
interface_get (InterfaceGetRequest& req, InterfaceGetResponseMsg *rsp)
{
    if_t             *hal_if;

    if (!req.has_key_or_handle()) {
        // When the key-handle is not set, this is a request for all interface
        // objects. Run through the hash table and retrieve all values.
        g_hal_state->if_id_ht()->walk(if_get_ht_cb, rsp);
    } else {
        hal_if = if_lookup_key_or_handle(req.key_or_handle());

        auto response = rsp->add_response();
        if (!hal_if) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            return HAL_RET_INVALID_ARG;
        } else {
            if_process_get(hal_if, response);
        }
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
            InterfaceKeyHandle::kInterfaceId) {
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
            L2SegmentKeyHandle::kSegmentId) {
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
        HAL_TRACE_ERR("no l2segment key or handle");
        return HAL_RET_L2SEG_ID_INVALID;
    }

    // L2 segment has to exist
    if (spec.l2segment_key_or_handle().key_or_handle_case() ==
            L2SegmentKeyHandle::kSegmentId &&
            !find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id())) {
        HAL_TRACE_ERR("Failed to find l2seg with id : {}",
                      spec.l2segment_key_or_handle().segment_id());
        return HAL_RET_L2SEG_NOT_FOUND;
    }
    if (spec.l2segment_key_or_handle().key_or_handle_case() ==
            L2SegmentKeyHandle::kL2SegmentHandle &&
            !l2seg_lookup_by_handle(spec.l2segment_key_or_handle().l2segment_handle())) {
        HAL_TRACE_ERR("Failed to find l2seg with handle : {}",
                      spec.l2segment_key_or_handle().l2segment_handle());
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    // uplink key/handle is must
    if (!spec.has_if_key_handle()) {
        HAL_TRACE_ERR("no if key or handle");
        return HAL_RET_INTERFACE_ID_INVALID;
    }

    // uplink has to exist
    if (spec.if_key_handle().key_or_handle_case() ==
            InterfaceKeyHandle::kInterfaceId &&
            !find_if_by_id(spec.if_key_handle().interface_id())) {
        HAL_TRACE_ERR("Failed to find interface with id : {}",
                      spec.if_key_handle().interface_id());
        return HAL_RET_IF_NOT_FOUND;
    }
    if (spec.if_key_handle().key_or_handle_case() ==
            InterfaceKeyHandle::kIfHandle &&
            !find_if_by_handle(spec.if_key_handle().if_handle())) {
        HAL_TRACE_ERR("Failed to find interface with handle : {}",
                      spec.if_key_handle().if_handle());
        return HAL_RET_IF_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("validation passed");
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
    oif_t                           oif = {};
    pd::pd_add_l2seg_uplink_args_t  pd_l2seg_uplink_args;
    pd::pd_func_args_t              pd_func_args = {0};


    // Validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    SDK_ASSERT((hal_if != NULL) && (l2seg != NULL));

    HAL_TRACE_DEBUG("if_id : {} <=> l2seg_id : {}",
                    hal_if->if_id, l2seg->seg_id);

    // PD Call
    pd::pd_add_l2seg_uplink_args_init(&pd_l2seg_uplink_args);
    pd_l2seg_uplink_args.l2seg  = l2seg;
    pd_l2seg_uplink_args.intf   = hal_if;
    pd_func_args.pd_add_l2seg_uplink = &pd_l2seg_uplink_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ADD_L2SEG_UPLINK, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed in pd, ret : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // Add Uplink in l2seg
    ret = l2seg_add_back_if(l2seg, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add if to "
                      "l2seg's if list. ret : {}",
                      ret);
        goto end;
    }

    // Add l2seg in uplink
    ret = if_add_l2seg(hal_if, l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add l2seg to "
                      "if's seg list. ret : {}",
                      ret);
        goto end;
    }

    // Deprecated: smart switch is no longer supported
#if 0
    // Add the uplink to the broadcast list of the l2seg
    if (is_forwarding_mode_smart_switch()) {
        oif.intf = hal_if;
        oif.l2seg = l2seg;

        ret = oif_list_add_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("bcast oiflist Failed. ret : {}",
                          ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
    }
#endif

end:

    rsp->set_api_status(hal_prepare_rsp(ret));
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
    oif_t                           oif = {};
    pd::pd_del_l2seg_uplink_args_t  pd_l2seg_uplink_args;
    pd::pd_func_args_t              pd_func_args = {0};

    // validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("validation Failed");
        return ret;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    SDK_ASSERT((hal_if != NULL) && (l2seg != NULL));

    HAL_TRACE_DEBUG("if_id : {} <=> l2seg_id : {}",
                    hal_if->if_id, l2seg->seg_id);

    // PD Call
    pd::pd_del_l2seg_uplink_args_init(&pd_l2seg_uplink_args);
    pd_l2seg_uplink_args.l2seg  = l2seg;
    pd_l2seg_uplink_args.intf   = hal_if;
    pd_func_args.pd_del_l2seg_uplink = &pd_l2seg_uplink_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DEL_L2SEG_UPLINK, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed in pd, ret {}:", ret);
        goto end;
    }

    // Deprecated: smart switch is no longer supported
#if 0
    // Del the uplink from the broadcast list of the l2seg
    if (is_forwarding_mode_smart_switch()) {
        oif.intf = hal_if;
        oif.l2seg = l2seg;

        ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("dell2seguplink<->link:bcast oiflist"
                                  " Failed. ret : {}", ret);
            goto end;
        }
    }
#endif

    // Del Uplink in l2seg
    ret = l2seg_del_back_if(l2seg, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del if to "
                      "l2seg's if list. ret : {}",
                      ret);
        goto end;
    }

    // Del l2seg in uplink
    ret = if_del_l2seg(hal_if, l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del l2seg to if's seg list. ret : {}", ret);
        goto end;
    }

end:

    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

//------------------------------------------------------------------------------
// CPU If Create
//------------------------------------------------------------------------------
hal_ret_t
cpu_if_create (const InterfaceSpec& spec, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif;

    HAL_TRACE_DEBUG("CPUif Create for id {}",
                    spec.key_or_handle().interface_id());

    ret = get_lif_handle_for_cpu_if(spec, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to find the lif handle, err : {}", ret);
        return ret;
    }

    lif = find_lif_by_handle(hal_if->lif_handle);
    HAL_TRACE_DEBUG("if_id : {} lif_id : {}", hal_if->if_id, lif->lif_id);

    return ret;
}


//------------------------------------------------------------------------------
// App Redir If Create
//------------------------------------------------------------------------------
hal_ret_t
app_redir_if_create (const InterfaceSpec& spec, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif;

    HAL_TRACE_DEBUG("Create for id {}",
                    spec.key_or_handle().interface_id());

    ret = get_lif_handle_for_app_redir_if(spec, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to find the lif handle Err : {}",
                      ret);
        return ret;
    }

    lif = find_lif_by_handle(hal_if->lif_handle);
    HAL_TRACE_DEBUG("if_id : {} lif_id : {}", hal_if->if_id, lif->lif_id);

    return ret;
}

//------------------------------------------------------------------------------
// Updates egress_en of enic
//------------------------------------------------------------------------------
hal_ret_t
enicif_update_egress_en(if_t *hal_if, bool egress_en)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_if_update_args_t     pd_if_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};
    bool                        curr_egress_en = hal_if->egress_en;

    // Valid only for enic and in smart nic mode.
    if (hal_if->if_type == intf::IF_TYPE_ENIC &&
        (hal_if->enic_type == intf::IF_ENIC_TYPE_USEG ||
         hal_if->enic_type == intf::IF_ENIC_TYPE_PVLAN)) {

        hal_if->egress_en = egress_en;
        pd_if_args.intf = hal_if;
        pd_if_args.egress_en_change = true;
        pd_if_args.egress_en = egress_en;

        pd_func_args.pd_if_update = &pd_if_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_UPDATE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to update if pd, err  : {}", ret);
            hal_if->egress_en = curr_egress_en;
        }
    }

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
        HAL_TRACE_ERR("unable to alloc memory");
        goto end;
    }
    l2seg_entry->l2seg_handle = l2seg->hal_handle;
    sdk::lib::dllist_reset(&l2seg_entry->lentry);
    sdk::lib::dllist_add(&hal_if->l2seg_list_clsc_head, &l2seg_entry->lentry);

    HAL_TRACE_DEBUG("L2segs:");
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
        HAL_TRACE_DEBUG("freeing l2seg handle: {}",
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
#if 0
//------------------------------------------------------------------------------
// Adds l2seg to the list for classic enicif
//------------------------------------------------------------------------------
hal_ret_t
enicif_classic_add_l2seg(if_t *hal_if, l2seg_t *l2seg)
{
    hal_ret_t               ret = HAL_RET_OK;

    if (hal_if == NULL || l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->l2seg_list_clsc->insert(&l2seg->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add l2seg {} to if {}",
                        l2seg->seg_id, hal_if->if_id);
        goto end;
    }

end:
    HAL_TRACE_DEBUG("Added l2seg {} to if {}", l2seg->seg_id, hal_if->if_id);
    return ret;
}

//----------------------------------------------------------------------------
// Checks if l2seg is present in classic enicif
//----------------------------------------------------------------------------
bool
l2seg_in_classic_enicif(if_t *hal_if, hal_handle_t l2seg_handle)
{
    hal_handle_t    *p_hdl_id = NULL;

    for (const void *ptr : *hal_if->l2seg_list_clsc) {
        p_hdl_id = (hal_handle_t *)ptr;
        if (*p_hdl_id == l2seg_handle) {
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
        HAL_TRACE_DEBUG("l2seg_handle : {}", entry->l2seg_handle);
    }
}
#endif

#if 0
// TODO: Deprecated
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
#endif

#if 0
// TODO: Deprecated
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
        HAL_TRACE_DEBUG("freeing l2seg handle : {}",
                        entry->l2seg_handle);
        // Remove from list
        sdk::lib::dllist_del(&entry->lentry);
        // Free the entry
        hal::delay_delete_to_slab(HAL_SLAB_ENIC_L2SEG_ENTRY, entry);
    }
}
#endif

#if 0
// TODO: Deprecated. Use hal_cleanup_handle_list()
hal_ret_t
enicif_cleanup_l2seg_entry_list(block_list **bl_list)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (*list == NULL) {
        return ret;
    }
    hal_remove_all_handles_block_list(*bl_list);
    block_list::destroy((*bl_list);
    // enicif_free_l2seg_entry_list(*list);
    // HAL_FREE(HAL_MEM_ALLOC_DLLIST, *list);
    *list = NULL;

    return ret;
}
#endif

//------------------------------------------------------------------------------
// Enic If Create
//------------------------------------------------------------------------------
hal_ret_t
enic_if_create (const InterfaceSpec& spec, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    l2seg_t             *l2seg;
    lif_t               *lif;
    vrf_t               *vrf = NULL;
    L2SegmentKeyHandle  l2seg_clsc_key_handle;

    HAL_TRACE_DEBUG("enicif create for id {} type : {}",
                    spec.key_or_handle().interface_id(),
                    spec.if_enic_info().enic_type());

    // lif for enic_if ... rsp is updated within the call
    ret = get_lif_handle_for_enic_if(spec, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to find the lif handle Err : {}",
                      ret);
        return ret;
    }

    auto if_enic_info = spec.if_enic_info();
    hal_if->enic_type = if_enic_info.enic_type();

    if (if_enic_info.has_pinned_uplink_if_key_handle()) {
        if (if_enic_info.pinned_uplink_if_key_handle().key_or_handle_case() == InterfaceKeyHandle::kInterfaceId) {
            hal_if->pinned_uplink = find_hal_handle_from_if_id(if_enic_info.pinned_uplink_if_key_handle().interface_id());
        } else {
            hal_if->pinned_uplink = if_enic_info.pinned_uplink_if_key_handle().if_handle();
        }
    }

    lif = find_lif_by_handle(hal_if->lif_handle);
    if (lif) {
        hal_if->lif_learned = false;
    }

    if (hal_if->enic_type == intf::IF_ENIC_TYPE_USEG ||
            hal_if->enic_type == intf::IF_ENIC_TYPE_PVLAN ||
            hal_if->enic_type == intf::IF_ENIC_TYPE_DIRECT) {
        l2seg = l2seg_lookup_key_or_handle(if_enic_info.mutable_enic_info()->l2segment_key_handle());
        if (l2seg == NULL) {
            HAL_TRACE_ERR("Failed to find l2seg_id : {}/{}",
                          if_enic_info.enic_info().l2segment_key_handle().segment_id(),
                          if_enic_info.enic_info().l2segment_key_handle().l2segment_handle());
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }

        hal_if->l2seg_handle = l2seg->hal_handle;

        // Fetch the vrf information from l2seg
        vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
        if(vrf == NULL ) {
            HAL_TRACE_ERR("Failed to find vrf using handle : {}",
                           l2seg->hal_handle);
            ret = HAL_RET_VRF_NOT_FOUND;
            goto end;
        }

        hal_if->tid = vrf->vrf_id;
        MAC_UINT64_TO_ADDR(hal_if->mac_addr,
                if_enic_info.mutable_enic_info()->mac_address());
        hal_if->encap_vlan = if_enic_info.mutable_enic_info()->encap_vlan_id();

        // Check filters to egress en
        ret = filter_check_enic(lif, hal_if, &hal_if->egress_en);

        /*
         * In host-pin mode:
         * - Every Enic should have a pinned uplink.
         * Case 1:
         *   Enic -> Lif -> pinned_uplink
         * Case 2:
         *   Enic -> pinned_uplink
         * Case 3:
         *   Enic -> pinned_uplink (dynamically allocated. Only for DOL)
         */
        HAL_TRACE_DEBUG("allow_dyn_pin: {}", g_hal_state->allow_dynamic_pinning());
        if (g_hal_state->allow_dynamic_pinning()) {
            if_t *uplink = NULL;
            // If either enic or lif doesnt have a pinned uplink, pick from uplinks.
            if_enicif_get_pinned_if(hal_if, &uplink);
            if (!uplink) {
                uplink = find_if_by_id(if_enicif_get_host_pinned_uplink(hal_if));
                if (uplink) {
                    HAL_TRACE_DEBUG("Dynamically picked pinned uplink: {}", uplink->if_id);
                    hal_if->pinned_uplink = uplink->hal_handle;
                }
            }
        }

        HAL_TRACE_DEBUG("l2_seg_id : {}, encap : {}, mac : {}, egress_en: {}",
                        l2seg->seg_id,
                        hal_if->encap_vlan, macaddr2str(hal_if->mac_addr),
                        hal_if->egress_en);

    } else if (hal_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC) {
        auto clsc_enic_info = if_enic_info.mutable_classic_enic_info();
        if (if_enic_info.mutable_classic_enic_info()->
                native_l2segment_id() != 0) {
            // Processing native l2seg
            l2seg = find_l2seg_by_id(if_enic_info.mutable_classic_enic_info()->
                                     native_l2segment_id());
#if 0
            l2seg = l2seg_lookup_by_handle(if_enic_info.
                    mutable_classic_enic_info()->native_l2segment_handle());
#endif
            if (l2seg == NULL) {
                HAL_TRACE_ERR("Failed to find l2seg_id : {}",
                              if_enic_info.mutable_classic_enic_info()->native_l2segment_id());
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
            HAL_TRACE_DEBUG("Adding l2seg_id : {} as native",
                            l2seg->seg_id);
            hal_if->native_l2seg_clsc = l2seg->hal_handle;
        }
        // Processing l2segments
        HAL_TRACE_DEBUG("Received {} number of l2segs",
                        clsc_enic_info->l2segment_key_handle_size());
        for (int i = 0; i < clsc_enic_info->l2segment_key_handle_size();
                i++) {
            l2seg_clsc_key_handle = clsc_enic_info->l2segment_key_handle(i);
            l2seg = l2seg_lookup_key_or_handle(l2seg_clsc_key_handle);
            if (l2seg == NULL) {
                HAL_TRACE_ERR("Failed to find l2seg_handle : {}",
                              l2seg_clsc_key_handle.l2segment_handle());
                ret = HAL_RET_L2SEG_NOT_FOUND;
                goto end;
            }
            enicif_classic_add_l2seg(hal_if, l2seg);
        }
    } else if (hal_if->enic_type == intf::IF_ENIC_TYPE_GFT) {

        MAC_UINT64_TO_ADDR(hal_if->mac_addr,
                if_enic_info.mutable_enic_info()->mac_address());
        HAL_TRACE_DEBUG("mac : {}", macaddr2str(hal_if->mac_addr));

    } else {
        HAL_TRACE_ERR("invalid enic type : {}",
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
uplink_if_create (const InterfaceSpec& spec, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;

#if 0
    hal_if->uplink_port_num = g_hal_state->catalog()->
        logical_port_to_tm_port(spec.if_uplink_info().port_num());
#endif

    ret = pltfm_get_port_from_front_port_num(spec.if_uplink_info().port_num(),
                                             &hal_if->uplink_port_num);
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, HAL_RET_INVALID_ARG);

    hal_if->fp_port_num = spec.if_uplink_info().port_num();
    hal_if->native_l2seg = spec.if_uplink_info().native_l2segment_id();
    hal_if->is_oob_management = spec.if_uplink_info().is_oob_management();
    hal_if->if_op_status = uplink_if_get_oper_state(hal_if->fp_port_num);
    HAL_TRACE_DEBUG("Uplink fp_port_num: {}, asic_port_num: {}, Status: {}, native_l2seg_id: {},"
                    "is_oob: {}",
                    hal_if->fp_port_num,
                    hal_if->uplink_port_num,
                    if_status_to_str(hal_if->if_op_status),
                    hal_if->native_l2seg,
                    hal_if->is_oob_management);

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

    HAL_TRACE_DEBUG("add Segment ID  : {} to uplinkpc_id : {}, ret : {}",
                    seg->seg_id, uppc->if_id, ret);
    return ret;
}
#endif

//------------------------------------------------------------------------------
// Uplink PC If Create
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_create (const InterfaceSpec& spec, if_t *hal_if)
{
    hal_ret_t    ret = HAL_RET_OK;
    InterfaceKeyHandle mbr_if_key_handle;
    // uint64_t     l2seg_id = 0;
    if_t         *mbr_if = NULL;
    // l2seg_t      *l2seg = NULL;

    if (!spec.has_if_uplink_pc_info()) {
        HAL_TRACE_DEBUG("no uplinkpcinfo. not much to process");
        goto end;
    }

    HAL_TRACE_DEBUG("native_l2seg_id : {}",
                    spec.if_uplink_pc_info().native_l2segment_id());

    hal_if->uplink_port_num = HAL_PORT_INVALID;
    // hal_if->uplink_pc_num = spec.if_uplink_pc_info().uplink_pc_num();
    hal_if->native_l2seg = spec.if_uplink_pc_info().native_l2segment_id();

    HAL_TRACE_DEBUG("adding {} no. of members",
                    spec.if_uplink_pc_info().member_if_key_handle_size());
    // Walk through member uplinks
    for (int i = 0; i < spec.if_uplink_pc_info().member_if_key_handle_size(); i++) {
        mbr_if_key_handle = spec.if_uplink_pc_info().member_if_key_handle(i);
        mbr_if = if_lookup_key_or_handle(mbr_if_key_handle);
        if (mbr_if == NULL || mbr_if->if_type != intf::IF_TYPE_UPLINK) {
            HAL_TRACE_ERR("Unable to add non-uplinkif. "
                          "Skipping if : {} , {}",
                          if_spec_keyhandle_to_str(mbr_if_key_handle),
						  (mbr_if == NULL) ? "Not Present" :
                          "Not Uplink If");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        uplinkpc_add_uplinkif(hal_if, mbr_if);
    }

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

static void
populate_mpls_tag_info (const types::MplsTag &mpls_tag_in, mpls_tag_t *mpls_tag_out)
{
    mpls_tag_out->label = mpls_tag_in.label();
    mpls_tag_out->exp   = mpls_tag_in.exp();
    mpls_tag_out->ttl   = mpls_tag_in.ttl();
    mpls_tag_out->bos   = 1;
    return;
}

//------------------------------------------------------------------------------
// Tunnel If Create
//------------------------------------------------------------------------------
hal_ret_t
tunnel_if_create (const InterfaceSpec& spec, if_t *hal_if)
{
    hal_ret_t  ret      = HAL_RET_OK;
    ep_t       *rtep_ep = NULL;
    uint32_t   lif_id   = LIF_ID_INVALID;

    HAL_TRACE_DEBUG("Tunnelif create for id {}",
                    spec.key_or_handle().interface_id());
    auto if_tunnel_info = spec.if_tunnel_info();
    hal_if->tid = if_tunnel_info.vrf_key_handle().vrf_id();
    hal_if->encap_type = if_tunnel_info.encap_type();
    /* Both source addr and dest addr have to be v4 or v6 */
    if (hal_if->encap_type ==
            TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        if ((if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
             !if_tunnel_info.vxlan_info().remote_tep().v4_addr()) ||
            (!if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
             if_tunnel_info.vxlan_info().remote_tep().v4_addr())) {
            ret = HAL_RET_IF_INFO_INVALID;
            // rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
            // rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            goto end;
        }
    }
    /* Both source addr and dest addr have to be v4 or v6 */
    if (hal_if->encap_type ==
               TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_GRE) {
        if ((if_tunnel_info.gre_info().source().v4_addr() &&
                    !if_tunnel_info.gre_info().destination().v4_addr()) ||
                   (!if_tunnel_info.gre_info().source().v4_addr() &&
                    if_tunnel_info.gre_info().destination().v4_addr())) {
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
    }
    /* Validations for MPLSoUDP encap. Check if all addresses are v4 */
    if (hal_if->encap_type ==
         TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS) {
        lif_id = g_hal_state->lif_name_id_map_find(if_tunnel_info.prop_mpls_info().lif_name());
        if (lif_id == LIF_ID_INVALID) {
            HAL_TRACE_ERR("LIF name not found");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }

        if ((!if_tunnel_info.prop_mpls_info().substrate_ip().v4_addr()) ||
            (!if_tunnel_info.prop_mpls_info().tunnel_dest_ip().v4_addr())) {
            HAL_TRACE_ERR("Substrate IP or Tun dest IP is not v4 type");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        int n = if_tunnel_info.prop_mpls_info().overlay_ip_size();
        for (int i = 0; i < n; i++) {
            if (!if_tunnel_info.prop_mpls_info().overlay_ip(i).v4_addr()) {
                HAL_TRACE_ERR("Overlay IP is not v4 type");
                ret = HAL_RET_IF_INFO_INVALID;
                goto end;
            }
        }
        if (if_tunnel_info.prop_mpls_info().mpls_if_size() != n) {
            HAL_TRACE_ERR("Num overlay_ip and mpls_if size mismatch!");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        hal_if->num_overlay_ip = hal_if->num_mpls_if = n;
        /* Validate the source gw address */
        auto addr = if_tunnel_info.prop_mpls_info().source_gw();
        if (!addr.has_prefix()) {
            HAL_TRACE_ERR("ONLY Ipv4 prefix supported for MplsoUDP source gw");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
        auto prefix = addr.prefix();
        if (!prefix.has_ipv4_subnet()) {
            HAL_TRACE_ERR("ONLY Ipv4 prefix supported for MplsoUDP source gw");
            ret = HAL_RET_IF_INFO_INVALID;
            goto end;
        }
    }

    /* Populate the interface structures */
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
    } else if (hal_if->encap_type ==
               TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS) {
        ip_addr_spec_to_ipv4_addr(&hal_if->substrate_ip,
                                if_tunnel_info.prop_mpls_info().substrate_ip());
        ip_addr_spec_to_ipv4_addr(&hal_if->tun_dst_ip,
                                if_tunnel_info.prop_mpls_info().tunnel_dest_ip());
        /* Already validated that num_overlay_ip == num_mpls_if */
        for (int i = 0; i < hal_if->num_overlay_ip; i++) {
            populate_mpls_tag_info(if_tunnel_info.prop_mpls_info().mpls_if(i),
                                   &hal_if->mpls_if[i]);
            ip_addr_spec_to_ipv4_addr(&hal_if->overlay_ip[i],
                                    if_tunnel_info.prop_mpls_info().overlay_ip(i));
        }
        populate_mpls_tag_info(if_tunnel_info.prop_mpls_info().mpls_tag(),
                               &hal_if->mpls_tag);
        /* TODO: Check if any validations required for bandwidth ?? */
        hal_if->ingress_bw = if_tunnel_info.prop_mpls_info().ingress_bw();
        hal_if->egress_bw = if_tunnel_info.prop_mpls_info().egress_bw();

        auto addr = if_tunnel_info.prop_mpls_info().source_gw();
        auto prefix = addr.prefix();
        hal_if->source_gw.len = prefix.ipv4_subnet().prefix_len();
        hal_if->source_gw.v4_addr = prefix.ipv4_subnet().address().v4_addr();
        MAC_UINT64_TO_ADDR(hal_if->gw_mac_da,
                           if_tunnel_info.prop_mpls_info().gw_mac_da());
        MAC_UINT64_TO_ADDR(hal_if->overlay_mac,
                           if_tunnel_info.prop_mpls_info().overlay_mac());
        HAL_TRACE_DEBUG("Overlay mac: {}",
                if_tunnel_info.prop_mpls_info().overlay_mac());
        MAC_UINT64_TO_ADDR(hal_if->pf_mac,
                           if_tunnel_info.prop_mpls_info().pf_mac());
        HAL_TRACE_DEBUG("PF mac: {}",
                if_tunnel_info.prop_mpls_info().pf_mac());
        hal_if->lif_id = lif_id;
    } else {
        ret = HAL_RET_IF_INFO_INVALID;
        HAL_TRACE_ERR("Unsupported encap type : {}", hal_if->encap_type);
        goto end;
    }

    // Get remote tep EP
    if (hal_if->encap_type != TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS) {
        rtep_ep = tunnel_if_get_remote_tep_ep(hal_if);
        if (!rtep_ep) {
            ret = HAL_RET_IF_INFO_INVALID;
            HAL_TRACE_ERR("Unable to find rtep ep for IP : {}, ret : {}",
                          hal_if->encap_type == TNNL_ENC_TYPE::IF_TUNNEL_ENCAP_TYPE_VXLAN ?
                          ipaddr2str(&hal_if->vxlan_rtep) :
                          ipaddr2str(&hal_if->gre_dest), ret);
            goto end;
        }
        hal_if->rtep_ep_handle = rtep_ep->hal_handle;
    }

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
        HAL_TRACE_ERR("spec has no key or handle");
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

    // check for no reference by mc entries
    if (dllist_count(&hal_if->mc_entry_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("multicast entries still referring:");
        hal_print_handles_list(&hal_if->mc_entry_list_head);
    }

    // check for no presence of l2segs
    if (hal_if->l2seg_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("l2segs still referring:");
        hal_print_handles_block_list(hal_if->l2seg_list);
    }

    // check if the uplink is not a member of PC
    if (hal_if->is_pc_mbr) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("PC is still referring. PC's handle : {}",
                      hal_if->uplinkpc_handle);
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

    // check for no reference by mc entries
    if (dllist_count(&hal_if->mc_entry_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("multicast entries still referring:");
        hal_print_handles_list(&hal_if->mc_entry_list_head);
    }

    // check for no presence of l2segs
    if (hal_if->l2seg_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("Failed to delete uplink PC, l2segs still referring");
        hal_print_handles_block_list(hal_if->l2seg_list);
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
        ret = HAL_RET_INVALID_OP;
        break;
    case intf::IF_TYPE_APP_REDIR:
        break;
    default:
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("invalid if type");
    }

    for (unsigned i = 0;
         (ret == HAL_RET_OK) && i < SDK_ARRAY_SIZE(hal_if->acl_list); i++) {
        if (hal_if->acl_list[i]->num_elems()) {
            ret = HAL_RET_OBJECT_IN_USE;
            HAL_TRACE_ERR("If delete failure, acls still referring {}:",
                          static_cast<if_acl_ref_type_t>(i));
            hal_print_handles_block_list(hal_if->acl_list[i]);
            goto end;
        }
    }

    if (hal_if->nh_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("If delete failure, NHs still referring:");
        hal_print_handles_block_list(hal_if->nh_list);
        goto end;
    }

    if (hal_if->ep_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("If delete failure, EPs still referring:");
        hal_print_handles_block_list(hal_if->ep_list);
        goto end;
    }

end:
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
    pd::pd_func_args_t          pd_func_args = {0};

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the if.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    intf = (if_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del cb {}", intf->if_id);

    // First of all, delete the interface from all the relevant OIF lists.
    // This needs to be done before the PD call to delete the interface.
    ret = if_update_oif_lists(intf, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to remove if from OIFs");
        goto end;
    }

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_if_delete_args_init(&pd_if_args);
    pd_if_args.intf = intf;
    pd_func_args.pd_if_delete = &pd_if_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete if pd, err : {}", ret);
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

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    intf = (if_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {}",
                    intf->if_id);

    if (intf->if_type == intf::IF_TYPE_ENIC) {
        if (intf->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
            // Remove from l2seg
            l2seg = l2seg_lookup_by_handle(intf->l2seg_handle);
            ret = l2seg_del_back_if(l2seg, intf);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to remove if from l2seg");
                goto end;
            }

            // Remove from lif
            lif = find_lif_by_handle(intf->lif_handle);
            if (lif) {
                ret = lif_del_if(lif, intf);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to remove if from lif");
                    goto end;
                }
            }
        } else {
            // Remove from lif
            lif = find_lif_by_handle(intf->lif_handle);
            if (lif) {
                ret = lif_del_if(lif, intf);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to remove if from lif");
                    goto end;
                }
            }

            // Del to uplink's back refs
            if (intf->pinned_uplink != HAL_HANDLE_INVALID) {
                uplink = find_if_by_handle(intf->pinned_uplink);
                if (uplink == NULL) {
                    HAL_TRACE_ERR("Unable to find uplink_hdl : {}",
                                  intf->pinned_uplink);
                    goto end;
                }
                ret = uplink_del_enicif(uplink, intf);
                SDK_ASSERT(ret == HAL_RET_OK);
            }
            // Del from native l2seg's back ref
            if (intf->native_l2seg_clsc != HAL_HANDLE_INVALID) {
                nat_l2seg = l2seg_lookup_by_handle(intf->native_l2seg_clsc);
                if (nat_l2seg == NULL) {
                    HAL_TRACE_ERR("Unable to find native_l2seg_hdl : {}",
                                  intf->native_l2seg_clsc);
                    goto end;
                }

                ret = l2seg_del_back_if(nat_l2seg, intf);
                SDK_ASSERT(ret == HAL_RET_OK);
            }

            //  - Del back refs to all l2segs
            ret = enicif_update_l2segs_relation(&intf->l2seg_list_clsc_head,
                                                intf, false);
            SDK_ASSERT(ret == HAL_RET_OK);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to del l2seg -/-> enicif "
                              "relation ret : {}",
                              ret);
                goto end;
            }

            // Free up the l2seg list
            enicif_free_l2seg_entry_list(&intf->l2seg_list_clsc_head);
        }
    }

    // Uplink PC: Remove relations from mbrs
    if (intf->if_type == intf::IF_TYPE_UPLINK_PC) {
        // Del relation from mbr uplink if to PC
        ret = uplinkpc_update_mbrs_relation(intf->mbr_if_list,
                                            intf, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del uplinkif -/-> uplinkpc "
                          "relation ret : {}",
                          ret);
            goto end;
        }

        // clean up mbr if list
        HAL_TRACE_DEBUG("cleaning up mbr list");
        hal_remove_all_handles_block_list(intf->mbr_if_list);
    }

    if (intf->if_type == intf::IF_TYPE_ENIC) {
        // Del relation from l2seg to enicifs
        ret = enicif_update_l2segs_relation(&intf->l2seg_list_clsc_head,
                                            intf, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del l2seg -/-> enicif "
                          "relation ret : {}",
                          ret);
            goto end;
        }
    }

    if ((intf->if_type == intf::IF_TYPE_UPLINK) ||
        (intf->if_type == intf::IF_TYPE_UPLINK_PC)) {
        g_num_uplink_ifs--;
        g_uplink_if_ids.erase(std::remove(g_uplink_if_ids.begin(), g_uplink_if_ids.end(), intf->if_id), g_uplink_if_ids.end());
    }

    // a. Remove from if id hash table
    ret = if_del_from_db(intf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del if {} from db, err : {}",
                      intf->if_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI if
    if_cleanup(intf);

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

    hal_api_trace(" API Begin: Interface delete ");
    proto_msg_dump(req);

    // validate the request message
    ret = validate_if_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("if delete request validation Failed, ret : {}", ret);
        goto end;
    }

    hal_if = if_lookup_key_or_handle(kh);
    if (hal_if == NULL) {
        HAL_TRACE_ERR("Failed to find if, id {}, handle {}", kh.interface_id(),
                      kh.if_handle());
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("if delete for id {} type : {} enictype : {}",
                    hal_if->if_id,
                    IfType_Name(hal_if->if_type),
                    (hal_if->if_type == intf::IF_TYPE_ENIC) ?
                    IfEnicType_Name(hal_if->enic_type) : "IF_ENIC_TYPE_NONE");

    ret = validate_if_delete(hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("if delete validation Failed, ret : {}", ret);
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
    return ret;
}

//------------------------------------------------------------------------------
// Get lif handle from spec
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_from_spec (const InterfaceSpec& spec, hal_handle_t *lif_handle)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_id_t            lif_id = 0;
    lif_t               *lif = NULL;

    *lif_handle = HAL_HANDLE_INVALID;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_enic_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else if (lif_kh.key_or_handle_case() == LifKeyHandle::kLifHandle) {
        *lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(*lif_handle);
    } else {
        HAL_TRACE_DEBUG("lif not provided.");
        goto end;
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("lif handle not found for id : {} hdl : {}",
                      lif_id, *lif_handle);
        *lif_handle = HAL_HANDLE_INVALID;
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    *lif_handle = lif->hal_handle;
end:
    return ret;
}

//------------------------------------------------------------------------------
// Get lif handle
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_for_enic_if (const InterfaceSpec& spec, if_t *hal_if)
{
    lif_id_t            lif_id = 0;
    hal_handle_t        lif_handle = 0;
    lif_t               *lif = NULL;
    hal_ret_t           ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_enic_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else if (lif_kh.key_or_handle_case() == LifKeyHandle::kLifHandle) {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    } else {
        HAL_TRACE_DEBUG("lif not provided.");
        goto end;
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("lif handle not found for id : {} hdl : {}",
                      lif_id, lif_handle);
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
get_lif_handle_for_cpu_if (const InterfaceSpec& spec, if_t *hal_if)
{
    lif_id_t        lif_id = 0;
    hal_handle_t    lif_handle = 0;
    lif_t           *lif = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_cpu_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("PI-CPUif:LIF handle not found for ID : {} HDL : {}",
                      lif_id, lif_handle);
        // rsp->set_api_status(types::API_STATUS_NOT_FOUND);
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
get_lif_handle_for_app_redir_if (const InterfaceSpec& spec, if_t *hal_if)
{
    lif_id_t        lif_id = 0;
    hal_handle_t    lif_handle = 0;
    lif_t           *lif = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_app_redir_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("LIF handle not found for ID : {} HDL : {}",
                      lif_id, lif_handle);
        // rsp->set_api_status(types::API_STATUS_NOT_FOUND);
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
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_if_nwsec_update_args_t   args;
    pd::pd_func_args_t              pd_func_args = {0};

    HAL_TRACE_DEBUG("if_id : {}", hal_if->if_id);
    pd::pd_if_nwsec_update_args_init(&args);
    args.l2seg = l2seg;
    args.intf = hal_if;
    args.nwsec_prof = nwsec_prof;

    pd_func_args.pd_if_nwsec_update = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_NWSEC_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call for nwsec update on if Failed. ret : {}", ret);
        goto end;
    }

end:

    return ret;
}

hal_ret_t
if_handle_lif_update (pd::pd_if_lif_update_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_func_args_t          pd_func_args = {0};

    if (args == NULL) {
        HAL_TRACE_ERR("args is NULL");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("if_id : {}", args->intf->if_id);

    pd_func_args.pd_if_lif_update = args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_LIF_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call for lif update on if Failed. ret : {}",
                ret);
        goto end;
    }

end:

    return ret;
}

bool
mbrif_in_pc (if_t *up_pc, hal_handle_t mbr_handle)
{
    hal_handle_t            *p_hdl_id = NULL;

    for (const void *ptr : *up_pc->mbr_if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        if (*p_hdl_id == mbr_handle) {
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
    HAL_TRACE_DEBUG("number of l2segs:{}",
                    num_l2segs);
    for (i = 0; i < num_l2segs; i++) {
        l2seg_key_handle = clsc_enic_info->l2segment_key_handle(i);
        l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);
        if (l2seg == NULL) {
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        if (l2seg_in_classic_enicif(hal_if, l2seg->hal_handle, NULL)) {
            continue;
        } else {
            // Add to added list
            enicif_add_to_l2seg_entry_list(*add_l2seglist, l2seg->hal_handle);
            *l2seglist_change = true;
            HAL_TRACE_DEBUG("added to add list hdl: {}",
                    l2seg->hal_handle);
        }
    }

    HAL_TRACE_DEBUG("Existing l2segs:");
    enicif_print_l2seg_entry_list(&hal_if->l2seg_list_clsc_head);
    HAL_TRACE_DEBUG("added l2segs:");
    enicif_print_l2seg_entry_list(*add_l2seglist);

    dllist_for_each(lnode, &(hal_if->l2seg_list_clsc_head)) {
        entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
#if 0
        HAL_TRACE_DEBUG("Checking for l2seg: {}",
                entry->l2seg_handle);
#endif
        for (i = 0; i < num_l2segs; i++) {
            l2seg_key_handle = clsc_enic_info->l2segment_key_handle(i);
            l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);
            // HAL_TRACE_DEBUG("grpc l2seg handle: {}", l2seg->hal_handle);
            if (entry->l2seg_handle == l2seg->hal_handle) {
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
            HAL_TRACE_DEBUG("added to delete list hdl: {}",
                    lentry->l2seg_handle);
        }
        l2seg_exists = false;
    }

    HAL_TRACE_DEBUG("deleted l2segs:");
    enicif_print_l2seg_entry_list(*del_l2seglist);

    if (!*l2seglist_change) {
        // Got same mbrs as existing
        enicif_cleanup_l2seg_entry_list(add_l2seglist);
        enicif_cleanup_l2seg_entry_list(del_l2seglist);
    }
end:
    return ret;
}
#if 0
//----------------------------------------------------------------------------
// Handle classic enicif l2seg list change
//----------------------------------------------------------------------------
hal_ret_t
enic_if_upd_l2seg_list_update(InterfaceSpec& spec, if_t *hal_if,
                              bool *l2seglist_change,
                              block_list **add_l2seglist,
                              block_list **del_l2seglist)
{
    hal_ret_t                       ret = HAL_RET_OK;
    uint16_t                        num_l2segs = 0, i = 0;
    dllist_ctxt_t                   *lnode = NULL;
    bool                            l2seg_exists = false;
    L2SegmentKeyHandle              l2seg_key_handle;
    l2seg_t                         *l2seg = NULL;
    if_l2seg_entry_t                *entry = NULL, *lentry = NULL;
    hal_handle_t                    *p_hdl_id = NULL;

    *l2seglist_change = false;

    auto if_enic_info = spec.if_enic_info();
    auto clsc_enic_info = if_enic_info.mutable_classic_enic_info();

    *add_l2seglist = block_list::factory(sizeof(hal_handle_t));
    *del_l2seglist = block_list::factory(sizeof(hal_handle_t));

    num_l2segs = clsc_enic_info->l2segment_key_handle_size();
    HAL_TRACE_DEBUG("number of l2segs  : {}",
                    num_l2segs);
    for (i = 0; i < num_l2segs; i++) {
        l2seg_key_handle = clsc_enic_info->l2segment_key_handle(i);
        l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);
        if (l2seg == NULL) {
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        if (l2seg_in_classic_enicif(hal_if, l2seg->hal_handle)) {
            continue;
        } else {
            // add to "add" list
            hal_add_to_handle_block_list(*add_l2seglist, l2seg->hal_handle);
            *l2seglist_change = true;
            HAL_TRACE_DEBUG("added to add list hdl : {}",
                    l2seg->hal_handle);
        }
    }

    HAL_TRACE_DEBUG("Existing l2segs:");
    hal_print_handles_block_list(hal_if->l2seg_list_clsc);
    HAL_TRACE_DEBUG("added l2segs:");
    hal_print_handles_block_list(*add_l2seglist);

    for (const void *ptr : *hal_if->mbr_if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("Checking for l2seg : {}", *p_hdl_id);
        for (i = 0; i < num_l2segs; i++) {
            l2seg_key_handle = clsc_enic_info->l2segment_key_handle(i);
            HAL_TRACE_DEBUG("grpc l2seg handle : {}", l2seg->hal_handle);
            if (*p_hdl_id == l2seg_key_handle.l2segment_handle()) {
                l2seg_exists = true;
                break;
            } else {
                continue;
            }
        }
        if (!l2seg_exists) {
            // add to delete list
            hal_add_to_handle_block_list(*del_l2seglist, *p_hdl_id);
            *l2seglist_change = true;
            HAL_TRACE_DEBUG("added to delete list hdl : {}", *p_hdl_id);
        }
        l2seg_exists = false;
    }

    HAL_TRACE_DEBUG("deleted l2segs:");
    hal_print_handles_block_list(*del_l2seglist);

    if (!*l2seglist_change) {
        // Got same mbrs as existing
        // enicif_cleanup_l2seg_entry_list(add_l2seglist);
        // enicif_cleanup_l2seg_entry_list(del_l2seglist);
        hal_cleanup_handle_block_list(add_l2seglist);
        hal_cleanup_handle_block_list(del_l2seglist);
    }

end:
    return ret;
}
#endif

//----------------------------------------------------------------------------
// Handle uplink pc mbr list update
//----------------------------------------------------------------------------
hal_ret_t
uplinkpc_mbr_list_update(InterfaceSpec& spec, if_t *hal_if,
                         bool *mbrlist_change,
                         block_list **add_mbrlist,
                         block_list **del_mbrlist,
                         block_list **aggr_mbrlist)
{
    hal_ret_t                       ret = HAL_RET_OK;
    uint16_t                        num_mbrs = 0, i = 0;
    bool                            mbr_exists = false;
    InterfaceKeyHandle              mbr_if_key_handle;
    if_t                            *mbr_if = NULL;
    hal_handle_t                    *p_hdl_id = NULL;

    *mbrlist_change = false;

    *add_mbrlist = block_list::factory(sizeof(hal_handle_t));
    *del_mbrlist = block_list::factory(sizeof(hal_handle_t));
    *aggr_mbrlist = block_list::factory(sizeof(hal_handle_t));

    num_mbrs = spec.if_uplink_pc_info().member_if_key_handle_size();
    HAL_TRACE_DEBUG("pc mbrs  : {}",
                    num_mbrs);
    for (i = 0; i < num_mbrs; i++) {
        mbr_if_key_handle = spec.if_uplink_pc_info().member_if_key_handle(i);
        mbr_if = if_lookup_key_or_handle(mbr_if_key_handle);
        if (mbr_if == NULL) {
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        // add to aggr list
        hal_add_to_handle_block_list(*aggr_mbrlist, mbr_if->hal_handle);

        if (mbr_if->if_type != intf::IF_TYPE_UPLINK) {
            HAL_TRACE_ERR("Unable to add non-uplinkif. "
                          "Skipping if id : {}", mbr_if->if_id);
            continue;
        }
        if (mbrif_in_pc(hal_if, mbr_if->hal_handle)) {
            continue;
        } else {
            // add to "add" list
            hal_add_to_handle_block_list(*add_mbrlist, mbr_if->hal_handle);
            *mbrlist_change = true;
            HAL_TRACE_DEBUG("added to add list hdl : {}",
                    mbr_if->hal_handle);
        }
    }

    HAL_TRACE_DEBUG("Existing mbrs:");
    hal_print_handles_block_list(hal_if->mbr_if_list);
    HAL_TRACE_DEBUG("New Aggregated mbrs:");
    hal_print_handles_block_list(*aggr_mbrlist);
    HAL_TRACE_DEBUG("added mbrs:");
    hal_print_handles_block_list(*add_mbrlist);

    for (const void *ptr : *hal_if->mbr_if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("Checking for mbr : {}", *p_hdl_id);
        for (i = 0; i < num_mbrs; i++) {
            mbr_if_key_handle = spec.if_uplink_pc_info().member_if_key_handle(i);
            mbr_if = if_lookup_key_or_handle(mbr_if_key_handle);
            HAL_TRACE_DEBUG("grpc mbr handle : {}", mbr_if->hal_handle);
            if (*p_hdl_id == mbr_if->hal_handle) {
                mbr_exists = true;
                break;
            } else {
                continue;
            }
        }
        if (!mbr_exists) {
            // add to delete list
            hal_add_to_handle_block_list(*del_mbrlist, *p_hdl_id);
            *mbrlist_change = true;
            HAL_TRACE_DEBUG("added to delete list hdl {}", *p_hdl_id);
        }
        mbr_exists = false;
    }

    HAL_TRACE_DEBUG("deleted mbrs:");
    hal_print_handles_block_list(*del_mbrlist);

#if 0
    if (!*mbrlist_change) {
        // Got same mbrs as existing
        // interface_cleanup_handle_list(add_mbrlist);
        // interface_cleanup_handle_list(del_mbrlist);
        // interface_cleanup_handle_list(aggr_mbrlist);

        hal_cleanup_handle_block_list(add_mbrlist);
        hal_cleanup_handle_block_list(del_mbrlist);
        hal_cleanup_handle_block_list(aggr_mbrlist);
    }
#endif

end:

    return ret;
}

bool
enicif_is_swm (if_t *hal_if)
{
    lif_t *lif = find_lif_by_handle(hal_if->lif_handle);
    return (lif && lif->type == types::LIF_TYPE_SWM);
}

hal_ret_t
enicif_clsc_l2seglist_change_update_oiflists(if_t *hal_if,
                                             dllist_ctxt_t *l2seg_list,
                                             bool add)
{
    hal_ret_t        ret = HAL_RET_OK;
    dllist_ctxt_t    *curr, *next;
    if_l2seg_entry_t *entry = NULL;
    l2seg_t          *l2seg = NULL;

    lif_t *lif = find_lif_by_handle(hal_if->lif_handle);

    dllist_for_each_safe(curr, next, l2seg_list) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);

        ret = enicif_classic_update_oif_lists(hal_if, l2seg, lif, add);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add if:{} to l2seg oiflists. ret: {}",
                          if_keyhandle_to_str(hal_if),
                          ret);
            goto end;
        }
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// Add/Del Oifs to/from Oif Lists for all l2segs in the list
//----------------------------------------------------------------------------
hal_ret_t
enicif_update_l2segs_oif_lists(if_t *hal_if, lif_t *lif, bool add)
{
    hal_ret_t        ret = HAL_RET_OK;
    dllist_ctxt_t    *curr, *next;
    if_l2seg_entry_t *entry = NULL;
    l2seg_t          *l2seg = NULL;
    dllist_ctxt_t    *l2segs_list = &hal_if->l2seg_list_clsc_head;

    dllist_for_each_safe(curr, next, l2segs_list) {
        entry = dllist_entry(curr, if_l2seg_entry_t, lentry);
        l2seg = l2seg_lookup_by_handle(entry->l2seg_handle);

        SDK_ASSERT(l2seg);

        ret = enicif_classic_update_oif_lists(hal_if, l2seg, lif, add);
    }

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
            HAL_TRACE_ERR("unable to find l2seg with handle:{}",
                          entry->l2seg_handle);
            ret = HAL_RET_L2SEG_NOT_FOUND;
            goto end;
        }
        if (add) {
            ret = l2seg_add_back_if(l2seg, hal_if);
        } else {
            ret = l2seg_del_back_if(l2seg, hal_if);
        }
    }

end:
    return ret;
}
//----------------------------------------------------------------------------
// Add/Del relation uplinkif -> uplinkpc for all mbrs in the list
//----------------------------------------------------------------------------
hal_ret_t
uplinkpc_update_mbrs_relation (block_list *mbr_list, if_t *uppc, bool add)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    *p_hdl_id = NULL;
    if_t            *up_if = NULL;

    for (const void *ptr : *mbr_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        up_if = find_if_by_handle(*p_hdl_id);
        if (!up_if) {
            HAL_TRACE_ERR("Unable to find uplinkif with handle {}",
                          *p_hdl_id);
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
        HAL_TRACE_ERR("invalid args");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(upif, __FILENAME__, __LINE__, __func__);

    upif->is_pc_mbr = true;
    upif->uplinkpc_handle = uppc->hal_handle;

    if_unlock(upif, __FILENAME__, __LINE__, __func__);

end:

    HAL_TRACE_DEBUG("add uplinkif => uplinkpc , {} => {}",
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
        HAL_TRACE_ERR("invalid args");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(upif, __FILENAME__, __LINE__, __func__);

    upif->is_pc_mbr = false;
    upif->uplinkpc_handle = HAL_HANDLE_INVALID;

    if_unlock(upif, __FILENAME__, __LINE__, __func__);


end:
    HAL_TRACE_DEBUG("del uplinkif =/=> uplinkpc , {} =/=> {}",
                    upif->if_id, uppc->if_id);
    return ret;
}


//-----------------------------------------------------------------------------
// Adds uplinkif into uplinkpc's member list
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_add_uplinkif (if_t *uppc, if_t *upif)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (uppc == NULL || upif == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(uppc, __FILENAME__, __LINE__, __func__);      // lock
    ret = uppc->mbr_if_list->insert(&upif->hal_handle);
    if_unlock(uppc, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add upif {} to uppc {}",
                        upif->if_id, uppc->if_id);
        goto end;
    }

end:
    HAL_TRACE_DEBUG("Added upif {} to uppc {}", upif->if_id, uppc->if_id);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove UplinkIf from Uplink PC's member list
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_del_uplinkif (if_t *uppc, if_t *upif)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (uppc == NULL || upif == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(uppc, __FILENAME__, __LINE__, __func__);      // lock
    ret = uppc->mbr_if_list->remove(&upif->hal_handle);
    if_unlock(uppc, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add upif {} to uppc {}",
                        upif->if_id, uppc->if_id);
        goto end;
    }

end:
    HAL_TRACE_DEBUG("Delete upif {} from uppc {}", upif->if_id, uppc->if_id);
    return ret;
}

//-----------------------------------------------------------------------------
// Adds l2segs into if list
//-----------------------------------------------------------------------------
hal_ret_t
if_add_l2seg (if_t *hal_if, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (hal_if == NULL || l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->l2seg_list->insert(&l2seg->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add l2seg {} to if {}",
                        l2seg->seg_id, hal_if->if_id);
        goto end;
    }

    HAL_TRACE_DEBUG("Added l2seg {} to if {}", l2seg->seg_id, hal_if->if_id);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Remove l2seg from if list
//-----------------------------------------------------------------------------
hal_ret_t
if_del_l2seg (if_t *hal_if, l2seg_t *l2seg)
{

    hal_ret_t                   ret = HAL_RET_OK;

    if (hal_if == NULL || l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->l2seg_list->remove(&l2seg->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add l2seg {} to vrf {}",
                        l2seg->seg_id, hal_if->if_id);
        goto end;
    }

    HAL_TRACE_DEBUG("Deleted l2seg {} to if {}", l2seg->seg_id, hal_if->if_id);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Adds nh to if back refs
//-----------------------------------------------------------------------------
hal_ret_t
if_add_nh (if_t *hal_if, nexthop_t *nh)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (hal_if == NULL || nh == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->nh_list->insert(&nh->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add nh {} to hal_if {}",
                        nh->nh_id, hal_if->if_id);
        goto end;
    }

    HAL_TRACE_DEBUG("Added nh {} to if {}", nh->nh_id, hal_if->if_id);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Remove nh from if
//-----------------------------------------------------------------------------
hal_ret_t
if_del_nh (if_t *hal_if, nexthop_t *nh)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (hal_if == NULL || nh == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->nh_list->remove(&nh->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add nh {} to hal_if {}",
                        nh->nh_id, hal_if->if_id);
        goto end;
    }

    HAL_TRACE_DEBUG("Deleted nh {} from hal_if {}", nh->nh_id, hal_if->if_id);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Adds EP to if back refs
//-----------------------------------------------------------------------------
hal_ret_t
if_add_ep (if_t *hal_if, ep_t *ep)
{
    hal_ret_t ret = HAL_RET_OK;

    if (hal_if == NULL || ep == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->ep_list->insert(&ep->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add ep {} to hal_if {}",
                        ep_l2_key_to_str(ep), if_keyhandle_to_str(hal_if));
        goto end;
    }

    HAL_TRACE_DEBUG("Added ep {} to if {}", ep_l2_key_to_str(ep),
                    if_keyhandle_to_str(hal_if));
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Remove EP from if
//-----------------------------------------------------------------------------
hal_ret_t
if_del_ep (if_t *hal_if, ep_t *ep)
{
    hal_ret_t ret = HAL_RET_OK;

    if (hal_if == NULL || ep == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->ep_list->remove(&ep->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to del nh {} from hal_if {}",
                        ep_l2_key_to_str(ep), if_keyhandle_to_str(hal_if));
        goto end;
    }

    HAL_TRACE_DEBUG("Deleted ep {} from hal_if {}",
                    ep_l2_key_to_str(ep), if_keyhandle_to_str(hal_if));
end:
    return ret;
}

void
port_event_cb (port_event_info_t *port_event_info)
{
    if_port_timer_ctxt_t *ctxt =
        (if_port_timer_ctxt_t *)g_hal_state->port_timer_ctxt_slab()->alloc();
    ctxt->port_num   = port_event_info->logical_port;
    ctxt->event      = port_event_info->event;
    ctxt->port_speed = port_event_info->speed;
    ctxt->port_type  = port_event_info->type;
    HAL_TRACE_DEBUG("Starting port timer. Port: {}, Event: {}, Speed: {}",
                    port_event_info->logical_port,
                    (uint32_t)port_event_info->event,
                    (uint32_t)port_event_info->speed);
    // Start a timer
    sdk::lib::timer_schedule(sdk::linkmgr::SDK_TIMER_ID_PORT_EVENT,
                             250, /* Milliseconds */
                             ctxt,
                             (sdk::lib::twheel_cb_t)port_event_timer_cb,
                             false);
}

// update admin state of all ports
static void
port_update_admin_state (port_admin_state_t admin_state)
{
    uint32_t fp_port;
    uint32_t ifindex;
    uint32_t logical_port;
    hal_ret_t ret = HAL_RET_OK;
    port_args_t port_args = { 0 };
    sdk::lib::catalog *catalog = g_hal_state->catalog();
    uint32_t num_fp_ports = catalog->num_fp_ports();
    uint64_t stats_data[MAX_MAC_STATS];

    memset(stats_data, 0, sizeof(uint64_t) * MAX_MAC_STATS);

    for (fp_port = 1; fp_port <= num_fp_ports; ++fp_port) {
        sdk::linkmgr::port_args_init(&port_args);

        ifindex = ETH_IFINDEX(
                catalog->slot(), fp_port, ETH_IF_DEFAULT_CHILD_PORT);
        logical_port = port_args.port_num =
            sdk::lib::catalog::ifindex_to_logical_port(ifindex);

        port_args.port_num = logical_port;
        port_args.stats_data = stats_data;

        ret = linkmgr::port_get(&port_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("port_get failed for {}", logical_port);
            continue;
        }
        port_args.admin_state = admin_state;

        hal_cfg_db_open(CFG_OP_WRITE);

        // update the port params
        ret = linkmgr::port_update(&port_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("port_update failed for {}", logical_port);
        }

        hal_cfg_db_close();
    }
}

//------------------------------------------------------------------------------
// port event cb
//------------------------------------------------------------------------------
sdk_ret_t
port_event_timer_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    static bool update_admin_state = true;

    if_port_timer_ctxt_t *port_ctxt = (if_port_timer_ctxt_t *)ctxt;
    port_event_info_t port_event_info;

    port_event_info.logical_port = port_ctxt->port_num;
    port_event_info.event = port_ctxt->event;
    port_event_info.speed = port_ctxt->port_speed;
    port_event_info.type  = port_ctxt->port_type;
    port_event_info.num_lanes = port_ctxt->num_lanes;
    port_event_info.oper_status = port_ctxt->oper_status;

    if ((update_admin_state == true) &&
        (port_ctxt->port_num == 9) &&
        (port_ctxt->event == port_event_t::PORT_EVENT_LINK_UP) &&
        (sdk::linkmgr::port_default_admin_state() ==
                port_admin_state_t::PORT_ADMIN_STATE_UP))  {
        HAL_TRACE_DEBUG("Updating admin state for all ports");
        port_update_admin_state(port_admin_state_t::PORT_ADMIN_STATE_UP);
        update_admin_state = false;
    }

    HAL_TRACE_DEBUG("Timer fired. Port: {}, Event: {}, Speed: {}",
                    port_ctxt->port_num, (uint32_t)port_ctxt->event,
                    (uint32_t)port_ctxt->port_speed);
    hal_stream_port_status_update(port_event_info);
    sdk::linkmgr::port_set_leds(port_ctxt->port_num, port_ctxt->event);
    if_port_oper_state_process_event(port_ctxt->port_num, port_ctxt->event);
    linkmgr::port_event_notify(&port_event_info);
    linkmgr::ipc::port_event_notify(&port_event_info);

    // Free ctxt
    hal::delay_delete_to_slab(HAL_SLAB_PORT_TIMER_CTXT, ctxt);

    return sdk::SDK_RET_OK;
}

//------------------------------------------------------------------------------
// Interface walk callback for port event
//------------------------------------------------------------------------------
static bool
if_port_event_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    if_port_event_cb_ctxt_t *cb_ctxt = (if_port_event_cb_ctxt_t *)ctxt;
    if_t                     *hal_if           = NULL;

    hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);
    if (hal_if->if_type == intf::IF_TYPE_UPLINK) {
        if (hal_if->fp_port_num == cb_ctxt->fp_port_num) {
            cb_ctxt->hal_if = hal_if;
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
// Port event to IF status
//------------------------------------------------------------------------------
IfStatus
port_event_to_if_status (port_event_t event)
{
    switch(event) {
    case port_event_t::PORT_EVENT_LINK_UP: return intf::IF_STATUS_UP;
    case port_event_t::PORT_EVENT_LINK_DOWN: return intf::IF_STATUS_DOWN;
    default: return intf::IF_STATUS_NONE;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
hal_ret_t
if_port_oper_state_process_event (uint32_t fp_port_num, port_event_t event)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_port_event_cb_ctxt_t ctxt = {0};
    IfStatus                new_status = intf::IF_STATUS_NONE;


    // Take CFG DB write lock
    g_hal_state->cfg_db()->wlock();

    new_status = port_event_to_if_status(event);

    ctxt.fp_port_num = fp_port_num;
    // Find uplink with this port_num
    g_hal_state->if_id_ht()->walk(if_port_event_get_ht_cb, &ctxt);

    if (ctxt.hal_if == NULL) {
        HAL_TRACE_ERR("No uplink for fp_port_num: {}", fp_port_num);
        ret = HAL_RET_ERR;
        goto end;
    }

    HAL_TRACE_DEBUG("Uplink If Id: {}, handle: {}. Status change {} => {}",
                    ctxt.hal_if->if_id, ctxt.hal_if->hal_handle,
                    if_status_to_str(ctxt.hal_if->if_op_status),
                    if_status_to_str(new_status));
    // Update uplink's oper status
    ctxt.hal_if->if_op_status = new_status;

    // have to repin in all modes,
    // for inp_prop cpu entry reprogram to drive right dst_lport
#if 0
    // Only for hostpin, repin l2segs
    if (!is_forwarding_mode_host_pinned()) {
        goto end;
    }
#endif

    // Walk through l2segs for this uplink
    ret = hal_if_repin_l2segs(ctxt.hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to repin l2segs. ret: {}", ret);
        goto end;
    }

    // Walk through mirror sessions for this uplink
    ret = hal_if_repin_mirror_sessions(ctxt.hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to repin mirror sessions. ret: {}", ret);
        goto end;
    }

end:
    // Release write lock
    g_hal_state->cfg_db()->wunlock();
    
    // Deprecated as fwding info is not going to come from flow anymore
#if 0
    // Repin ipfix flows only if you are in host pin mode
    if (ctxt.hal_if && is_forwarding_mode_host_pinned()) {
        // Repin ipfix flows
        ret = hal_if_repin_ipfix_flows(ctxt.hal_if);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to repin ipfix sessions. ret: {}", ret);
        }
    }
#endif

    return ret;
}

//-----------------------------------------------------------------------------
// Re pin IPFIX flows
//-----------------------------------------------------------------------------
hal_ret_t
hal_if_repin_ipfix_flows (if_t *uplink)
{
    hal_ret_t       ret = HAL_RET_OK;
    
    if (!uplink) {
        // Nothing to be done
        return ret;
    }
    if (uplink->if_type != intf::IF_TYPE_UPLINK) {
        // Nothing to be done
        return ret;
    }
    if (uplink->is_oob_management) {
        // Nothing to be done
        return ret;
    }
    
    HAL_TRACE_DEBUG("Repin IPFIX flows");
    // We need to post session update to FTE
    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t  *session = (session_t *)entry;
        dllist_ctxt_t   *list_head = (dllist_ctxt_t *) ctxt;

        hal_handle_id_list_entry_t *lentry = (hal_handle_id_list_entry_t *)g_hal_state->
                hal_handle_id_list_entry_slab()->alloc();
        if (lentry == NULL) {
            HAL_TRACE_ERR("Out of memory - skipping update session {}", session->hal_handle);
            return false;
        }
        HAL_TRACE_DEBUG("add the handle {}", session->hal_handle);
        lentry->handle_id = session->hal_handle;
        dllist_add(list_head, &lentry->dllist_ctxt);
        return false;
    };

    // build list of session_ids
    dllist_ctxt_t ipfix_session_list;
    dllist_reset(&ipfix_session_list);

    if (g_hal_state->session_hal_telemetry_ht()->num_entries() > 0) {
        HAL_TRACE_DEBUG("Calling IPFIX sessions walk func");
        g_hal_state->session_hal_telemetry_ht()->walk_safe(walk_func,
                                                        &ipfix_session_list);
    }

    HAL_TRACE_DEBUG("Update the IPFIX sessions");
    uint64_t bitmap =
        (uint64_t)(1 << fte::feature_id("pensando.io/network:fwding"));
    bitmap |= (uint64_t)(1 << fte::feature_id("pensando.io/telemetry:telemetry"));
    session_update_list(&ipfix_session_list, true, bitmap);
    
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Re pin mirror sessions on the uplink
//-----------------------------------------------------------------------------
hal_ret_t
hal_if_repin_mirror_sessions (if_t *uplink)
{
    hal_ret_t       ret = HAL_RET_OK;
    
    ret = telemetry_mirror_session_handle_repin(uplink);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("telemetry_mirror_session_handle_repin failed! ret: {}", ret);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// Re pin l2segs on the uplink
//-----------------------------------------------------------------------------
hal_ret_t
hal_if_repin_l2segs (if_t *uplink)
{
    hal_ret_t       ret = HAL_RET_OK;
    l2seg_t         *l2seg    = NULL;
    hal_handle_t    *p_hdl_id = NULL;

    // walk L2 segs
    for (const void *ptr : *uplink->l2seg_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        l2seg = l2seg_lookup_by_handle(*p_hdl_id);
        // Mgmt L2segs are classic
        if (!l2seg_is_mgmt(l2seg) &&
            (l2seg->pinned_uplink == uplink->hal_handle ||
            l2seg->pinned_uplink == HAL_HANDLE_INVALID)) {
            HAL_TRACE_DEBUG("l2seg: Id: {}. Trigger change of pinned "
                            "uplink: from: {}",
                            l2seg->seg_id, l2seg->pinned_uplink);
            ret = l2seg_handle_repin(l2seg);
        } else {
            HAL_TRACE_DEBUG("l2seg: Id: {}. Not triggering change of "
                            "pinned uplink from: {}",
                            l2seg->seg_id, l2seg->pinned_uplink);
        }
    }
    return ret;
}

//-----------------------------------------------------------------------------
// Adds enics into if list of uplinks
//-----------------------------------------------------------------------------
hal_ret_t
uplink_add_enicif (if_t *uplink, if_t *enic_if)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (uplink == NULL || enic_if== NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(uplink, __FILENAME__, __LINE__, __func__);      // lock
    ret = uplink->enicif_list->insert(&enic_if->hal_handle);
    if_unlock(uplink, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add l2seg {} to uplink {}",
                        enic_if->if_id, uplink->if_id);
        goto end;
    }

end:
    HAL_TRACE_DEBUG("Added enicif {} to uplink {}", enic_if->if_id, uplink->if_id);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove enicif from if list of uplink
//-----------------------------------------------------------------------------
hal_ret_t
uplink_del_enicif (if_t *uplink, if_t *enic_if)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (uplink == NULL || enic_if== NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(uplink, __FILENAME__, __LINE__, __func__);      // lock
    ret = uplink->enicif_list->remove(&enic_if->hal_handle);
    if_unlock(uplink, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add l2seg {} to uplink {}",
                        enic_if->if_id, uplink->if_id);
        goto end;
    }

    HAL_TRACE_DEBUG("Deleted enicif {} to uplink {}",
                    enic_if->if_id, uplink->if_id);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// given an interface, marshall it for persisting its state (spec, status,
// stats)
//
// obj points to interface object i.e., if_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
if_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    InterfaceGetResponse    rsp;
    uint32_t                serialized_state_sz;
    if_t                    *hal_if = (if_t *)obj;

    SDK_ASSERT((hal_if != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this interface
    if_process_get(hal_if, &rsp);
    serialized_state_sz = rsp.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall interface {}, not enough room, "
                      "required size {}, available size {}",
                      hal_if->if_id, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (rsp.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize interface {}", hal_if->if_id);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled interface {}, len {}",
                    hal_if->if_id, serialized_state_sz);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a IF's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
if_init_from_status (if_t *hal_if, const InterfaceStatus& status)
{
    hal_if->hal_handle = status.if_handle();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a IF's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
if_init_from_stats (if_t *hal_if, const InterfaceStats& stats)
{
    // TODO: Not sure if we have to store interface stats
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// interface restore add callback
//-----------------------------------------------------------------------------
hal_ret_t
if_restore_add (if_t *hal_if, const InterfaceGetResponse& if_info)
{
    hal_ret_t                   ret;
    pd::pd_if_restore_args_t    pd_if_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    // restore pd state
    pd_if_args.hal_if = hal_if;
    pd_if_args.if_status = &if_info.status();
    pd_func_args.pd_if_restore = &pd_if_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_RESTORE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore IF {} pd, err : {}",
                      hal_if->if_id, ret);
    }
    return ret;
}

static hal_ret_t
if_restore_commit (if_t *hal_if)
{
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Committing IF {} restore", hal_if->if_id);

    ret = if_add_to_db_and_refs(hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to add to DB or refs: if:{}, err:{}",
                      hal_if->if_id, ret);
        goto end;
    }

end:
    return ret;
}

static hal_ret_t
if_restore_abort (if_t *hal_if, const InterfaceGetResponse& if_info)
{
    HAL_TRACE_ERR("Aborting IF {} restore", hal_if->if_id);
    if_create_abort_cleanup(hal_if, hal_if->hal_handle);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// interface restore callback
//-----------------------------------------------------------------------------
uint32_t
if_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t               ret;
    InterfaceGetResponse    if_info;
    if_t                    *hal_if;
    uint32_t                rc = 0;

    // de-serialize the object
    if (if_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized interface obj");
        goto end;
    }

    // allocate VRF obj from slab
    hal_if = if_alloc_init();
    if (hal_if == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init if, err : {}", ret);
        return 0;
    }

    // initialize vrf attrs from its spec
    ret = if_init_from_spec(hal_if, if_info.spec());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to init IF from spec. err:{}", ret);
        SDK_ASSERT(0);
    }
    ret = if_init_from_status(hal_if, if_info.status());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to init IF from status. err:{}", ret);
        SDK_ASSERT(0);
    }
    ret = if_init_from_stats(hal_if, if_info.stats());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to init IF from stats. err:{}", ret);
        SDK_ASSERT(0);
    }

    // repopulate handle db
    ret = hal_handle_insert(HAL_OBJ_ID_INTERFACE, hal_if->hal_handle,
                      (void *)hal_if);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    ret = if_restore_add(hal_if, if_info);
    if (ret != HAL_RET_OK) {
        if_restore_abort(hal_if, if_info);
    }
    if_restore_commit(hal_if);

end:
    return rc;
}

//-----------------------------------------------------------------------------
// adds acl into if list
//-----------------------------------------------------------------------------
hal_ret_t
if_add_acl (if_t *hal_if, acl_t *acl, if_acl_ref_type_t type)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (hal_if == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->acl_list[type]->insert(&acl->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add acl {} to hal_if {} type {}",
                        acl->key, hal_if->if_id, type);
        goto end;
    }


    HAL_TRACE_DEBUG("Added acl {} to hal_if {} type {}",
                    acl->key, hal_if->if_id, type);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// remove acl from hal_if list
//-----------------------------------------------------------------------------
hal_ret_t
if_del_acl (if_t *hal_if, acl_t *acl, if_acl_ref_type_t type)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (hal_if == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if_lock(hal_if, __FILENAME__, __LINE__, __func__);      // lock
    ret = hal_if->acl_list[type]->remove(&acl->hal_handle);
    if_unlock(hal_if, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove acl {} from from hal_if {} type {}, "
                      "err : {}",
                       acl->key, hal_if->if_id, type, ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted acl {} from hal_if {} type {}",
                    acl->key, hal_if->if_id, type);

end:
    return ret;
}

//------------------------------------------------------------------------------
// if spec's keyhandle to str
//------------------------------------------------------------------------------
const char *
if_spec_keyhandle_to_str (const InterfaceKeyHandle& key_handle)
{
	static thread_local char       if_str[4][50];
	static thread_local uint8_t    if_str_next = 0;
	char                           *buf;

	buf = if_str[if_str_next++ & 0x3];
	memset(buf, 0, 50);

    if (key_handle.key_or_handle_case() ==
            InterfaceKeyHandle::kInterfaceId) {
		snprintf(buf, 50, "if_id: %lu", key_handle.interface_id());
    }
    if (key_handle.key_or_handle_case() ==
            InterfaceKeyHandle::kIfHandle) {
		snprintf(buf, 50, "if_handle: 0x%lx", key_handle.if_handle());
    }

	return buf;
}

//------------------------------------------------------------------------------
// PI vrf to str
//------------------------------------------------------------------------------
const char *
if_keyhandle_to_str (if_t *hal_if)
{
    static thread_local char       if_str[4][50];
    static thread_local uint8_t    if_str_next = 0;
    char                           *buf;

    buf = if_str[if_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (hal_if) {
        snprintf(buf, 50, "hal_if(id: %lu, handle: %lu)",
                 hal_if->if_id, hal_if->hal_handle);
    }
    return buf;
}

const char*
if_status_to_str (IfStatus status)
{
    switch(status) {
        case intf::IF_STATUS_UP: return "IF_STATUS_UP";
        case intf::IF_STATUS_DOWN: return "IF_STATUS_DOWN";
        default: return "IF_STATUS_NONE";
    }
}

}    // namespace hal
