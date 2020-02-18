//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/mcast/multicast.hpp"
#include "nic/hal/plugins/cfg/mcast/oif_list_api.hpp"
#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/src/utils/utils.hpp"

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

char *
mc_key_handle_spec_to_string(MulticastEntryKeyHandle& spec)
{
    L2SegmentKeyHandle             l2seg_spec;
    static thread_local char       mc_key_str[4][120];
    static thread_local uint8_t    mc_key_str_next = 0;
    char                           l2seg_str[40];
    char                           *buf;

    buf = mc_key_str[mc_key_str_next++ & 0x3];

    if (spec.key_or_handle_case() == MulticastEntryKeyHandle::kKey) {
        l2seg_spec = spec.key().l2segment_key_handle();
        if (l2seg_spec.key_or_handle_case() == L2SegmentKeyHandle::kSegmentId) {
            snprintf(l2seg_str, 40, "L2SegId %lu", l2seg_spec.segment_id());
        } else if (l2seg_spec.key_or_handle_case() ==
                   L2SegmentKeyHandle::kL2SegmentHandle) {
            snprintf(l2seg_str, 40, "L2SegHndl %lu", l2seg_spec.l2segment_handle());
        } else {
            snprintf(l2seg_str, 40, "%s", "Unknown L2seg Type");
        }

        if (spec.key().has_ip()) {
            ip_addr_t src = {};
            ip_addr_t grp = {};
            ip_addr_spec_to_ip_addr(&src, spec.key().ip().source());
            ip_addr_spec_to_ip_addr(&grp, spec.key().ip().group());
            snprintf(buf, 120, "(%s, %s): %s",
                     ipaddr2str(&src), ipaddr2str(&grp), l2seg_str);
        } else if (spec.key().has_mac()) {
            mac_addr_t grp = {};
            MAC_UINT64_TO_ADDR(grp, spec.key().mac().group());
            snprintf(buf, 120, "[%s]: %s", macaddr2str(grp), l2seg_str);
        }
        else {
            snprintf(buf, 120, "Unknown Key Type: %s", l2seg_str);
        }
    } else if (spec.key_or_handle_case() ==
               MulticastEntryKeyHandle::kMulticastHandle) {
        snprintf(buf, 120, "%lu", spec.multicast_handle());
    }

    return buf;
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

    SDK_ASSERT(ht_entry != NULL);
    mc_entry = (mc_entry_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(mc_entry->key);
}

// ----------------------------------------------------------------------------
// hash table key size
// ----------------------------------------------------------------------------
uint32_t
mc_entry_key_size ()
{
    return sizeof(mc_key_t);
}

//------------------------------------------------------------------------------
// Lookup mc entry from key or handle
//------------------------------------------------------------------------------
mc_entry_t *
mc_entry_lookup_key_or_handle (const MulticastEntryKeyHandle& kh)
{
    mc_entry_t *mc_entry = NULL;

    if (kh.key_or_handle_case() == MulticastEntryKeyHandle::kKey) {
        mc_key_t mc_key = {};
        auto l2seg = l2seg_lookup_key_or_handle(kh.key().l2segment_key_handle());
        if (!l2seg) {
            return NULL;
        }
        mc_key.l2seg_handle = l2seg->hal_handle;
        if (kh.key().has_ip()) {
            mc_key.type = MC_KEY_TYPE_IP;
            ip_addr_spec_to_ip_addr(&mc_key.u.ip, kh.key().ip().group());
        } else if (kh.key().has_mac()) {
            mc_key.type = MC_KEY_TYPE_MAC;
            MAC_UINT64_TO_ADDR(mc_key.u.mac, kh.key().mac().group());
        }
        mc_entry = find_mc_entry_by_key(&mc_key);
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

    HAL_TRACE_DEBUG("Adding to mc_key hash table");
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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add mc key to handle mapping, err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

//------------------------------------------------------------------------------
// delete a mc_entry from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
mc_entry_del_from_db (mc_entry_t *mc_entry)
{
    hal_handle_id_ht_entry_t *entry;

    HAL_TRACE_DEBUG("Removing from mc key hash table");
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->mc_key_ht()->
            remove(&mc_entry->key);

    // free up
    return hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_prepare_rsp (MulticastEntryResponse *rsp, hal_ret_t ret, mc_entry_t *mc_entry)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_entry_status()->set_handle(mc_entry ?
                                                mc_entry->hal_handle :
                                                HAL_HANDLE_INVALID);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Creates and programs OIF list for a multicast entry
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_create_and_program_oifs (mc_entry_t *mc_entry)
{
    hal_ret_t                     ret;
    hal_handle_id_list_entry_t    *entry = NULL;
    dllist_ctxt_t                 *lnode = NULL;
    l2seg_t                       *l2seg = NULL;
    if_t                          *pi_if = NULL;
    oif_t                         oif = {};

    l2seg = l2seg_lookup_by_handle(mc_entry->key.l2seg_handle);
    SDK_ASSERT(l2seg != NULL);

#if 0
    ret = oif_list_create(&mc_entry->oif_list);
    SDK_ASSERT(ret == HAL_RET_OK);
#endif

    ret = oif_list_create_block(&mc_entry->oif_list, HAL_MC_ENTRY_OIFL_BLOCK);
    SDK_ASSERT(ret == HAL_RET_OK);

    // Always attaches to L2seg oifls. So no HI entries.
#if 0
    ret = oif_list_set_honor_ingress(mc_entry_mgmt_repls(mc_entry->oif_list));
    SDK_ASSERT(ret == HAL_RET_OK);

    ret = oif_list_set_honor_ingress(mc_entry_mseg_bm_repls(mc_entry->oif_list));
    SDK_ASSERT(ret == HAL_RET_OK);

    ret = oif_list_set_honor_ingress(mc_entry_mgmt_mseg_bm_repls(mc_entry->oif_list));
    SDK_ASSERT(ret == HAL_RET_OK);
#endif


    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_if = find_if_by_handle(entry->handle_id);
        SDK_ASSERT(pi_if != NULL);
        oif.intf = pi_if;
        oif.l2seg = l2seg;

        ret = oif_list_add_oif(mc_entry_mgmt_repls(mc_entry->oif_list), &oif);
        SDK_ASSERT(ret == HAL_RET_OK);
        ret = oif_list_add_oif(mc_entry_mgmt_mseg_bm_repls(mc_entry->oif_list), &oif);
        SDK_ASSERT(ret == HAL_RET_OK);
#if 0
        ret = oif_list_add_oif(mc_entry->oif_list, &oif);
        SDK_ASSERT(ret == HAL_RET_OK);

        ret = oif_list_add_oif(mc_entry_shared_oifl(mc_entry), &oif);
        SDK_ASSERT(ret == HAL_RET_OK);
#endif
    }

    if (l2seg_is_mgmt(l2seg)) {
        /*
         * 0: oifl[Reg. Repls] -> L2seg's flood_idx+1[ALL_MC]
         * 1: oifl[Empty] -> L2seg's flood_idx + 4[Empty] -> Cust L2seg's flood_idx + 4[WLs]
         * 2: oifl[Reg. Repls] -> L2seg's flood_idx + 7[ALL_MC] -> Cust L2seg's flood_idx + 7[WLs]
         */
        ret = oif_list_attach(mc_entry_mgmt_repls(mc_entry->oif_list), 
                              l2seg_mc_mgmt_oifl(l2seg_base_oifl_id(l2seg, NULL)));
        SDK_ASSERT(ret == HAL_RET_OK);
        ret = oif_list_attach(mc_entry_mseg_bm_repls(mc_entry->oif_list), 
                              l2seg_mc_mseg_bm_oifl(l2seg_base_oifl_id(l2seg, NULL)));
        SDK_ASSERT(ret == HAL_RET_OK);
        ret = oif_list_attach(mc_entry_mgmt_mseg_bm_repls(mc_entry->oif_list), 
                              l2seg_mc_mgmt_mseg_bm_oifl(l2seg_base_oifl_id(l2seg, NULL)));
        SDK_ASSERT(ret == HAL_RET_OK);


#if 0
        // Shared l2seg may not have attached yet, but all mcast repl. oiflists
        //  are attached to l2seg's ALL_MC which inturn will attach to
        //  customer l2seg's WLs
        ret = oif_list_attach(mc_entry->oif_list, l2seg_get_mcast_oif_list(l2seg));
        SDK_ASSERT(ret == HAL_RET_OK);

        ret = oif_list_attach(mc_entry_shared_oifl(mc_entry), 
                              l2seg_get_shared_mcast_oif_list(l2seg));
        SDK_ASSERT(ret == HAL_RET_OK);
#endif
    }

    return ret;
}

//------------------------------------------------------------------------------
// Deprogram and delete OIF list for a multicast entry
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_deprogram_and_delete_oifs(mc_entry_t *mc_entry)
{
    hal_ret_t                     ret;
    hal_handle_id_list_entry_t    *entry = NULL;
    dllist_ctxt_t                 *lnode = NULL;
    l2seg_t                       *l2seg = NULL;
    if_t                          *pi_if = NULL;
    oif_t                         oif = {};

    l2seg = l2seg_lookup_by_handle(mc_entry->key.l2seg_handle);
    if (!l2seg) {
        HAL_TRACE_ERR("L2segment not found");
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }

    if (l2seg_is_mgmt(l2seg)) {

        ret = oif_list_detach(mc_entry_mgmt_repls(mc_entry->oif_list));
        SDK_ASSERT(ret == HAL_RET_OK);
        ret = oif_list_detach(mc_entry_mseg_bm_repls(mc_entry->oif_list));
        SDK_ASSERT(ret == HAL_RET_OK);
        ret = oif_list_detach(mc_entry_mgmt_mseg_bm_repls(mc_entry->oif_list));
        SDK_ASSERT(ret == HAL_RET_OK);

#if 0
        ret = oif_list_detach(mc_entry->oif_list);
        SDK_ASSERT(ret == HAL_RET_OK);

        ret = oif_list_detach(mc_entry_shared_oifl(mc_entry));
        SDK_ASSERT(ret == HAL_RET_OK);
#endif
    }

    // Now Delete the OIFs
    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_if = find_if_by_handle(entry->handle_id);
        SDK_ASSERT(pi_if != NULL);
        oif.intf = pi_if;
        oif.l2seg = l2seg;

        ret = oif_list_remove_oif(mc_entry_mgmt_repls(mc_entry->oif_list), &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Del OIF failed! if-hndl:{}, l2seg-hndl:{}, ret:{}",
                          pi_if->hal_handle, l2seg->hal_handle, ret);
        }

        ret = oif_list_remove_oif(mc_entry_mgmt_mseg_bm_repls(mc_entry->oif_list), &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Del OIF failed! if-hndl:{}, l2seg-hndl:{}, ret:{}",
                          pi_if->hal_handle, l2seg->hal_handle, ret);
        }

#if 0
        ret = oif_list_remove_oif(mc_entry->oif_list, &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Del OIF failed! if-hndl:{}, l2seg-hndl:{}, ret:{}",
                          pi_if->hal_handle, l2seg->hal_handle, ret);
        }

        ret = oif_list_remove_oif(mc_entry_shared_oifl(mc_entry), &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Del OIF failed! if-hndl:{}, l2seg-hndl:{}, ret:{}",
                          pi_if->hal_handle, l2seg->hal_handle, ret);
        }
