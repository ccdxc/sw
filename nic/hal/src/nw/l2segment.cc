//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/oif_list_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/src/utils/utils.hpp"

namespace hal {

static void l2seg_ep_learning_update(l2seg_t *l2seg, const L2SegmentSpec& spec);
//----------------------------------------------------------------------------
// hash table seg_id => entry
//  - Get key from entry
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
// hash table seg_id => entry - compute hash
//----------------------------------------------------------------------------
uint32_t
l2seg_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(l2seg_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// hash table seg_id => entry - compare function
//----------------------------------------------------------------------------
bool
l2seg_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(l2seg_id_t *)key1 == *(l2seg_id_t *)key2) {
        return true;
    }
    return false;
}

// allocate a l2segment instance
static inline l2seg_t *
l2seg_alloc (void)
{
    l2seg_t    *l2seg;

    l2seg = (l2seg_t *)g_hal_state->l2seg_slab()->alloc();
    if (l2seg == NULL) {
        return NULL;
    }
    return l2seg;
}

// initialize a l2segment instance
static inline l2seg_t *
l2seg_init (l2seg_t *l2seg)
{
    if (!l2seg) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&l2seg->slock, PTHREAD_PROCESS_SHARED);
    l2seg->vrf_handle       = 0;
    l2seg->seg_id           = 0;
    l2seg->segment_type     = types::L2_SEGMENT_TYPE_NONE;
    l2seg->mcast_fwd_policy = l2segment::MULTICAST_FWD_POLICY_NONE;
    l2seg->bcast_fwd_policy = l2segment::BROADCAST_FWD_POLICY_NONE;

    // initialize the operational state
    l2seg->num_ep = 0;
    l2seg->pd     = NULL;

    // initialize meta information
    l2seg->if_list = block_list::factory(sizeof(hal_handle_t),
                                         BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK,
                                         hal_mmgr());
    l2seg->mbrif_list = block_list::factory(sizeof(hal_handle_t),
                                            BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK,
                                            hal_mmgr());
    l2seg->nw_list = block_list::factory(sizeof(hal_handle_t),
                                         BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK,
                                         hal_mmgr());
    l2seg->eplearn_cfg.dhcp_cfg.trusted_servers_list =
        block_list::factory(sizeof(ip_addr_t),
                            BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK,
                            hal_mmgr());
    l2seg->acl_list = block_list::factory(sizeof(hal_handle_t),
                                          BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK,
                                          hal_mmgr());
    return l2seg;
}

// allocate and initialize a l2segment instance
static inline l2seg_t *
l2seg_alloc_init (void)
{
    return l2seg_init(l2seg_alloc());
}

// anti l2seg_alloc
static inline hal_ret_t
l2seg_free (l2seg_t *l2seg)
{
    HAL_SPINLOCK_DESTROY(&l2seg->slock);
    hal::delay_delete_to_slab(HAL_SLAB_L2SEG, l2seg);
    return HAL_RET_OK;
}

l2seg_t *
find_l2seg_by_id (l2seg_id_t l2seg_id)
{
    hal_handle_id_ht_entry_t    *entry;
    l2seg_t                     *l2seg;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        l2seg_id_ht()->lookup(&l2seg_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_L2SEG);
        l2seg = (l2seg_t *)hal_handle_get_obj(entry->handle_id);
        return l2seg;
    }
    return NULL;
}

l2seg_t *
l2seg_lookup_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle : {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_L2SEG) {
        HAL_TRACE_DEBUG("Failed to find l2seg with handle : {}", handle);
        return NULL;
    }
   return (l2seg_t *)hal_handle_get_obj(handle);
}

// anti l2seg_alloc_init
static inline hal_ret_t
l2seg_cleanup (l2seg_t *l2seg)
{
    if (l2seg->nw_list) {
        hal_cleanup_handle_block_list(&l2seg->nw_list);
    }
    if (l2seg->if_list) {
        hal_cleanup_handle_block_list(&l2seg->if_list);
    }
    if (l2seg->mbrif_list) {
        hal_cleanup_handle_block_list(&l2seg->mbrif_list);
    }
    if (l2seg->acl_list) {
        hal_cleanup_handle_block_list(&l2seg->acl_list);
    }

    if (l2seg->eplearn_cfg.dhcp_cfg.trusted_servers_list) {
        hal_cleanup_handle_block_list(&l2seg->eplearn_cfg.dhcp_cfg.trusted_servers_list);
    }
    l2seg_free(l2seg);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert a l2segment to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
l2seg_add_to_db (l2seg_t *l2seg, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding l2seg {} to db", l2seg->seg_id);

    // allocate an entry to establish mapping from seg id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from seg id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->l2seg_id_ht()->insert_with_key(&l2seg->seg_id,
                                                          entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add l2seg {} to handle mapping, err : {}",
                      l2seg->seg_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return ret;
}

//------------------------------------------------------------------------------
// delete a l2seg from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
l2seg_del_from_db (l2seg_t *l2seg)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Deleting l2seg {} from db", l2seg->seg_id);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->l2seg_id_ht()->
        remove(&l2seg->seg_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

static inline hal_ret_t
l2seg_create_oifs (l2seg_t *l2seg)
{
    hal_ret_t  ret = HAL_RET_OK;

    // create the broadcast/flood list for this l2seg
    if (is_forwarding_mode_classic_nic()) {
        ret = oif_list_create_block(&l2seg->base_oif_list_id, 3);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to create broadcast list[3], err : {}", ret);
            goto end;
        }
        ret = oif_list_set_honor_ingress(l2seg_get_bcast_oif_list(l2seg));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to set honor-ingr for bcast, err : {}", ret);
            oif_list_delete_block(l2seg->base_oif_list_id, 3);
            l2seg->base_oif_list_id = OIF_LIST_ID_INVALID;
            goto end;
        }
        ret = oif_list_set_honor_ingress(l2seg_get_mcast_oif_list(l2seg));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to set honor-ingr for mcast, err : {}", ret);
            oif_list_clr_honor_ingress(l2seg_get_bcast_oif_list(l2seg));
            oif_list_delete_block(l2seg->base_oif_list_id, 3);
            l2seg->base_oif_list_id = OIF_LIST_ID_INVALID;
            goto end;
        }
        ret = oif_list_set_honor_ingress(l2seg_get_prmsc_oif_list(l2seg));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to set honor-ingr for promisc, err : {}", ret);
            oif_list_clr_honor_ingress(l2seg_get_bcast_oif_list(l2seg));
            oif_list_clr_honor_ingress(l2seg_get_mcast_oif_list(l2seg));
            oif_list_delete_block(l2seg->base_oif_list_id, 3);
            l2seg->base_oif_list_id = OIF_LIST_ID_INVALID;
            goto end;
        }
    } else {
        ret = oif_list_create(&l2seg->base_oif_list_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to create broadcast list, err : {}", ret);
            goto end;
        }

        if (is_forwarding_mode_host_pinned()) {
            ret = oif_list_set_honor_ingress(l2seg_get_bcast_oif_list(l2seg));
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to set honor-ingr for bcast, err : {}", ret);
                oif_list_delete(l2seg->base_oif_list_id);
                l2seg->base_oif_list_id = OIF_LIST_ID_INVALID;
                goto end;
            }
        }
    }

end:

    return ret;
}

static inline hal_ret_t
l2seg_delete_oifs (l2seg_t *l2seg)
{
    // create the broadcast/flood list for this l2seg
    if (is_forwarding_mode_classic_nic()) {
        oif_list_clr_honor_ingress(l2seg_get_bcast_oif_list(l2seg));
        oif_list_clr_honor_ingress(l2seg_get_mcast_oif_list(l2seg));
        oif_list_clr_honor_ingress(l2seg_get_prmsc_oif_list(l2seg));
        oif_list_delete_block(l2seg->base_oif_list_id, 3);
    } else {
        if (is_forwarding_mode_host_pinned()) {
            oif_list_clr_honor_ingress(l2seg_get_bcast_oif_list(l2seg));
        }
        oif_list_delete(l2seg->base_oif_list_id);
    }

    l2seg->base_oif_list_id = OIF_LIST_ID_INVALID;
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dump l2segment spec
//-----------------------------------------------------------------------------
static inline void
l2segment_dump (L2SegmentSpec& spec)
{
    std::string    l2seg_cfg;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &l2seg_cfg);
    HAL_TRACE_DEBUG("L2 segment configuration:");
    HAL_TRACE_DEBUG("{}", l2seg_cfg.c_str());
}

