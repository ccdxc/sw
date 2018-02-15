// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/include/ip.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/multicast.hpp"
#include "nic/include/oif_list_api.hpp"
#include "nic/hal/src/if_utils.hpp"
#include "nic/hal/src/utils.hpp"

extern char *macaddr2str (const mac_addr_t mac_addr);

namespace hal {

char *
mc_key_to_string (mc_key_t *key)
{
    if (key->type == MC_KEY_TYPE_IP) {
        return ipaddr2str(&key->u.ip);
    } else {
        return macaddr2str(key->u.mac);
    }
}

// ----------------------------------------------------------------------------
// hash table key => entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
mc_entry_get_key_func (void *entry)
{
    mc_entry_t                  *mc_entry = NULL;
    hal_handle_id_ht_entry_t    *ht_entry = (hal_handle_id_ht_entry_t *)entry;

    HAL_ASSERT(ht_entry != NULL);
    mc_entry = (mc_entry_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(mc_entry->key);
}

// ----------------------------------------------------------------------------
// hash table key => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
mc_entry_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(mc_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table key => entry - compare function
// ----------------------------------------------------------------------------
bool
mc_entry_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    return (memcmp(key1, key2, sizeof(mc_key_t)) == 0);
}

//------------------------------------------------------------------------------
// Lookup mc entry from key or handle
//------------------------------------------------------------------------------
mc_entry_t *
mc_entry_lookup_key_or_handle (const MulticastEntryKeyHandle& kh)
{
    mc_entry_t *mc_entry = NULL;

    if (kh.key_or_handle_case() == MulticastEntryKeyHandle::kKey) {
        mc_key_t mc_key;
        auto l2seg = l2seg_lookup_key_or_handle(kh.key().l2segment_key_handle());
        if (!l2seg) {
            return NULL;
        }
        mc_key.l2seg_handle = l2seg->hal_handle;
        if (kh.key().has_ip()) {
            ip_addr_spec_to_ip_addr(&mc_key.u.ip, kh.key().ip().group());
            mc_entry = find_mc_entry_by_key(&mc_key);
        } else if (kh.key().has_mac()) {
            MAC_UINT64_TO_ADDR(mc_key.u.mac, kh.key().mac().group());
            mc_entry = find_mc_entry_by_key(&mc_key);
        }
    } else if (kh.key_or_handle_case() == MulticastEntryKeyHandle::kMulticastHandle) {
        mc_entry = find_mc_entry_by_handle(kh.multicast_handle());
    }

    return mc_entry;
}

//------------------------------------------------------------------------------
// insert a mc_entry to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
mc_entry_add_to_db (mc_entry_t *mc_entry, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("{}:adding to mc_key hash table", __FUNCTION__);
    // allocate an entry to establish mapping from seg id to its handle
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
             hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from mc_key to handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->mc_key_ht()->insert_with_key(&mc_entry->key, entry,
                                                        &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add mc key to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a mc_entry from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
mc_entry_del_from_db (mc_entry_t *mc_entry)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("{}:removing from mc key hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->mc_key_ht()->
            remove(&mc_entry->key);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming Multicast Entry create request
// 1. check if Multicast Entry exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_mc_entry_create(MulticastEntrySpec& spec,
                         MulticastEntryResponse *rsp)
{
    // must have meta set
    if (!spec.has_meta()) {
        HAL_TRACE_ERR("{}:meta not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // must have key-handle set
    if (!spec.has_key_or_handle() || !spec.key_or_handle().has_key()) {
        HAL_TRACE_ERR("{}:mc_entry key not set in create request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // must provide valid IP if providing IP based key
    if (spec.key_or_handle().key().has_ip()) {
        ip_addr_t ip_addr = {0};
        ip_addr_spec_to_ip_addr(&ip_addr, spec.key_or_handle().key().ip().group());
        if (!ip_addr_is_multicast(&ip_addr)){
            HAL_TRACE_ERR("{}:mc_entry ip not valid in request",
                          __FUNCTION__);
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    // must provide valid mac if providing mac based key
    if (spec.key_or_handle().key().has_mac()) {
        mac_addr_t mac_addr = {0};
        MAC_UINT64_TO_ADDR(mac_addr, spec.key_or_handle().key().mac().group());
        if (!IS_MCAST_MAC_ADDR(mac_addr)){
            HAL_TRACE_ERR("{}:mc_entry mac not valid in request",
                          __FUNCTION__);
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    // must provide valid l2segment
    if (!spec.key_or_handle().key().has_l2segment_key_handle()) {
        HAL_TRACE_ERR("{}:mc_entry l2segment not valid in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_prepare_rsp (MulticastEntryResponse *rsp, hal_ret_t ret, mc_entry_t *mc_entry)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_entry_status()->set_multicast_handle(mc_entry ?
                                                          mc_entry->hal_handle :
                                                          HAL_HANDLE_INVALID);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Reads OIFs from spec
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_read_oifs (mc_entry_t *mc_entry, MulticastEntrySpec& spec)
{
    hal_ret_t               ret = HAL_RET_OK;
    uint32_t                num_oifs = 0;
    if_t                    *pi_if = NULL;
    InterfaceKeyHandle      if_key_handle;

    num_oifs = (uint32_t) spec.oif_key_handles_size();

    HAL_TRACE_DEBUG("{}:adding {} no. of oifs", __FUNCTION__, num_oifs);
    HAL_TRACE_DEBUG("{}:received {} oifs", __FUNCTION__, num_oifs);

    sdk::lib::dllist_reset(&mc_entry->if_list_head);
    for (uint32_t i = 0; i < num_oifs; i++) {
        if_key_handle = spec.oif_key_handles(i);
        pi_if = if_lookup_key_or_handle(if_key_handle);
        if (pi_if == NULL) {
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }

        HAL_TRACE_DEBUG("{}:adding if_id:{} type:{} handle:{} to oif.",
                        __FUNCTION__, pi_if->hal_handle);

        // add if to list
        hal_add_to_handle_list(&mc_entry->if_list_head, pi_if->hal_handle);
    }

    HAL_TRACE_DEBUG("{}:oifs added:", __FUNCTION__);
    hal_print_handles_list(&mc_entry->if_list_head);

end:
    return ret;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                     ret = HAL_RET_OK;
    dhl_entry_t                   *dhl_entry = NULL;
    mc_entry_t                    *mc_entry = NULL;
    hal_handle_id_list_entry_t    *entry = NULL;
    dllist_ctxt_t                 *lnode = NULL;
    l2seg_t                       *l2seg = NULL;
    if_t                          *pi_if = NULL;
    oif_t                         oif = { 0 };
    pd::pd_mc_entry_create_args_t pd_mc_entry_args = { 0 };

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;

    HAL_ASSERT(mc_entry);

    HAL_TRACE_DEBUG("{}:create add CB {}",__FUNCTION__,
                    mc_entry->hal_handle);

    l2seg = l2seg_lookup_by_handle(mc_entry->key.l2seg_handle);
    HAL_ASSERT(l2seg != NULL);
    ret = oif_list_create(&mc_entry->oif_list);
    HAL_ASSERT(ret == HAL_RET_OK);
    ret = oif_list_set_honor_ingress(mc_entry->oif_list);
    HAL_ASSERT(ret == HAL_RET_OK);

    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_if = find_if_by_handle(entry->handle_id);
        HAL_ASSERT(pi_if != NULL && pi_if->if_type == intf::IF_TYPE_ENIC);
        oif.intf = pi_if;
        oif.l2seg = l2seg;
        ret = oif_list_add_oif(mc_entry->oif_list, &oif);
        HAL_ASSERT(ret == HAL_RET_OK);
    }

    // Check all the other Classic Enics on this l2seg and add
    // them if they have a packet filter of type all-multicast
    for (const void *ptr : *l2seg->if_list) {
        auto p_hdl_id = (hal_handle_t *)ptr;
        pi_if = find_if_by_handle(*p_hdl_id);
        if (!pi_if) {
            HAL_TRACE_ERR("mc_entry_create_add_cb:{}:"
                          "unable to find if with handle:{}",
                          __FUNCTION__, entry->handle_id);
            continue;
        }

        if (pi_if->if_type == intf::IF_TYPE_ENIC &&
            pi_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC) {

            lif_t *lif = find_lif_by_handle(pi_if->lif_handle);
            HAL_ASSERT(lif != NULL);

            if (lif->packet_filters.receive_all_multicast) {
                oif.intf = pi_if;
                oif.l2seg = l2seg;
                ret = oif_list_add_oif(mc_entry->oif_list, &oif);
                HAL_ASSERT(ret == HAL_RET_OK);
            }
        }
    }

    // PD Call to allocate PD resources and HW programming
    pd::pd_mc_entry_create_args_init(&pd_mc_entry_args);
    pd_mc_entry_args.mc_entry = mc_entry;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_CREATE, (void *)&pd_mc_entry_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to create mc_entry pd, err : {}",
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as mc_entry_create_add_cb() was a success
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    mc_entry_t                  *mc_entry = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}: create commit CB {}", __FUNCTION__,
                    mc_key_to_string(&mc_entry->key));

    // 1. a. Add to mc key hash table
    ret = mc_entry_add_to_db(mc_entry, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add mc entry {} to db, err : {}",
                      __FUNCTION__, mc_key_to_string(&mc_entry->key), ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// mc_entry_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                     ret = HAL_RET_OK;
    hal_handle_t                  hal_handle = 0;
    dllist_ctxt_t                 *lnode = NULL;
    mc_entry_t                    *mc_entry = NULL;
    dhl_entry_t                   *dhl_entry = NULL;
    pd::pd_mc_entry_delete_args_t pd_mc_entry_args = { 0 };

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:create abort CB {}",
                    __FUNCTION__, mc_key_to_string(&mc_entry->key));

    // delete call to PD
    if (mc_entry->pd) {
        pd::pd_mc_entry_delete_args_init(&pd_mc_entry_args);
        pd_mc_entry_args.mc_entry = mc_entry;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_DELETE, (void *)&pd_mc_entry_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:failed to delete mc_entry pd, err : {}",
                          __FUNCTION__, ret);
        }
    }

    // clean up oifs as these are inserted before callbacks
    HAL_TRACE_DEBUG("{}:freeing up if list entries", __FUNCTION__);
    hal_free_handles_list(&mc_entry->if_list_head);

    // remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // Free mc entry
    mc_entry_free(mc_entry);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
mc_entry_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

hal_ret_t multicast_entry_create(MulticastEntrySpec& spec,
                                 MulticastEntryResponse *rsp)
{
    hal_ret_t                   ret;
    mc_entry_t                  *mc_entry = NULL;
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt  = { 0 };
    mc_entry_create_app_ctxt_t  app_ctxt  = { 0 };
    L2SegmentKeyHandle          kh;
    l2seg_t                     *l2seg = NULL;

    hal_api_trace(" API Begin: mc entry create ");

    // validate the request message
    ret = validate_mc_entry_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:validation Failed. ret: {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // instantiate the mc_entry
    mc_entry = mc_entry_alloc_init();
    if (mc_entry == NULL) {
        HAL_TRACE_ERR("{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    if (spec.key_or_handle().key().has_mac()) {
        mc_entry->key.type = MC_KEY_TYPE_MAC;
        MAC_UINT64_TO_ADDR(mc_entry->key.u.mac,
                           spec.key_or_handle().key().mac().group());
    } else {
        mc_entry->key.type = MC_KEY_TYPE_IP;
        ip_addr_spec_to_ip_addr(&mc_entry->key.u.ip,
                                spec.key_or_handle().key().ip().group());
    }

    kh = spec.key_or_handle().key().l2segment_key_handle();
    // make sure the l2segment is configured
    l2seg = l2seg_lookup_key_or_handle(kh);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("{}:failed to create a mc_entry, "
                      "l2seg {} {} doesnt exist", __FUNCTION__,
                      kh.segment_id(), kh.l2segment_handle());
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }
    mc_entry->key.l2seg_handle = l2seg->hal_handle;

    HAL_TRACE_DEBUG("{}:mc_entry create :{}", __FUNCTION__,
                    mc_key_to_string(&mc_entry->key));

    // check if the mc_entry exists already, and reject if one is found
    if (find_mc_entry_by_key(&mc_entry->key) != NULL) {
        HAL_TRACE_ERR("{}:failed to create a mc_entry, "
                      "mc_entry {} exists already", __FUNCTION__,
                      mc_key_to_string(&mc_entry->key));
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    mc_entry->oif_list = OIF_LIST_ID_INVALID;

    // allocate hal handle id
    mc_entry->hal_handle = hal_handle_alloc(HAL_OBJ_ID_MC_ENTRY);
    if (mc_entry->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle {}",
                      __FUNCTION__, mc_key_to_string(&mc_entry->key));
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // read OIFs from spec
    mc_entry_read_oifs(mc_entry, spec);

    // form ctxt and call infra add
    dhl_entry.handle = mc_entry->hal_handle;
    dhl_entry.obj = mc_entry;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(mc_entry->hal_handle, &cfg_ctxt,
                             mc_entry_create_add_cb,
                             mc_entry_create_commit_cb,
                             mc_entry_create_abort_cb,
                             mc_entry_create_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        if (mc_entry) {
            hal_free_handles_list(&mc_entry->if_list_head);
            mc_entry_free(mc_entry);
            mc_entry = NULL;
        }
    }

    mc_entry_prepare_rsp(rsp, ret, mc_entry);
    hal_api_trace(" API End: mc entry create ");
    return ret;
}

hal_ret_t multicast_entry_update(MulticastEntrySpec& spec,
                                 MulticastEntryResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t multicast_entry_delete(MulticastEntryDeleteRequest& req,
                                 MulticastEntryDeleteResponse *rsp)
{
    return HAL_RET_OK;
}

static void
mc_entry_get_fill_rsp(MulticastEntryGetResponse *rsp,
                      mc_entry_t *mc_entry)
{
    dllist_ctxt_t                   *lnode = NULL;
    hal_handle_id_list_entry_t      *entry = NULL;

    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        rsp->add_oif_handles(entry->handle_id);
    }

    rsp->set_api_status(types::API_STATUS_OK);
}

static bool
mc_entry_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t        *entry      = (hal_handle_id_ht_entry_t *)ht_entry;
    MulticastEntryGetResponseMsg    *response   = (MulticastEntryGetResponseMsg *)ctxt;
    mc_entry_t                      *mc_entry   = NULL;
    MulticastEntryGetResponse       *rsp;

    mc_entry = (mc_entry_t *)hal_handle_get_obj(entry->handle_id);
    rsp = response->add_response();

    mc_entry_get_fill_rsp(rsp, mc_entry);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

hal_ret_t multicast_entry_get(MulticastEntryGetRequest& req,
                              MulticastEntryGetResponseMsg *rsp)
{
    mc_entry_t                  *mc_entry;
    MulticastEntryGetResponse   *response;

    if (!req.has_key_or_handle()) {
        g_hal_state->mc_key_ht()->walk(mc_entry_get_ht_cb, rsp);
        return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    response = rsp->add_response();
    mc_entry = mc_entry_lookup_key_or_handle(kh);
    if (!mc_entry) {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    mc_entry_get_fill_rsp(response, mc_entry);

    return HAL_RET_OK;
}

}    // namespace hal