#endif
    }

#if 0
        ret = oif_list_clr_honor_ingress(mc_entry_mgmt_repls(mc_entry->oif_list));
        SDK_ASSERT(ret == HAL_RET_OK);

        ret = oif_list_clr_honor_ingress(mc_entry_mseg_bm_repls(mc_entry->oif_list));
        SDK_ASSERT(ret == HAL_RET_OK);

        ret = oif_list_clr_honor_ingress(mc_entry_mgmt_mseg_bm_repls(mc_entry->oif_list));
        SDK_ASSERT(ret == HAL_RET_OK);
#endif

    // Delete the OIF List
#if 0
    ret = oif_list_delete(mc_entry->oif_list);
    SDK_ASSERT(ret == HAL_RET_OK);
#endif
    ret = oif_list_delete_block(mc_entry->oif_list, HAL_MC_ENTRY_OIFL_BLOCK);
    SDK_ASSERT(ret == HAL_RET_OK);
    mc_entry->oif_list = OIF_LIST_ID_INVALID;

end:
    return ret;
}

//------------------------------------------------------------------------------
// Cleanup and delete a multicast entry
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_cleanup_and_delete(mc_entry_t *mc_entry)
{
    hal_ret_t                   ret;
    dllist_ctxt_t               *lnode = NULL;
    hal_handle_id_list_entry_t  *entry = NULL;
    if_t                        *pi_if = NULL;

    // Remove back references from each interface
    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_if = find_if_by_handle(entry->handle_id);
        SDK_ASSERT(pi_if != NULL);
        hal_remove_from_handle_list(&pi_if->mc_entry_list_head, mc_entry->hal_handle);
    }

    // Free the outgoing interface handles linked list
    hal_free_handles_list(&mc_entry->if_list_head);

    // Free the multicast entry
    ret = mc_entry_free(mc_entry);
    SDK_ASSERT(ret == HAL_RET_OK);
    return ret;
}