//------------------------------------------------------------------------------
// validate an incoming L2 segment create request
// TODO:
// 1. check if L2 segment exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_l2segment_create (L2SegmentSpec& spec, L2SegmentResponse *rsp,
                           l2seg_create_app_ctxt_t &app_ctxt)
{
    vrf_t   *vrf = NULL;

    if (!spec.has_vrf_key_handle() ||
        spec.vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("No vrf_key_handle or invalid vrf id");
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
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
        HAL_TRACE_ERR("L2seg id and handle not set in request");
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            L2SegmentKeyHandle::kSegmentId) {
        HAL_TRACE_ERR("L2seg id not set in request");
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have wire encap
    if (!spec.has_wire_encap()) {
        HAL_TRACE_ERR("L2seg id doesn't have wire encap");
        rsp->set_api_status(types::API_STATUS_ENCAP_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    switch (spec.wire_encap().encap_type()) {
    case types::ENCAP_TYPE_NONE:
    case types::ENCAP_TYPE_DOT1Q:
    case types::ENCAP_TYPE_VXLAN:
        break;
    default:
        HAL_TRACE_ERR("L2seg id wire encap type {} not allowed", spec.wire_encap().encap_type());
        rsp->set_api_status(types::API_STATUS_ENCAP_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.has_tunnel_encap()) {
        switch (spec.tunnel_encap().encap_type()) {
        case types::ENCAP_TYPE_NONE:
        case types::ENCAP_TYPE_VXLAN:
            break;
        default:
            HAL_TRACE_ERR("L2seg id tunnel encap type {} not allowed", spec.tunnel_encap().encap_type());
            rsp->set_api_status(types::API_STATUS_ENCAP_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    }

    // fetch the vrf
    vrf = vrf_lookup_key_or_handle(spec.vrf_key_handle());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Vrf {}/{} not found", spec.vrf_key_handle().vrf_id(),
                      spec.vrf_key_handle().vrf_handle());
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_VRF_NOT_FOUND;
    }
    app_ctxt.vrf = vrf;

    if ((vrf->vrf_type == types::VRF_TYPE_INFRA) &&
        vrf->l2seg_list->num_elems()) {
        HAL_TRACE_ERR("Infra l2seg exists, only one infra l2segment allowed");
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// pd call to allocate pd resources and h/w programming
//------------------------------------------------------------------------------
hal_ret_t
l2seg_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_l2seg_create_args_t  pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                     *l2seg = NULL;
    l2seg_create_app_ctxt_t     *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt  = (l2seg_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    l2seg     = (l2seg_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add cb for l2seg_id : {}", l2seg->seg_id);

    ret = l2seg_create_oifs(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create OIFs for l2seg, err : {}", ret);
        goto end;
    }

    // PD Call to allocate PD resources and HW programming
    pd::pd_l2seg_create_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    pd_l2seg_args.vrf = app_ctxt->vrf;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_CREATE, (void *)&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create l2seg pd, err : {}", ret);
    }

end:

    return ret;
}

//----------------------------------------------------------------------------
// Add/Del relation nw -> l2seg for all networks in the list
//----------------------------------------------------------------------------
hal_ret_t
l2seg_update_network_relation (block_list *nw_list, l2seg_t *l2seg, bool add)
{
    hal_ret_t       ret = HAL_RET_OK;
    network_t       *nw = NULL;
    hal_handle_t    *p_hdl_id = NULL;

    for (const void *ptr : *nw_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        nw = find_network_by_handle(*p_hdl_id);
        if (!nw) {
            HAL_TRACE_ERR("Unable to find network with handle : {}",
                          *p_hdl_id);
            ret = HAL_RET_NETWORK_NOT_FOUND;
            goto end;
        }
        if (add) {
            ret = network_add_l2seg(nw, l2seg);
        } else {
            ret = network_del_l2seg(nw, l2seg);
        }
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// Add/Del relation if -> l2seg for all IFs in the list
//----------------------------------------------------------------------------
hal_ret_t
l2seg_update_if_relation (block_list *if_list, l2seg_t *l2seg, bool add)
{
    hal_ret_t       ret = HAL_RET_OK;
    if_t            *hal_if = NULL;
    hal_handle_t    *p_hdl_id = NULL;

    if (!if_list) goto end;

    for (const void *ptr : *if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl_id);
        if (!hal_if) {
            HAL_TRACE_ERR("Unable to find IF with handle : {}",
                          *p_hdl_id);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }
        if (add) {
            ret = if_add_l2seg(hal_if, l2seg);
        } else {
            ret = if_del_l2seg(hal_if, l2seg);
        }
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// Update Mcast oiflist of L2seg
//----------------------------------------------------------------------------
hal_ret_t
l2seg_update_oiflist (block_list *if_list, l2seg_t *l2seg, bool add)
{
    hal_ret_t       ret = HAL_RET_OK;
    if_t            *hal_if = NULL;
    hal_handle_t    *p_hdl_id = NULL;
    oif_t           oif;

    if (!if_list) goto end;

    for (const void *ptr : *if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl_id);
        if (!hal_if) {
            HAL_TRACE_ERR("Unable to find IF with handle : {}",
                          *p_hdl_id);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }

        oif.intf = hal_if;
        oif.l2seg = l2seg;

        if (add) {
            ret = oif_list_add_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Add IF to bcast oiflist Failed. ret : {}",
                              ret);
                goto end;
            }
        } else {
            ret = oif_list_remove_oif(l2seg_get_bcast_oif_list(l2seg), &oif);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Del IF to bcast oiflist"
                              " Failed. ret : {}", ret);
                goto end;
            }
        }
    }

end:
    return ret;
}

hal_ret_t
l2seg_add_to_db_and_refs (l2seg_t *l2seg, hal_handle_t hal_handle,
                          vrf_t *vrf)
{
    hal_ret_t ret        = HAL_RET_OK;

    // Add to l2seg id hash table
    ret = l2seg_add_to_db(l2seg, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add l2seg {} to db, err : {}",
                      l2seg->seg_id, ret);
        goto end;
    }

    // Add l2seg to vrf's l2seg list
    ret = vrf_add_l2seg(vrf, l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add rel. from vrf");
        goto end;
    }

    // Add l2seg to network's l2seg list
    ret = l2seg_update_network_relation (l2seg->nw_list, l2seg, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add network -> l2seg "
                      "relation ret : {}", ret);
        goto end;
    }

    // Add l2seg to IF's l2seg list
    ret = l2seg_update_if_relation(l2seg->mbrif_list, l2seg, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add IF -> l2seg "
                      "relation ret : {}", ret);
        goto end;
    }

    if (is_forwarding_mode_smart_nic()) {
        ret = l2seg_update_oiflist(l2seg->if_list, l2seg, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add to form bcast oiflist "
                          "ret : {}", ret);
            goto end;
        }
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
    hal_ret_t               ret        = HAL_RET_OK;
    dllist_ctxt_t           *lnode     = NULL;
    dhl_entry_t             *dhl_entry = NULL;
    l2seg_t                 *l2seg     = NULL;
    hal_handle_t            hal_handle = HAL_HANDLE_INVALID;
    l2seg_create_app_ctxt_t *app_ctxt  = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (l2seg_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    l2seg = (l2seg_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb for l2seg_id : {}",
                    l2seg->seg_id);

    ret = l2seg_add_to_db_and_refs(l2seg, hal_handle, app_ctxt->vrf);

end:
    return ret;
}

static hal_ret_t
l2seg_create_abort_cleanup (l2seg_t *l2seg, hal_handle_t hal_handle)
{
    hal_ret_t                       ret;
    pd::pd_l2seg_delete_args_t      pd_l2seg_args = { 0 };

    // delete call to PD
    if (l2seg->pd) {
        pd::pd_l2seg_delete_args_init(&pd_l2seg_args);
        pd_l2seg_args.l2seg = l2seg;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_DELETE, (void *)&pd_l2seg_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete l2seg pd, err : {}",
                          ret);
        }
    }

    // remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // Free l2seg. This will also cleanup nws if there are any
    // l2seg_cleanup(l2seg);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// l2seg_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
l2seg_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                    *l2seg = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create abort cb {}",
                    l2seg->seg_id);

    ret = l2seg_create_abort_cleanup(l2seg, hal_handle);

end:
    return ret;
}

//----------------------------------------------------------------------------
// Dummy create cleanup callback
//----------------------------------------------------------------------------
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
l2seg_prepare_rsp (L2SegmentResponse *rsp, hal_ret_t ret, l2seg_t *l2seg)
{
    pd::pd_l2seg_get_flow_lkupid_args_t args;
    if (ret == HAL_RET_OK) {
        // No error, hance l2seg is valid
#if 0
        rsp->mutable_l2segment_status()->set_l2segment_handle(l2seg ?
                                                              l2seg->hal_handle :
                                                              HAL_HANDLE_INVALID);
#endif
        rsp->mutable_l2segment_status()->set_l2segment_handle(l2seg->hal_handle);
        // TODO: REMOVE DOL test only
        args.l2seg = l2seg;
        pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, (void *)&args);
        rsp->mutable_l2segment_status()->set_vrf_id(args.hwid);

    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Reads networks from spec
//------------------------------------------------------------------------------
hal_ret_t
l2seg_read_networks (l2seg_t *l2seg, const L2SegmentSpec& spec)
{
    hal_ret_t               ret = HAL_RET_OK;
    uint32_t                num_nws = 0, i = 0;
    network_t               *nw = NULL;
    NetworkKeyHandle        nw_key_handle;

    num_nws = spec.network_key_handle_size();

    HAL_TRACE_DEBUG("received {} networks", num_nws);

    for (i = 0; i < num_nws; i++) {
        nw_key_handle = spec.network_key_handle(i);
        nw = network_lookup_key_or_handle(nw_key_handle);
        if (nw == NULL) {
            ret = HAL_RET_NETWORK_NOT_FOUND;
            goto end;
        }
        HAL_TRACE_DEBUG("adding network: {} with handle : {}",
                        ippfx2str(&nw->nw_key.ip_pfx), nw->hal_handle);

        // add nw to list
        // hal_add_to_handle_list(&l2seg->nw_list_head, nw->hal_handle);
        hal_add_to_handle_block_list(l2seg->nw_list, nw->hal_handle);
    }
    HAL_TRACE_DEBUG("networks added:");
    hal_print_handles_block_list(l2seg->nw_list);
end:
    return ret;
}

uint32_t
l2seg_if_list_size (const void *spec)
{
    L2SegmentSpec *l2seg_spec = (L2SegmentSpec *)spec;
    return l2seg_spec->if_key_handle_size();
}

hal_handle_t
l2seg_if_list_get_hdl (const void *spec, uint32_t idx)
{
    L2SegmentSpec *l2seg_spec = (L2SegmentSpec *)spec;
    if_t *hal_if = NULL;

    auto if_key_hdl = l2seg_spec->if_key_handle(idx);
    hal_if = if_lookup_key_or_handle(if_key_hdl);
    return hal_if ? hal_if->hal_handle : 0;
}


hal_ret_t
l2seg_read_ifs (l2seg_t *l2seg, const L2SegmentSpec& spec)
{
    hal_ret_t       ret         = HAL_RET_OK;
    // hal_handle_t    *p_hdl      = NULL;
    // if_t            *hal_if     = NULL;
    bool            has_changed = false;

    ret = hal_find_changed_lists(l2seg->mbrif_list,
                                 &spec,
                                 l2seg_if_list_size,
                                 l2seg_if_list_get_hdl,
                                 NULL,
                                 NULL,
                                 &l2seg->mbrif_list,
                                 &has_changed);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to find changed IF list. err:{}", ret);
        if (ret == HAL_RET_ENTRY_NOT_FOUND) ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("New IFs: ");
    hal_print_handles_block_list(l2seg->mbrif_list);
#if 0
    for (const void *ptr : *l2seg->mbrif_list) {
        p_hdl = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl);
        if (!hal_if) {
            HAL_TRACE_ERR("Failed to find IF(hdl): {}", *p_hdl);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }
    }
#endif

end:
    return ret;
}

static bool
is_l2seg_same (l2seg_t *l2seg, L2SegmentSpec& spec, l2seg_create_app_ctxt_t &app_ctxt)
{
    bool             ret            = false;
    uint32_t         num_nws        = 0;
    uint32_t         i              = 0;
    NetworkKeyHandle nw_key_handle;
    ip_addr_t        gipo;

    // All L2Segment's should be associated with a valid VRF object.
    if (spec.vrf_key_handle().vrf_id() != app_ctxt.vrf->vrf_id) {
        return ret;
    }

    if (spec.segment_type() != l2seg->segment_type) {
        return ret;
    }

    if (spec.mcast_fwd_policy() != l2seg->mcast_fwd_policy) {
        return ret;
    }

    if (spec.bcast_fwd_policy() != l2seg->bcast_fwd_policy) {
        return ret;
    }

    if (spec.has_wire_encap()) {
        if ((spec.wire_encap().encap_type() != l2seg->wire_encap.type) ||
            (spec.wire_encap().encap_value() != l2seg->wire_encap.val)) {
            return ret;
        }
    } else {
        if (l2seg->wire_encap.type || l2seg->wire_encap.val) {
            return ret;
        }
    }

    if (spec.has_tunnel_encap()) {
        if ((spec.tunnel_encap().encap_type() != l2seg->tunnel_encap.type) ||
            (spec.tunnel_encap().encap_value() != l2seg->tunnel_encap.val)) {
            return ret;
        }
    } else {
        if (l2seg->tunnel_encap.type || l2seg->tunnel_encap.val) {
            return ret;
        }
    }

    if (spec.has_gipo()) {
        hal_ret_t ip_ret = ip_addr_spec_to_ip_addr(&gipo, spec.gipo());

        if (ip_ret != HAL_RET_OK) {
            return ret;
        }

        if (!ip_addr_check_equal(&gipo, &l2seg->gipo)) {
            return ret;
        }
    } else {
        memset(&gipo, 0, sizeof(gipo));
        if (memcmp(&gipo, &l2seg->gipo, sizeof(gipo))) {
            return ret;
        }
    }

    if (spec.pinned_uplink_if_handle() != l2seg->pinned_uplink) {
        return ret;
    }

    num_nws = spec.network_key_handle_size();

    for (i = 0; i < num_nws; i++) {
        nw_key_handle = spec.network_key_handle(i);
        auto nw = network_lookup_key_or_handle(nw_key_handle);
        if (nw == NULL) {
            return false;
        }

        if (!hal_handle_in_block_list(l2seg->nw_list, nw->hal_handle)) {
            return false;
        }
    }

    // If we've reached this point, it means that the L2Segment spec is the same
    // as the one that exists. Return the existing handle in this case.
    return true;
}