//------------------------------------------------------------------------------
// Reads OIFs from spec
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_read_oifs (mc_entry_t *mc_entry, MulticastEntrySpec& spec)
{
    hal_ret_t               ret = HAL_RET_OK;
    uint64_t                index = 0;
    uint32_t                num_oifs = 0;
    if_t                    *pi_if = NULL;
    hal_handle_t            hash_table[HAL_MAX_OIF_PER_MC_ENTRY];
    InterfaceKeyHandle      if_key_handle;

    num_oifs = (uint32_t) spec.oif_key_handles_size();
    if (num_oifs > HAL_MAX_OIF_PER_MC_ENTRY) {
        HAL_TRACE_ERR("Too many OIFs! Only {} supported per mc_entry",
                      HAL_MAX_OIF_PER_MC_ENTRY);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    HAL_TRACE_DEBUG("Received {} no. of oifs", num_oifs);

    memset(hash_table, 0x00, sizeof(hash_table));
    sdk::lib::dllist_reset(&mc_entry->if_list_head);

    for (uint32_t i = 0; i < num_oifs; i++) {
        if_key_handle = spec.oif_key_handles(i);
        pi_if = if_lookup_key_or_handle(if_key_handle);
        if (pi_if == NULL) {
            HAL_TRACE_ERR("pi_if in OIF[{}] not found", i);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }

        for(index = pi_if->hal_handle % HAL_MAX_OIF_PER_MC_ENTRY;
            hash_table[index];
            index = ((index + 1) % HAL_MAX_OIF_PER_MC_ENTRY)) {
            if (hash_table[index] == pi_if->hal_handle) {
                HAL_TRACE_ERR("Duplicate OIF in request! if_handle {}",
                              pi_if->hal_handle);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
            // Can not be an infinite loop since the table is as big as
            // the max number of OIFs allowed per entry, which was validated
            // above for this request. Every OIF will eventually find a
            // free slot.
        }

        hash_table[index] = pi_if->hal_handle;

        if (pi_if->if_type != intf::IF_TYPE_ENIC &&
            pi_if->if_type != intf::IF_TYPE_UPLINK &&
            pi_if->if_type != intf::IF_TYPE_UPLINK_PC) {
            HAL_TRACE_ERR("Only Enics/Uplink/UplinkPC allowed for OIFs");
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        HAL_TRACE_DEBUG("Adding if_id:{} type:{} handle:{} to oif.",
                        pi_if->if_id, pi_if->if_type, pi_if->hal_handle);

        // add if to list
        hal_add_to_handle_list(&mc_entry->if_list_head, pi_if->hal_handle);
    }

    hal_print_handles_list(&mc_entry->if_list_head);

end:
    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming Multicast Entry create request
// 1. check if Multicast Entry exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_mc_entry_create(MulticastEntrySpec& spec,
                         MulticastEntryResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle() || !spec.key_or_handle().has_key()) {
        HAL_TRACE_ERR("mc_entry key not set in create request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // must provide valid IP if providing IP based key
    if (spec.key_or_handle().key().has_ip()) {
        ip_addr_t ip_addr = {};
        ip_addr_spec_to_ip_addr(&ip_addr, spec.key_or_handle().key().ip().group());
        if (!ip_addr_is_multicast(&ip_addr)){
            HAL_TRACE_ERR("mc_entry ip not valid in request");
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    // must provide valid mac if providing mac based key
    if (spec.key_or_handle().key().has_mac()) {
        mac_addr_t mac_addr = {};
        MAC_UINT64_TO_ADDR(mac_addr, spec.key_or_handle().key().mac().group());
        if (!IS_MCAST_MAC_ADDR(mac_addr)){
            HAL_TRACE_ERR("mc_entry mac not valid in request");
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    // must provide valid l2segment
    if (!spec.key_or_handle().key().has_l2segment_key_handle()) {
        HAL_TRACE_ERR("mc_entry l2segment not valid in request");
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                     ret;
    dhl_entry_t                   *dhl_entry = NULL;
    mc_entry_t                    *mc_entry = NULL;
    dllist_ctxt_t                 *lnode = NULL;
    pd::pd_mc_entry_create_args_t pd_mc_entry_args = {};
    pd::pd_func_args_t            pd_func_args = {};

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;

    SDK_ASSERT(mc_entry);

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // Create and Program OIFS
    ret = mc_entry_create_and_program_oifs(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create OIFs, err : {}", ret);
        goto end;
    }

    // PD Call to allocate PD resources and HW programming
    pd::pd_mc_entry_create_args_init(&pd_mc_entry_args);
    pd_mc_entry_args.mc_entry = mc_entry;
    pd_func_args.pd_mc_entry_create = &pd_mc_entry_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create mc_entry pd, err : {}", ret);
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
    hal_ret_t                   ret;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    mc_entry_t                  *mc_entry = NULL;
    hal_handle_t                hal_handle = 0;
    hal_handle_id_list_entry_t  *entry = NULL;
    if_t                        *pi_if = NULL;

    SDK_ASSERT(cfg_ctxt);

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // 1. a. Add to mc key hash table
    ret = mc_entry_add_to_db(mc_entry, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add mc entry {} to db, err : {}",
                      mc_key_to_string(&mc_entry->key), ret);
        goto end;
    }

    // 2. Build back references from each interface
    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_if = find_if_by_handle(entry->handle_id);
        SDK_ASSERT(pi_if != NULL);
        hal_add_to_handle_list(&pi_if->mc_entry_list_head, mc_entry->hal_handle);
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
    hal_ret_t                     ret;
    dllist_ctxt_t                 *lnode = NULL;
    mc_entry_t                    *mc_entry = NULL;
    dhl_entry_t                   *dhl_entry = NULL;
    pd::pd_mc_entry_delete_args_t pd_mc_entry_args = {};
    pd::pd_func_args_t          pd_func_args = {};

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // delete call to PD
    if (mc_entry->pd) {
        pd::pd_mc_entry_delete_args_init(&pd_mc_entry_args);
        pd_mc_entry_args.mc_entry = mc_entry;
        pd_func_args.pd_mc_entry_delete = &pd_mc_entry_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete mc_entry pd, err : {}", ret);
        }
    }

    // Call deprogram OIF here
    ret = mc_entry_deprogram_and_delete_oifs(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete OIFs, err : {}", ret);
        goto end;
    }

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

hal_ret_t multicastentry_create(MulticastEntrySpec& spec,
                                MulticastEntryResponse *rsp)
{
    hal_ret_t                   ret;
    mc_entry_t                  *mc_entry = NULL;
    dhl_entry_t                 dhl_entry = {};
    cfg_op_ctxt_t               cfg_ctxt  = {};
    mc_entry_create_app_ctxt_t  app_ctxt  = {};
    L2SegmentKeyHandle          kh;
    l2seg_t                     *l2seg = NULL;

    hal_api_trace(" API Begin: mc entry create ");
    proto_msg_dump(spec);

    // validate the request message
    ret = validate_mc_entry_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Message validation Failed. ret: {}", ret);
        goto end;
    }

    // instantiate the mc_entry
    mc_entry = mc_entry_alloc_init();
    if (mc_entry == NULL) {
        HAL_TRACE_ERR("Unable to allocate handle/memory ret: {}", ret);
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
        HAL_TRACE_ERR("Failed to create a mc_entry, l2seg {} {} doesnt exist",
                      kh.segment_id(), kh.l2segment_handle());
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }
    mc_entry->key.l2seg_handle = l2seg->hal_handle;

    // check if the mc_entry exists already, and reject if one is found
    if (find_mc_entry_by_key(&mc_entry->key) != NULL) {
        HAL_TRACE_ERR("Failed to create a mc_entry, mc_entry {} exists already",
                      mc_key_to_string(&mc_entry->key));
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    mc_entry->oif_list = OIF_LIST_ID_INVALID;

    // allocate hal handle id
    mc_entry->hal_handle = hal_handle_alloc(HAL_OBJ_ID_MC_ENTRY);
    if (mc_entry->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc hal handle for mc_entry {}",
                      mc_key_to_string(&mc_entry->key));
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // read OIFs from spec
    ret = mc_entry_read_oifs(mc_entry, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Reading OIFs failed!");
        goto end;
    }

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
            hal_handle_free(mc_entry->hal_handle);
            mc_entry_free(mc_entry);
            mc_entry = NULL;
        }
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_CREATE_SUCCESS);
    }

    mc_entry_prepare_rsp(rsp, ret, mc_entry);
    hal_api_trace(" API End: mc entry create ");
    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming Multicast Entry delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_mc_entry_delete(MulticastEntryDeleteRequest& req,
                         MulticastEntryDeleteResponse *rsp)
{
    // must have key-handle set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("Key_or_handle not set");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // must have key-handle set
    if (req.key_or_handle().has_key()) {
        if (req.key_or_handle().key().has_ip()) {
            // must provide valid Multicast IP if providing IP based key
            ip_addr_t ip_addr = {};
            ip_addr_spec_to_ip_addr(&ip_addr, req.key_or_handle().key().ip().\
                                    group());
            if (!ip_addr_is_multicast(&ip_addr)){
                HAL_TRACE_ERR("ip not valid in request");
                rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                return HAL_RET_INVALID_ARG;
            }
        } else if (req.key_or_handle().key().has_mac()) {
            // must provide valid mac if providing mac based key
            mac_addr_t mac_addr = {};
            MAC_UINT64_TO_ADDR(mac_addr, req.key_or_handle().key().mac().group());
            if (!IS_MCAST_MAC_ADDR(mac_addr)){
                HAL_TRACE_ERR("MAC not valid in request");
                rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                return HAL_RET_INVALID_ARG;
            }
        }
        // must provide valid l2segment
        if (!req.key_or_handle().key().has_l2segment_key_handle()) {
            HAL_TRACE_ERR("l2segment not valid in request");
            rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (req.key_or_handle().multicast_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Handle not valid in request");
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        return HAL_RET_HANDLE_INVALID;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to delete PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                     ret;
    dhl_entry_t                   *dhl_entry = NULL;
    mc_entry_t                    *mc_entry = NULL;
    dllist_ctxt_t                 *lnode = NULL;
    pd::pd_mc_entry_delete_args_s pd_mc_entry_args = {};
    pd::pd_func_args_t          pd_func_args = {};

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;

    SDK_ASSERT(mc_entry);
    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // PD Call to allocate PD resources and HW programming
    pd::pd_mc_entry_delete_args_init(&pd_mc_entry_args);
    pd_mc_entry_args.mc_entry = mc_entry;
    pd_func_args.pd_mc_entry_delete = &pd_mc_entry_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete mc_entry pd, err : {}", ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as mc_entry_delete_del_cb() was a succcess
//      a. Delete from mc_key hash table
//      b. Remove object from handle id based hash table
//      c. Free PI l2seg
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    mc_entry_t                  *mc_entry = NULL;
    hal_handle_t                hal_handle;

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mc_entry = (mc_entry_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // Call deprogram OIF here
    ret = mc_entry_deprogram_and_delete_oifs(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete OIFs, err : {}", ret);
        goto end;
    }

    // Remove the multicast entry from Key based hash table
    ret = mc_entry_del_from_db(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete mc_entry from db, err : {}", ret);
        goto end;
    }

    // Cleanup and delete the mc_entry
    ret = mc_entry_cleanup_and_delete(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete entry. ret:{}", ret);
        goto end;
    }

    // Free the multicast entry's HAL Handle
    hal_handle_free(hal_handle);

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit CBs can't fail: ret:{}", ret);
        SDK_ASSERT(0);
    }
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t multicastentry_delete(MulticastEntryDeleteRequest& req,
                                MulticastEntryDeleteResponse *rsp)
{
    hal_ret_t                   ret;
    mc_entry_t                  *mc_entry;
    dhl_entry_t                 dhl_entry = {};
    cfg_op_ctxt_t               cfg_ctxt  = {};
    mc_entry_create_app_ctxt_t  app_ctxt  = {};
    L2SegmentKeyHandle          l2segkh;
    MulticastEntryKeyHandle     mcastkh;
    mc_key_t                    mc_key = {};

    hal_api_trace(" API Begin: mc entry delete ");

    // validate the request message
    ret = validate_mc_entry_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Validation Failed. ret: {}", ret);
        goto end;
    }

    mcastkh = req.key_or_handle();

    mc_entry = mc_entry_lookup_key_or_handle(mcastkh);
    if (mc_entry == NULL) {
        HAL_TRACE_ERR("mc_entry not found: {}",
                      mc_key_handle_spec_to_string(mcastkh));
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    memcpy(&mc_key, &mc_entry->key, sizeof(mc_key_t));

    // form ctxt and call infra add
    dhl_entry.handle = mc_entry->hal_handle;
    dhl_entry.obj = mc_entry;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(mc_entry->hal_handle, &cfg_ctxt,
                             mc_entry_delete_del_cb,
                             mc_entry_delete_commit_cb,
                             mc_entry_delete_abort_cb,
                             mc_entry_delete_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_DELETE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_DELETE_SUCCESS);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: mc entry delete ");
    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming Multicast Entry delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_mc_entry_update(MulticastEntrySpec& req,
                         MulticastEntryResponse *rsp)
{
    // must have key-handle set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("Key_or_handle not set");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // must have key-handle set
    if (req.key_or_handle().has_key()) {
        if (req.key_or_handle().key().has_ip()) {
            // must provide valid Multicast IP if providing IP based key
            ip_addr_t ip_addr = {};
            ip_addr_spec_to_ip_addr(&ip_addr, req.key_or_handle().key().ip().\
                                group());
            if (!ip_addr_is_multicast(&ip_addr)){
                HAL_TRACE_ERR("ip not valid in request");
                rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                return HAL_RET_INVALID_ARG;
            }
        } else if (req.key_or_handle().key().has_mac()) {
            // must provide valid mac if providing mac based key
            mac_addr_t mac_addr = {};
            MAC_UINT64_TO_ADDR(mac_addr, req.key_or_handle().key().mac().group());
            if (!IS_MCAST_MAC_ADDR(mac_addr)){
                HAL_TRACE_ERR("MAC not valid in request");
                rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                return HAL_RET_INVALID_ARG;
            }
        }
        // must provide valid l2segment
        if (!req.key_or_handle().key().has_l2segment_key_handle()) {
            HAL_TRACE_ERR("l2segment not valid in request");
            rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (req.key_or_handle().multicast_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Handle not valid in request");
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        return HAL_RET_HANDLE_INVALID;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to delete PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                     ret;
    dhl_entry_t                   *dhl_entry = NULL;
    mc_entry_t                    *mc_entry, *upd_entry;
    dllist_ctxt_t                 *lnode = NULL;
    pd::pd_mc_entry_update_args_t pd_mc_entry_args = {};
    pd::pd_func_args_t          pd_func_args = {};

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    upd_entry = (mc_entry_t *)dhl_entry->cloned_obj;
    mc_entry = (mc_entry_t *)dhl_entry->obj;

    SDK_ASSERT(mc_entry && upd_entry);

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // Create and Program OIFS
    ret = mc_entry_create_and_program_oifs(upd_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create OIFs, err : {}", ret);
        goto end;
    }

    // PD Call to allocate PD resources and HW programming
    pd::pd_mc_entry_update_args_init(&pd_mc_entry_args);
    pd_mc_entry_args.mc_entry = mc_entry;
    pd_mc_entry_args.upd_entry = upd_entry;
    pd_func_args.pd_mc_entry_update = &pd_mc_entry_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update mc_entry pd, err : {}", ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as mc_entry_update_del_cb() was a succcess
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    mc_entry_t                  *mc_entry, *upd_entry;
    hal_handle_id_list_entry_t  *entry = NULL;
    if_t                        *pi_if = NULL;

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    upd_entry = (mc_entry_t *)dhl_entry->cloned_obj;
    mc_entry = (mc_entry_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("MC entry {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // Delete the old OIF lists
    ret = mc_entry_deprogram_and_delete_oifs(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete OIFs, err : {}", ret);
        goto end;
    }

    // Cleanup and delete the mc_entry
    ret = mc_entry_cleanup_and_delete(mc_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete old entry. ret:{}", ret);
        goto end;
    }

    // Add back references to each interface in the new OIF list
    dllist_for_each(lnode, &upd_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_if = find_if_by_handle(entry->handle_id);
        SDK_ASSERT(pi_if != NULL);
        hal_add_to_handle_list(&pi_if->mc_entry_list_head, upd_entry->hal_handle);
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("commit CBs can't fail: ret:{}", ret);
        SDK_ASSERT(0);
    }
    return ret;
}

//------------------------------------------------------------------------------
// If Update fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    mc_entry_t                  *upd_entry = NULL;

    SDK_ASSERT(cfg_ctxt);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    upd_entry = (mc_entry_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("MC entry {}, key: {}", upd_entry->hal_handle,
                    mc_key_to_string(&upd_entry->key));

    // Call deprogram OIF here
    ret = mc_entry_deprogram_and_delete_oifs(upd_entry);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete OIFs, err : {}", ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// If Update fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
mc_entry_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t multicastentry_update(MulticastEntrySpec& req,
                                MulticastEntryResponse *rsp)
{
    hal_ret_t                   ret;
    mc_entry_t                  *mc_entry  = NULL;
    mc_entry_t                  *upd_entry = NULL;
    dhl_entry_t                 dhl_entry  = {};
    cfg_op_ctxt_t               cfg_ctxt   = {};
    mc_entry_create_app_ctxt_t  app_ctxt   = {};
    L2SegmentKeyHandle          l2segkh;
    MulticastEntryKeyHandle     mcastkh;

    hal_api_trace(" API Begin: mc entry update ");
	proto_msg_dump(req);

    // validate the request message
    ret = validate_mc_entry_update(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("mc entry upd Validation Failed. ret: {}", ret);
        goto end;
    }

    mcastkh = req.key_or_handle();

    mc_entry = mc_entry_lookup_key_or_handle(mcastkh);
    if (mc_entry == NULL) {
        HAL_TRACE_ERR("mc_entry upd  not found: {}",
                      mc_key_handle_spec_to_string(mcastkh));
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("MC entry upd  {}, key: {}", mc_entry->hal_handle,
                    mc_key_to_string(&mc_entry->key));

    // instantiate a new mc_entry
    upd_entry = mc_entry_alloc_init();
    if (upd_entry == NULL) {
        HAL_TRACE_ERR("Unable to allocate handle/memory ret: {}", ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(upd_entry, mc_entry, sizeof(mc_entry_t));
    upd_entry->oif_list = OIF_LIST_ID_INVALID;

    // Read the new OIFs into upd_entry from spec
    ret = mc_entry_read_oifs(upd_entry, req);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Reading OIFs failed!");
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = mc_entry->hal_handle;
    dhl_entry.obj = mc_entry;
    dhl_entry.cloned_obj = upd_entry;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(mc_entry->hal_handle, &cfg_ctxt,
                             mc_entry_update_upd_cb,
                             mc_entry_update_commit_cb,
                             mc_entry_update_abort_cb,
                             mc_entry_update_cleanup_cb);

end:
    if (ret != HAL_RET_OK) {
        if (upd_entry) {
            hal_free_handles_list(&upd_entry->if_list_head);
            mc_entry_free(upd_entry);
            mc_entry = NULL;
        }
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_UPDATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_UPDATE_SUCCESS);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace("API End: mc entry update ");
    return ret;
}

static void
mc_entry_get_fill_rsp(MulticastEntryGetResponse *rsp,
                      mc_entry_t *mc_entry)
{
    hal_ret_t                       ret;
    uint32_t                        numoif = 0;
    if_t                            *intf  = NULL;
    l2seg_t                         *l2seg = NULL;
    dllist_ctxt_t                   *lnode = NULL;
    hal_handle_id_list_entry_t      *entry = NULL;
    pd::pd_func_args_t              pd_func_args = {};
    pd::pd_mc_entry_get_args_t      pd_mc_entry_get_args = {};

    SDK_ASSERT(mc_entry);

    l2seg = l2seg_lookup_by_handle(mc_entry->key.l2seg_handle);
    if (l2seg == NULL) {
        rsp->set_api_status(types::API_STATUS_ERR);
        goto end;
    }

    rsp->mutable_spec()->mutable_key_or_handle()->mutable_key()->
        mutable_l2segment_key_handle()->set_segment_id(l2seg->seg_id);

    if (mc_entry->key.type == MC_KEY_TYPE_IP) {
        if (mc_entry->key.u.ip.af == types::IP_AF_INET) {
            rsp->mutable_spec()->mutable_key_or_handle()->mutable_key()->
                mutable_ip()->mutable_group()->set_ip_af(types::IP_AF_INET);
            rsp->mutable_spec()->mutable_key_or_handle()->mutable_key()->
                mutable_ip()->mutable_group()->set_v4_addr(mc_entry->
                key.u.ip.addr.v4_addr);
        } else if (mc_entry->key.u.ip.af == types::IP_AF_INET6) {
            rsp->mutable_spec()->mutable_key_or_handle()->mutable_key()->
                mutable_ip()->mutable_group()->set_ip_af(types::IP_AF_INET6);
            rsp->mutable_spec()->mutable_key_or_handle()->mutable_key()->
                mutable_ip()->mutable_group()->set_v6_addr(&mc_entry->
                key.u.ip.addr.v6_addr, sizeof(ipv6_addr_t));
        } else {
            rsp->set_api_status(types::API_STATUS_ERR);
            goto end;
        }
    } else if (mc_entry->key.type == MC_KEY_TYPE_MAC) {
        rsp->mutable_spec()->mutable_key_or_handle()->mutable_key()->
            mutable_mac()->set_group(MAC_TO_UINT64(mc_entry->key.u.mac));
    } else {
        rsp->set_api_status(types::API_STATUS_ERR);
        goto end;
    }

    dllist_for_each(lnode, &mc_entry->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        intf  = (if_t *)hal_handle_get_obj(entry->handle_id);
        SDK_ASSERT(intf);
        rsp->mutable_spec()->add_oif_key_handles()->set_interface_id(intf->if_id);
        numoif ++;
    }

    // Stats
    rsp->mutable_stats()->set_num_oifs(numoif);

    //Status
    rsp->mutable_status()->set_handle(mc_entry->hal_handle);

    // PD Status
    pd::pd_mc_entry_get_args_init(&pd_mc_entry_get_args);
    pd_mc_entry_get_args.mc_entry = mc_entry;
    pd_mc_entry_get_args.rsp = rsp;
    pd_func_args.pd_mc_entry_get = &pd_mc_entry_get_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MC_ENTRY_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get mc_entry pd, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_ERR);
        goto end;
    }

    // OIF Status
    ret = oif_list_get(mc_entry->oif_list,
            rsp->mutable_status()->mutable_oif_list());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get oif list, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_ERR);
        goto end;
    }

    // Status OK
    rsp->set_api_status(types::API_STATUS_OK);

end:
    return;
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

hal_ret_t multicastentry_get(MulticastEntryGetRequest& req,
                             MulticastEntryGetResponseMsg *rsp)
{
    mc_entry_t                  *mc_entry;
    MulticastEntryGetResponse   *response;

    if (!req.has_key_or_handle()) {
        g_hal_state->mc_key_ht()->walk(mc_entry_get_ht_cb, rsp);
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_GET_SUCCESS);
        return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    response = rsp->add_response();
    mc_entry = mc_entry_lookup_key_or_handle(kh);
    if (!mc_entry) {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
        HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_GET_FAIL);
        return HAL_RET_INVALID_ARG;
    }

    mc_entry_get_fill_rsp(response, mc_entry);

    HAL_API_STATS_INC(HAL_API_MULTICASTENTRY_GET_SUCCESS);

    return HAL_RET_OK;
}

}    // namespace hal