static hal_ret_t
l2seg_init_from_spec(l2seg_t *l2seg, const L2SegmentSpec& spec)
{
    vrf_t           *vrf;
    hal_ret_t       ret;
    // fetch the vrf
    vrf = vrf_lookup_key_or_handle(spec.vrf_key_handle());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Vrf {}/{} not found", spec.vrf_key_handle().vrf_id(),
                      spec.vrf_key_handle().vrf_handle());
        return HAL_RET_VRF_NOT_FOUND;
    }

    l2seg->vrf_handle       = vrf->hal_handle;
    l2seg->seg_id           = spec.key_or_handle().segment_id();
    l2seg->segment_type     = spec.segment_type();
    l2seg->pinned_uplink    = spec.pinned_uplink_if_handle();
    l2seg->mcast_fwd_policy = spec.mcast_fwd_policy();
    l2seg->bcast_fwd_policy = spec.bcast_fwd_policy();
    ip_addr_spec_to_ip_addr(&l2seg->gipo, spec.gipo());
    if (spec.has_wire_encap()) {
        l2seg->wire_encap.type = spec.wire_encap().encap_type();
        l2seg->wire_encap.val = spec.wire_encap().encap_value();
        HAL_TRACE_DEBUG("Wire enc_type : {} enc_val : {}",
                        l2seg->wire_encap.type, l2seg->wire_encap.val);
    }
    if (spec.has_tunnel_encap()) {
        l2seg->tunnel_encap.type = spec.tunnel_encap().encap_type();
        l2seg->tunnel_encap.val = spec.tunnel_encap().encap_value();
        HAL_TRACE_DEBUG("Tunnel enc_type : {} enc_val : {}",
                        l2seg->tunnel_encap.type, l2seg->tunnel_encap.val);
    }

    ret = l2seg_read_networks(l2seg, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in reading networks, err {}", ret);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = l2seg_read_ifs(l2seg, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in reading IFs, err {}", ret);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }


    l2seg_ep_learning_update(l2seg, spec);
    l2seg->proxy_arp_enabled = spec.proxy_arp_enabled();

end:

    return ret;
}

static hal_ret_t
l2seg_init_from_status(l2seg_t *l2seg, const L2SegmentStatus& status)
{
    l2seg->hal_handle = status.l2segment_handle();
    return HAL_RET_OK;
}

static hal_ret_t
l2seg_init_from_stats(l2seg_t *l2seg, const L2SegmentStats& stats)
{
    l2seg->num_ep = stats.num_endpoints();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a L2 segment create request
// TODO: if L2 segment exists, treat this as modify (vrf id in the vrf_key_handle must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
l2segment_create (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t                   ret;
    l2seg_t                     *l2seg    = NULL;
    l2seg_create_app_ctxt_t     app_ctxt  = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt  = { 0 };
    l2seg_t                     *existing_l2seg = NULL;
    bool                        is_same   = false;

    HAL_TRACE_DEBUG("L2seg create of : {}",
                    spec.key_or_handle().segment_id());
    l2segment_dump(spec);

    // validate the request message
    ret = validate_l2segment_create(spec, rsp, app_ctxt);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("Validation Failed. ret: {}", ret);
        goto end;
    }

    // check if l2segment exists already, and reject if one is found
    existing_l2seg = l2seg_lookup_key_or_handle(spec.key_or_handle());
    if (existing_l2seg) {
        // Check if the 2 specs are the same. If so, then return the old handle
        is_same = is_l2seg_same(existing_l2seg, spec, app_ctxt);
        if (is_same) {
            l2seg = existing_l2seg;
            ret = HAL_RET_OK;
            goto end;
        }

        HAL_TRACE_ERR("Failed to create a l2seg, l2seg {} exists already",
                      spec.key_or_handle().segment_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate the L2 segment
    l2seg = l2seg_alloc_init();
    if (l2seg == NULL) {
        HAL_TRACE_ERR("Unable to allocate handle/memory ret: {}", ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // Init l2seg from spec
    ret = l2seg_init_from_spec(l2seg, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to init l2seg, ret: {}", ret);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        goto end;
    }

    // allocate hal handle id
    l2seg->hal_handle = hal_handle_alloc(HAL_OBJ_ID_L2SEG);
    if (l2seg->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle {}", l2seg->seg_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        l2seg_cleanup(l2seg);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = l2seg->hal_handle;
    dhl_entry.obj = l2seg;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(l2seg->hal_handle, &cfg_ctxt,
                             l2seg_create_add_cb,
                             l2seg_create_commit_cb,
                             l2seg_create_abort_cb,
                             l2seg_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (l2seg) {
            // free l2seg
            l2seg_cleanup(l2seg);
            l2seg = NULL;
        }
        HAL_API_STATS_INC(HAL_API_L2SEGMENT_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_L2SEGMENT_CREATE_SUCCESS);
    }
    l2seg_prepare_rsp(rsp, ret, l2seg);
    return ret;
}

//------------------------------------------------------------------------------
// validate l2seg update request
//------------------------------------------------------------------------------
hal_ret_t
validate_l2seg_update (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // if vrf is set, it has to be right
    if (spec.has_vrf_key_handle() &&
        spec.vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("no vrf_key_handle or invalid vrf id");
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
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
        HAL_TRACE_DEBUG("mcast_fwd_policy updated");
        app_ctxt->mcast_fwd_policy_change = true;
        app_ctxt->new_mcast_fwd_policy = spec.mcast_fwd_policy();
    }
    if (l2seg->bcast_fwd_policy != spec.bcast_fwd_policy()) {
        HAL_TRACE_DEBUG("bcast_fwd_policy updated");
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
    pd::pd_l2seg_update_args_t  pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    l2seg_t                    *l2seg = NULL, *l2seg_clone = NULL;
    l2seg_update_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (l2seg_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    l2seg = (l2seg_t *)dhl_entry->obj;
    l2seg_clone = (l2seg_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb {}",
                    l2seg->seg_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_l2seg_update_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg_clone;
    pd_l2seg_args.iflist_change = app_ctxt->iflist_change;
    pd_l2seg_args.add_iflist = app_ctxt->add_iflist;
    pd_l2seg_args.del_iflist = app_ctxt->del_iflist;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_UPDATE, (void *)&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update l2seg pd, err : {}", ret);
    }

    // TODO: Call the cb for mcast and bcast fwd policy change
    if (app_ctxt->mcast_fwd_policy_change) {
    }
    if (app_ctxt->bcast_fwd_policy_change) {
    }
    if (app_ctxt->nwlist_change) {
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
//  Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
l2seg_make_clone (l2seg_t *l2seg, l2seg_t **l2seg_clone)
{
    pd::pd_l2seg_make_clone_args_t args;

    *l2seg_clone = l2seg_alloc_init();
    memcpy(*l2seg_clone, l2seg, sizeof(l2seg_t));

    args.l2seg = l2seg;
    args.clone = *l2seg_clone;
    pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_MAKE_CLONE, (void *)&args);

    // Lists are copied into clone.
    //  - Update Success:
    //      - Destroy Add and Delete lists in app ctxt
    //      - Destroy clone's list which is original's list as well
    //      - Assign aggr list to clone's list
    //      - Free original
    //  - Update Failure:
    //      - Free clone

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
//  - Update Success:
//      - Destroy Add and Delete lists in app ctxt
//      - Destroy clone's list which is original's list as well
//      - Assign aggr list to clone's list
//      - Free original
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_update_pi_with_new_lists (l2seg_t *l2seg, l2seg_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;

    // lock l2seg
    // Revisit: this is a clone and may be we dont have to take the lock
    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);

    // Destroy clone's list which is original's list as well
    hal_cleanup_handle_block_list(&l2seg->nw_list);
    hal_cleanup_handle_block_list(&l2seg->mbrif_list);
    // Assign aggr list to clone's list
    l2seg->nw_list = app_ctxt->aggr_nwlist;
    l2seg->mbrif_list = app_ctxt->agg_iflist;

    // add/del relations from nws.
    ret = l2seg_update_network_relation(app_ctxt->add_nwlist, l2seg, true);
    ret = l2seg_update_network_relation(app_ctxt->del_nwlist, l2seg, false);

    // add/del relations from IFs.
    ret = l2seg_update_if_relation(app_ctxt->add_iflist, l2seg, true);
    ret = l2seg_update_if_relation(app_ctxt->del_iflist, l2seg, false);

    // Free add & del list
    hal_cleanup_handle_block_list(&app_ctxt->add_nwlist);
    hal_cleanup_handle_block_list(&app_ctxt->del_nwlist);
    hal_cleanup_handle_block_list(&app_ctxt->add_iflist);
    hal_cleanup_handle_block_list(&app_ctxt->del_iflist);

    // Unlock if
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);

    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD l2seg.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
l2seg_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_l2seg_mem_free_args_t    pd_l2seg_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    l2seg_update_app_ctxt_t         *app_ctxt = NULL;
    l2seg_t                         *l2seg = NULL, *l2seg_clone = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (l2seg_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    l2seg = (l2seg_t *)dhl_entry->obj;
    l2seg_clone = (l2seg_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb for l2seg_id : {}", l2seg->seg_id);

#if 0
    // move lists to clone
    dllist_move(&l2seg_clone->nw_list_head, &l2seg->nw_list_head);
    dllist_move(&l2seg_clone->if_list_head, &l2seg->if_list_head);

    // update clone with new nw lists
    if (app_ctxt->nwlist_change) {
        ret = l2seg_update_pi_with_nw_list(l2seg_clone, app_ctxt);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to update pi with nwlists, "
                          "ret : {}",
                          ret);
            goto end;
        }
    }
#endif

    // Bcast oiflist update
    ret = l2seg_update_oiflist(app_ctxt->add_iflist, l2seg_clone, true);
    ret = l2seg_update_oiflist(app_ctxt->del_iflist, l2seg_clone, false);

    //  - Update Success:
    //      - Destroy Add and Delete lists in app ctxt
    //      - Destroy clone's list which is original's list as well
    //      - Assign aggr list to clone's list
    //      - Free original
    ret = l2seg_update_pi_with_new_lists(l2seg_clone, app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update pi with nwlists, ret : {}", ret);
        goto end;
    }

    // Free PD
    pd::pd_l2seg_mem_free_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_MEM_FREE, (void *)&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete l2seg pd, err : {}", ret);
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
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_l2seg_mem_free_args_t    pd_l2seg_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    l2seg_update_app_ctxt_t         *app_ctxt = NULL;
    l2seg_t                         *l2seg = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (l2seg_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    // assign clone as we are trying to free only the clone
    l2seg = (l2seg_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}",
                    l2seg->seg_id);

    // Free PD
    pd::pd_l2seg_mem_free_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_MEM_FREE, (void *)&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete l2seg pd, err : {}",
                      ret);
    }

    // Free nw lists
    hal_cleanup_handle_block_list(&app_ctxt->add_nwlist);
    hal_cleanup_handle_block_list(&app_ctxt->del_nwlist);
    hal_cleanup_handle_block_list(&app_ctxt->aggr_nwlist);

    // Free Clone
    l2seg_free(l2seg);
end:

    return ret;
}

hal_ret_t
l2seg_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
l2seg_nw_list_update (L2SegmentSpec& spec, l2seg_t *l2seg,
                      bool *nwlist_change, block_list *add_nwlist,
                      block_list *del_nwlist, block_list *aggr_nwlist)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint16_t            num_nws = 0, i = 0;
    bool                nw_exists = false;
    NetworkKeyHandle    nw_key_handle;
    network_t           *nw = NULL;
    hal_handle_t        *p_hdl_id = NULL;

    *nwlist_change = false;
    num_nws = spec.network_key_handle_size();
    HAL_TRACE_DEBUG("num. of nws : {}", num_nws);
    for (i = 0; i < num_nws; i++) {
        nw_key_handle = spec.network_key_handle(i);
        nw = network_lookup_key_or_handle(nw_key_handle);
        if (nw == NULL ) {
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        // Add to aggregated list
        hal_add_to_handle_block_list(aggr_nwlist, nw->hal_handle);

        if (hal_handle_in_block_list(l2seg->nw_list, nw->hal_handle)) {
            continue;
        } else {
            // Add to added list
            hal_add_to_handle_block_list(add_nwlist, nw->hal_handle);
            *nwlist_change = true;
            HAL_TRACE_DEBUG("added to add list hdl: {}", nw->hal_handle);
        }
    }

    HAL_TRACE_DEBUG("Existing nws:");
    hal_print_handles_block_list(l2seg->nw_list);
    HAL_TRACE_DEBUG("New Aggregated nws:");
    hal_print_handles_block_list(aggr_nwlist);
    HAL_TRACE_DEBUG("added nws:");
    hal_print_handles_block_list(add_nwlist);

    // dllist_for_each(lnode, &(l2seg->nw_list_head)) {
    //     entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
    for (const void *ptr : *l2seg->nw_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("Checking for nw: {}",
                        *p_hdl_id);
        for (i = 0; i < num_nws; i++) {
            nw_key_handle = spec.network_key_handle(i);
            HAL_TRACE_DEBUG("grpc nw handle: {}", nw->hal_handle);
            if (*p_hdl_id == nw->hal_handle) {
                nw_exists = true;
                break;
            } else {
                continue;
            }
        }
        if (!nw_exists) {
#if 0
            // Have to delet the nw
            lentry = (hal_handle_id_list_entry_t *)g_hal_state->
                hal_handle_id_list_entry_slab()->alloc();
            if (lentry == NULL) {
                ret = HAL_RET_OOM;
                goto end;
            }
            lentry->handle_id = *p_hdl_id;

            // Insert into the list
            sdk::lib::dllist_add(*del_nwlist, &lentry->dllist_ctxt);
#endif
            hal_add_to_handle_block_list(del_nwlist, *p_hdl_id);
            *nwlist_change = true;
            HAL_TRACE_DEBUG("added to delete list hdl: {}",
                            *p_hdl_id);
        }
        nw_exists = false;
    }

    HAL_TRACE_DEBUG("deleted nws:");
    hal_print_handles_block_list(del_nwlist);

#if 0
    if (!*nwlist_change) {
        // Got same nws as existing
        hal_cleanup_handle_list(add_nwlist);
        hal_cleanup_handle_list(del_nwlist);
        hal_cleanup_handle_list(aggr_nwlist);
    }
#endif
end:

    return ret;
}

//------------------------------------------------------------------------------
// check if there are any changes in update
//------------------------------------------------------------------------------
hal_ret_t
l2seg_check_update (L2SegmentSpec& spec, l2seg_t *l2seg,
                    l2seg_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;

    // check for fwd policy change
    ret = l2seg_fwdpolicy_update(spec, l2seg, app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to check if fwdpolicy is updated.");
        goto end;
    }

    // check for network list change
    ret = l2seg_nw_list_update(spec, l2seg, &app_ctxt->nwlist_change,
                               app_ctxt->add_nwlist,
                               app_ctxt->del_nwlist,
                               app_ctxt->aggr_nwlist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to check nw list change. ret : {}", ret);
        goto end;
    }

    // check if mbr ifs change
    ret = hal_find_changed_lists(l2seg->mbrif_list,
                                 &spec,
                                 l2seg_if_list_size,
                                 l2seg_if_list_get_hdl,
                                 &app_ctxt->add_iflist,
                                 &app_ctxt->del_iflist,
                                 &app_ctxt->agg_iflist,
                                 &app_ctxt->iflist_change);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to check if list change. ret : {}", ret);
        goto end;
    }

    if (app_ctxt->iflist_change) {
        HAL_TRACE_DEBUG("New IFs: ");
        hal_print_handles_block_list(app_ctxt->agg_iflist);
        HAL_TRACE_DEBUG("Added IFs: ");
        hal_print_handles_block_list(app_ctxt->add_iflist);
        HAL_TRACE_DEBUG("Deleted IFs: ");
        hal_print_handles_block_list(app_ctxt->del_iflist);
    } else {
        HAL_TRACE_DEBUG("New IFs: ");
        hal_print_handles_block_list(app_ctxt->agg_iflist);
    }


    if (app_ctxt->mcast_fwd_policy_change ||
        app_ctxt->bcast_fwd_policy_change || app_ctxt->nwlist_change ||
        app_ctxt->iflist_change) {
        app_ctxt->l2seg_change = true;
    }

end:

    return ret;
}


//------------------------------------------------------------------------------
// Match the vrf during create and update/delete
//------------------------------------------------------------------------------
hal_ret_t
l2seg_validate_vrf (vrf_id_t vrf_id, l2seg_t *l2seg)
{
    hal_ret_t   ret  = HAL_RET_OK;
    vrf_t    *ten = NULL;

    if (vrf_id == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("invalid vrf_id : {}", vrf_id);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ten = vrf_lookup_by_id(vrf_id);
    if (ten == NULL) {
        HAL_TRACE_ERR("Unable to find vrf_id : {}", vrf_id);
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    if (ten->hal_handle != l2seg->vrf_handle) {
        HAL_TRACE_ERR("Unable to match cr_ten_hdl : {}, "
                      "up_ten_hdl : {}",
                      l2seg->vrf_handle, ten->hal_handle);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Init update app ctxt
//------------------------------------------------------------------------------
hal_ret_t
l2seg_update_app_ctxt_init(l2seg_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!app_ctxt) {
        HAL_TRACE_ERR("app ctxt is NULL");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    memset(app_ctxt, 0, sizeof(l2seg_update_app_ctxt_t));
    app_ctxt->add_nwlist  = block_list::factory(sizeof(hal_handle_t));
    app_ctxt->del_nwlist  = block_list::factory(sizeof(hal_handle_t));
    app_ctxt->aggr_nwlist = block_list::factory(sizeof(hal_handle_t));

    HAL_TRACE_DEBUG("nw lists: {:#x}, {:#x}, {:#x} ",
                    (uint64_t)app_ctxt->add_nwlist, (uint64_t)app_ctxt->del_nwlist,
                    (uint64_t)app_ctxt->aggr_nwlist);

end:
    return ret;
}


//------------------------------------------------------------------------------
// process a L2 segment update request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_update (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    l2seg_t                     *l2seg = NULL, *l2seg_clone = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const L2SegmentKeyHandle    &kh = spec.key_or_handle();
    l2seg_update_app_ctxt_t     app_ctxt = { 0 };

    HAL_TRACE_DEBUG("L2seg Update of : {}",
                    l2seg_spec_keyhandle_to_str(spec.key_or_handle()));
    l2segment_dump(spec);

    // validate the request message
    ret = validate_l2seg_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("l2seg update validation Failed, ret : {}", ret);
        goto end;
    }

    l2seg = l2seg_lookup_key_or_handle(kh);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("Failed to find l2seg, id {}, handle {}",
                      kh.segment_id(), kh.l2segment_handle());
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }

    if (spec.has_vrf_key_handle()) {
        ret = l2seg_validate_vrf(spec.vrf_key_handle().vrf_id(), l2seg);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("mismatch of vrfs for l2seg, "
                          "id : {}, handle : {}",
                          kh.segment_id(), kh.l2segment_handle());
            goto end;
        }
    }

    HAL_TRACE_DEBUG("update l2seg {}",
                    l2seg->seg_id);

    ret = l2seg_update_app_ctxt_init(&app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("l2seg Unable to init upd app ctxt, "
                      "ret : {}", ret);
        goto end;
    }

    l2seg_ep_learning_update(l2seg, spec);

    ret = l2seg_check_update(spec, l2seg, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("l2seg check update Failed, ret : {}", ret);
        goto end;
    }

    if (!app_ctxt.l2seg_change) {
        HAL_TRACE_ERR("no change in l2seg update: noop");
        // Its a no-op. We can just return HAL_RET_OK
        // ret = HAL_RET_INVALID_OP;
        l2seg_clone = l2seg;
        goto end;
    }

    // TODO: Check if we have to do something as fwd policy changed

    l2seg_make_clone(l2seg, &l2seg_clone);
    // l2seg_make_clone(l2seg, (l2seg_t **)&dhl_entry.cloned_obj);
    dhl_entry.cloned_obj = l2seg_clone;

    // form ctxt and call infra update object
    dhl_entry.handle = l2seg->hal_handle;
    dhl_entry.obj = l2seg;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(l2seg->hal_handle, &cfg_ctxt,
                             l2seg_update_upd_cb,
                             l2seg_update_commit_cb,
                             l2seg_update_abort_cb,
                             l2seg_update_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_L2SEGMENT_UPDATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_L2SEGMENT_UPDATE_FAIL);
    }
    l2seg_prepare_rsp(rsp, ret, (ret == HAL_RET_OK) ? l2seg_clone : l2seg);
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
        l2seg = l2seg_lookup_by_handle(kh.l2segment_handle());
    }

    return l2seg;
}

//------------------------------------------------------------------------------
// validate l2seg delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_l2seg_delete_req (L2SegmentDeleteRequest& req, L2SegmentDeleteResponse* rsp)
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
// validate l2seg delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_l2seg_delete (l2seg_t *l2seg)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of back refernces
    if (l2seg->if_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("ifs still referring:");
        hal_print_handles_block_list(l2seg->if_list);
        goto end;
    }

    if (l2seg->acl_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("acls still referring:");
        hal_print_handles_block_list(l2seg->acl_list);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
l2seg_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret           = HAL_RET_OK;
    pd::pd_l2seg_delete_args_t  pd_l2seg_args = { 0 };
    dllist_ctxt_t               *lnode        = NULL;
    dhl_entry_t                 *dhl_entry    = NULL;
    l2seg_t                     *l2seg        = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del cb {}",
                    l2seg->seg_id);

    // TODO: Check the dependency ref count for the l2seg. whoever is refering
    //       to l2seg should be counted against this ref count.
    //      - Sessions
    //       If its non zero, fail the delete.

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_l2seg_delete_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_DELETE, (void *)&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete l2seg pd, err : {}", ret);
        goto end;
    }

    ret = l2seg_delete_oifs(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete OIFs for l2seg, err : {}", ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// detach from networks
//------------------------------------------------------------------------------
hal_ret_t
l2seg_detach_from_networks (l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = l2seg_update_network_relation(l2seg->nw_list, l2seg, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del network -/-> l2seg"
                      "relation ret : {}",
                      ret);
        goto end;
    }

    // clean up sgs list
    HAL_TRACE_DEBUG("cleaning up network list");
    hal_remove_all_handles_block_list(l2seg->nw_list);

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
    vrf_t                    *vrf = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    l2seg = (l2seg_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb {}",
                    l2seg->seg_id);

    // Remove l2seg references from other objects
    vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
    ret = vrf_del_l2seg(vrf, l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del rel. from vrf");
        goto end;
    }

    // remove back refs from networks and free up list
    ret = l2seg_detach_from_networks(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to detach from networks, "
                      "ret : {}", ret);
        goto end;
    }

    // remove back refs from IFs
    ret = l2seg_update_if_relation (l2seg->mbrif_list, l2seg, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to detach from IFs, "
                      "ret : {}", ret);
        goto end;
    }

    // a. Remove from l2seg id hash table
    ret = l2seg_del_from_db(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del l2seg {} from db, err : {}",
                      l2seg->seg_id, ret);
        goto end;
    }

    // TODO: Bharat There wont be anything called infra l2seg. There will be
    //       only Infra VRF.
#if 0
    // If it is Infra L2seg, remove the reference
    if (l2seg->segment_type == types::L2_SEGMENT_TYPE_INFRA) {
        g_hal_state->set_infra_l2seg(NULL);
    }
#endif

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI l2seg
    l2seg_cleanup(l2seg);

    // TODO: Decrement the ref counts of dependent objects
    //  - vrf

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit cbs can't fail: ret : {}",
                      ret);
        HAL_ASSERT(0);
    }
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
l2segment_delete (L2SegmentDeleteRequest& req, L2SegmentDeleteResponse* rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    l2seg_t                     *l2seg = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const L2SegmentKeyHandle    &kh = req.key_or_handle();

    HAL_TRACE_DEBUG("Received L2seg Delete");

    // validate the request message
    ret = validate_l2seg_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("l2seg delete validation Failed, ret : {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("L2seg Delete Validated");

    l2seg = l2seg_lookup_key_or_handle(kh);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("Failed to find l2seg, id {}, handle {}",
                      kh.segment_id(), kh.l2segment_handle());
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("deleting l2seg {}", l2seg->seg_id);

    // validate if there no objects referring this sec. profile
    ret = validate_l2seg_delete(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("l2seg delete validation Failed, "
                      "ret : {}", ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = l2seg->hal_handle;
    dhl_entry.obj = l2seg;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(l2seg->hal_handle, &cfg_ctxt,
                             l2seg_delete_del_cb,
                             l2seg_delete_commit_cb,
                             l2seg_delete_abort_cb,
                             l2seg_delete_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_L2SEGMENT_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_L2SEGMENT_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

static void
l2segment_process_get (l2seg_t *l2seg, L2SegmentGetResponse *rsp)
{
    hal_handle_t                *p_hdl_id   = NULL;
    NetworkKeyHandle            *nkh        = NULL;
    pd::pd_l2seg_get_args_t     args        = {0};
    hal_ret_t                   ret         = HAL_RET_OK;

    // fill config spec of this L2 segment
    rsp->mutable_spec()->mutable_vrf_key_handle()->set_vrf_id(vrf_lookup_by_handle(l2seg->vrf_handle)->vrf_id);
    rsp->mutable_spec()->mutable_key_or_handle()->set_segment_id(l2seg->seg_id);
    rsp->mutable_spec()->set_segment_type(l2seg->segment_type);
    rsp->mutable_spec()->set_mcast_fwd_policy(l2seg->mcast_fwd_policy);
    rsp->mutable_spec()->set_bcast_fwd_policy(l2seg->bcast_fwd_policy);
    rsp->mutable_spec()->mutable_wire_encap()->set_encap_type(l2seg->wire_encap.type);
    rsp->mutable_spec()->mutable_wire_encap()->set_encap_value(l2seg->wire_encap.val);
    rsp->mutable_spec()->mutable_tunnel_encap()->set_encap_type(l2seg->tunnel_encap.type);
    rsp->mutable_spec()->mutable_tunnel_encap()->set_encap_value(l2seg->tunnel_encap.val);

    if (l2seg->nw_list) {
        for (const void *ptr : *l2seg->nw_list) {
            p_hdl_id = (hal_handle_t *)ptr;
            nkh = rsp->mutable_spec()->add_network_key_handle();
            nkh->set_nw_handle(*p_hdl_id);
        }
    }

    if (l2seg->mbrif_list) {
        for (const void *ptr : *l2seg->mbrif_list) {
            p_hdl_id = (hal_handle_t *)ptr;
            auto ifkh = rsp->mutable_spec()->add_if_key_handle();
            ifkh->set_if_handle(*p_hdl_id);
        }
    }
#if 0
    lnode = l2seg->nw_list_head.next;
    dllist_for_each(lnode, &(l2seg->nw_list_head)) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        nkh = rsp->mutable_spec()->add_network_key_handle();
        nkh->set_nw_handle(entry->handle_id);
    }
#endif
    // fill operational state of this L2 segment
    rsp->mutable_status()->set_l2segment_handle(l2seg->hal_handle);

    // fill stats of this L2 segment
    rsp->mutable_stats()->set_num_endpoints(l2seg->num_ep);

    // Getting PD information
    args.l2seg = l2seg;
    args.rsp = rsp;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_GET, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for L2Segment id : {}. err : {}",
                      l2seg->seg_id, ret);
    }

    rsp->set_api_status(types::API_STATUS_OK);
}

static bool
l2segment_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    L2SegmentGetResponseMsg *rsp    = (L2SegmentGetResponseMsg *)ctxt;
    L2SegmentGetResponse    *response = rsp->add_response();
    l2seg_t                 *l2seg    = NULL;

    l2seg = (l2seg_t *)hal_handle_get_obj(entry->handle_id);
    l2segment_process_get(l2seg, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

static bool
l2segment_get_list_cb (void *list_entry, void *ctxt)
{
    hal_handle_t l2seg_handle         = *((hal_handle_t *)list_entry);
    l2seg_t      *l2seg               = NULL;
    L2SegmentGetResponseMsg *rsp      = (L2SegmentGetResponseMsg *)ctxt;
    L2SegmentGetResponse    *response = rsp->add_response();

    l2seg = l2seg_lookup_by_handle(l2seg_handle);

    // l2seg should not be deleted here because we are still in the context
    // of the config thread.
    HAL_ASSERT(l2seg != NULL);

    l2segment_process_get(l2seg, response);

    return true;
}

//------------------------------------------------------------------------------
// process a L2 segment get request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_get (L2SegmentGetRequest& req, L2SegmentGetResponseMsg *rsp)
{
    l2seg_t *l2seg = NULL;
    vrf_t   *vrf   = NULL;

    if (req.has_vrf_key_handle()) {
        vrf = vrf_lookup_key_or_handle(req.vrf_key_handle());
        if (!vrf) {
            auto response = rsp->add_response();
            response->set_api_status(types::API_STATUS_VRF_ID_INVALID);
            HAL_API_STATS_INC(HAL_API_L2SEGMENT_GET_FAIL);
            return HAL_RET_INVALID_ARG;
        }
    }

    if (!req.has_key_or_handle() && !req.has_vrf_key_handle()) {
        g_hal_state->l2seg_id_ht()->walk(l2segment_get_ht_cb, rsp);
    } else if (req.has_vrf_key_handle() && (!req.has_key_or_handle())) {
        vrf->l2seg_list->iterate(l2segment_get_list_cb, (void *)rsp);
    } else {
        l2seg = l2seg_lookup_key_or_handle(req.key_or_handle());
        auto response = rsp->add_response();
        if (l2seg == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_L2SEGMENT_GET_FAIL);
            return HAL_RET_L2SEG_NOT_FOUND;
        } else {
            l2segment_process_get(l2seg, response);
        }
    }
    HAL_API_STATS_INC(HAL_API_L2SEGMENT_GET_SUCCESS);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Adds If into l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_add_if (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t ret = HAL_RET_OK;

    if (l2seg == NULL || hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);      // lock
    ret = l2seg->mbrif_list->insert(&hal_if->hal_handle);
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);    // unlock

end:
    HAL_TRACE_DEBUG("add l2seg => if ,{} => {}, ret : {}",
                    l2seg->seg_id, hal_if->if_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove If from l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_del_if (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t ret = HAL_RET_OK;

    if (l2seg == NULL || hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);      // lock
    ret = l2seg->mbrif_list->remove(&hal_if->hal_handle);
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove from l2seg's if list. ret : {}",
                      ret);
        goto end;
    }

    HAL_TRACE_DEBUG("del l2seg =/=> if ,{} =/=> {}, ret : {}",
                   l2seg->seg_id, hal_if->if_id, ret);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Adds back ref If into l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_add_back_if (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t ret = HAL_RET_OK;

    if (l2seg == NULL || hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);      // lock
    ret = l2seg->if_list->insert(&hal_if->hal_handle);
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);    // unlock

end:
    HAL_TRACE_DEBUG("add l2seg => if ,{} => {}, ret : {}",
                    l2seg->seg_id, hal_if->if_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove If from l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_del_back_if (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t ret = HAL_RET_OK;

    if (l2seg == NULL || hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);      // lock
    ret = l2seg->if_list->remove(&hal_if->hal_handle);
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove from l2seg's if list. ret : {}",
                      ret);
        goto end;
    }

    HAL_TRACE_DEBUG("del l2seg =/=> if ,{} =/=> {}, ret : {}",
                   l2seg->seg_id, hal_if->if_id, ret);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Adds If into l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_add_acl (l2seg_t *l2seg, acl_t *acl)
{
    hal_ret_t ret = HAL_RET_OK;

    if (l2seg == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);      // lock
    ret = l2seg->acl_list->insert(&acl->hal_handle);
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);    // unlock

end:
    HAL_TRACE_DEBUG("add l2seg => acl ,{} => {}, ret : {}",
                    l2seg->seg_id, acl->key, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove acl from l2seg list
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_del_acl (l2seg_t *l2seg, acl_t *acl)
{
    hal_ret_t ret = HAL_RET_OK;

    if (l2seg == NULL || acl == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    l2seg_lock(l2seg, __FILENAME__, __LINE__, __func__);      // lock
    ret = l2seg->acl_list->remove(&acl->hal_handle);
    l2seg_unlock(l2seg, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove from l2seg's acl list. ret : {}",
                      ret);
        goto end;
    }

    HAL_TRACE_DEBUG("del l2seg =/=> acl ,{} =/=> {}, ret : {}",
                   l2seg->seg_id, acl->key, ret);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Hanlde nwsec update coming from vrf
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_handle_nwsec_update (l2seg_t *l2seg, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    hal_handle_t                *p_hdl_id = NULL;

    if (l2seg == NULL) {
        return ret;
    }

    HAL_TRACE_DEBUG("Handling nwsec update seg_id: {}",
                    l2seg->seg_id);

    // We have to check if we have to walk through uplinks in mbrifs

    // Walk through Ifs and call respective functions
    for (const void *ptr : *l2seg->if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        // TODO: Uncomment this after if is migrated to new scheme
        // hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);
        hal_if = find_if_by_handle(*p_hdl_id);
        if (!hal_if) {
            HAL_TRACE_ERR("Unable to find if with handle : {}",
                          *p_hdl_id);
            continue;
        }
        if_handle_nwsec_update(l2seg, hal_if, nwsec_prof);
    }

    return ret;
}

static void
l2seg_ep_learning_update (l2seg_t *l2seg, const L2SegmentSpec& spec)
{
    if (spec.has_eplearn_cfg()) {
        if (spec.eplearn_cfg().has_arp()) {
            if (spec.eplearn_cfg().arp().entry_timeout()) {
                l2seg->eplearn_cfg.arp_cfg.enabled = true;
                l2seg->eplearn_cfg.arp_cfg.entry_timeout = \
                        spec.eplearn_cfg().arp().entry_timeout();
            } else {
                l2seg->eplearn_cfg.arp_cfg.enabled = false;
            }
        }

        if (spec.eplearn_cfg().has_dhcp()) {
            if (spec.eplearn_cfg().dhcp().trusted_servers_size()) {
                l2seg->eplearn_cfg.dhcp_cfg.enabled = true;
            } else {
                l2seg->eplearn_cfg.dhcp_cfg.enabled = true;
            }
        }
    }
}

//-----------------------------------------------------------------------------
// given a l2seg, store it for persisting its state (spec, status, stats)
//
// obj points to vrf object i.e., l2seg_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    L2SegmentGetResponse    rsp;
    uint32_t                serialized_state_sz;
    l2seg_t                 *l2seg = (l2seg_t *)obj;

    HAL_ASSERT((l2seg != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this l2seg (includes spec, status & stats)
    l2segment_process_get(l2seg, &rsp);
    serialized_state_sz = rsp.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall L2 segment {}, not enough room, "
                      "required size {}, available size {}",
                      l2seg->seg_id, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (rsp.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize L2 segment {}", l2seg->seg_id);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled L2 segment {}, len {}",
                    l2seg->seg_id, serialized_state_sz);
    return HAL_RET_OK;
}

static hal_ret_t
l2seg_restore_add (l2seg_t *l2seg, const L2SegmentGetResponse& l2seg_info)
{
    hal_ret_t                       ret;
    pd::pd_l2seg_restore_args_t     pd_l2seg_args = { 0 };

    // restore pd state
    pd::pd_l2seg_restore_args_init(&pd_l2seg_args);
    pd_l2seg_args.l2seg = l2seg;
    pd_l2seg_args.l2seg_status = &l2seg_info.status();
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_RESTORE, &pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore l2seg {} pd, err : {}",
                      l2seg->seg_id, ret);
    }
    return ret;
}

static hal_ret_t
l2seg_restore_commit (l2seg_t *l2seg, const L2SegmentGetResponse& l2seg_info)
{
    hal_ret_t   ret;
    vrf_t       *vrf;

    // Add l2seg to vrf's l2seg list
    vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
    if (!vrf) {
        HAL_TRACE_ERR("Failed to find vrf {}", l2seg->vrf_handle);
        goto end;
    }

    // Add to DB and set back refs
    ret = l2seg_add_to_db_and_refs(l2seg, l2seg->hal_handle, vrf);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to add to DB or refs: l2seg:{}, err:{}",
                      l2seg->seg_id, ret);
        goto end;
    }

end:
    return ret;
}

static hal_ret_t
l2seg_restore_abort (l2seg_t *l2seg, const L2SegmentGetResponse& l2seg_info)
{
    HAL_TRACE_ERR("Aborting l2seg {} restore", l2seg->seg_id);
    l2seg_create_abort_cleanup(l2seg, l2seg->hal_handle);
    return HAL_RET_OK;
}

uint32_t
l2seg_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t               ret;
    L2SegmentGetResponse    l2seg_info;
    l2seg_t                 *l2seg;

    // de-serialize the object
    if (l2seg_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized l2seg obj");
        HAL_ASSERT(0);
        return 0;
    }

    // instantiate the L2 segment
    l2seg = l2seg_alloc_init();
    if (l2seg == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init l2seg, err : {}", ret);
        return 0;
    }

    l2seg_init_from_spec(l2seg, l2seg_info.spec());
    l2seg_init_from_status(l2seg, l2seg_info.status());
    l2seg_init_from_stats(l2seg, l2seg_info.stats());

    // repopulate handle db
    hal_handle_insert(HAL_OBJ_ID_L2SEG, l2seg->hal_handle, (void *)l2seg);

    l2seg_create_oifs(l2seg);

    ret = l2seg_restore_add(l2seg, l2seg_info);
    if (ret != HAL_RET_OK) {
        l2seg_restore_abort(l2seg, l2seg_info);
    }
    l2seg_restore_commit(l2seg, l2seg_info);

    return 0;
}

//------------------------------------------------------------------------------
// spec's keyhandle to str
//------------------------------------------------------------------------------
const char *
l2seg_spec_keyhandle_to_str (const L2SegmentKeyHandle& key_handle)
{
	static thread_local char       l2seg_str[4][50];
	static thread_local uint8_t    l2seg_str_next = 0;
	char                           *buf;

	buf = l2seg_str[l2seg_str_next++ & 0x3];
	memset(buf, 0, 50);

    if (key_handle.key_or_handle_case() == L2SegmentKeyHandle::kSegmentId) {
		snprintf(buf, 50, "l2seg_id: %lu", key_handle.segment_id());
    }
    if (key_handle.key_or_handle_case() == L2SegmentKeyHandle::kL2SegmentHandle) {
		snprintf(buf, 50, "l2seg_handle: 0x%lx", key_handle.l2segment_handle());
    }

	return buf;
}

//------------------------------------------------------------------------------
// PI l2seg's keyhandle to str
//------------------------------------------------------------------------------
const char *
l2seg_keyhandle_to_str (l2seg_t *l2seg)
{
    static thread_local char       l2seg_str[4][50];
    static thread_local uint8_t    l2seg_str_next = 0;
    char                           *buf;

    buf = l2seg_str[l2seg_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (l2seg) {
        snprintf(buf, 50, "l2seg(id: %lu, handle: %lu)",
                 l2seg->seg_id, l2seg->hal_handle);
    }
    return buf;
}

//-----------------------------------------------------------------------------
// print l2seg spec
//-----------------------------------------------------------------------------
void
l2seg_spec_dump (L2SegmentSpec& spec)
{
    std::string    l2seg_cfg;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &l2seg_cfg);
    HAL_TRACE_DEBUG("L2Seg configuration:");
    HAL_TRACE_DEBUG("{}", l2seg_cfg.c_str());
}
}    // namespace hal
