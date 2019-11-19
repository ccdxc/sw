//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/pd/iris/nw/l2seg_uplink_pd.hpp"
#include "nic/hal/pd/iris/nw/enicif_pd.hpp"
#include "nic/hal/pd/iris/nw/endpoint_pd.hpp"
#include "nic/hal/pd/iris/p4pd_defaults.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/plugins/cfg/nw/vrf_api.hpp"

namespace hal {
namespace pd {

#define inp_prop data.action_u.input_properties_input_properties

hal_ret_t l2seg_pd_alloc_res(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_alloc_cpuid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_alloc_hwid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_dealloc_res(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_dealloc_cpuid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_dealloc_hwid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_cleanup(pd_l2seg_t *l2seg_pd);
uint32_t l2seg_pd_l2seguplink_count(pd_l2seg_t *l2seg_pd);
uint32_t pd_l2seg_get_l4_prof_idx(pd_l2seg_t *pd_l2seg);
pd_vrf_t *pd_l2seg_get_pd_vrf(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_program_hw(pd_l2seg_t *l2seg_pd,
                              bool is_upgrade = false);
hal_ret_t l2seg_pd_pgm_inp_prop_tbl(pd_l2seg_t *l2seg_pd,
                                    bool is_upgrade = false);
hal_ret_t l2seg_pd_deprogram_hw(pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_pd_depgm_inp_prop_tbl(pd_l2seg_t *l2seg_pd);

//-----------------------------------------------------------------------------
// key function for flow lkupid hash table
//-----------------------------------------------------------------------------
void *
flow_lkupid_get_hw_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry = NULL;
    l2seg_t                  *l2seg    = NULL;
    pd_l2seg_t               *l2seg_pd = NULL;
    vrf_t                    *vrf      = NULL;
    pd_vrf_t                 *vrf_pd   = NULL;

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if ((ht_entry == NULL) || (ht_entry->handle_id == HAL_HANDLE_INVALID)) {
        return NULL;
    }
    if (hal_handle_get_from_handle_id(ht_entry->handle_id)->obj_id() ==
                                      HAL_OBJ_ID_L2SEG) {
        l2seg = (l2seg_t *)hal_handle_get_obj(ht_entry->handle_id);
        l2seg_pd = (pd_l2seg_t *)l2seg->pd;
        return (void *)&(l2seg_pd->l2seg_fl_lkup_id);
    } else if (hal_handle_get_from_handle_id(ht_entry->handle_id)->obj_id() ==
                                             HAL_OBJ_ID_VRF){
        vrf = (vrf_t *)hal_handle_get_obj(ht_entry->handle_id);
        vrf_pd = (pd_vrf_t *)vrf->pd;
        return (void *)&(vrf_pd->vrf_fl_lkup_id);
    } else {
        // TODO: Remove assert eventually
        SDK_ASSERT(0);
        return NULL;
    }
}

//-----------------------------------------------------------------------------
// key size for flow lkupid hash table
//-----------------------------------------------------------------------------
uint32_t
flow_lkupid_hw_key_size ()
{
    return sizeof(l2seg_hw_id_t);
}

//-----------------------------------------------------------------------------
// get the PI vrf or l2seg given the flow lookup id
//-----------------------------------------------------------------------------
hal_ret_t pd_get_object_from_flow_lkupid(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_get_object_from_flow_lkupid_args_t *args = pd_func_args->pd_get_object_from_flow_lkupid;
    hal_handle_id_ht_entry_t    *entry;
    uint32_t flow_lkupid = args->flow_lkupid;
    hal_obj_id_t *obj_id = args->obj_id;
    void **pi_obj = args->pi_obj;

    *obj_id = HAL_OBJ_ID_NONE;
    *pi_obj = NULL;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state_pd->
        flow_lkupid_ht()->lookup(&flow_lkupid);
    if ((entry == NULL) || (entry->handle_id == HAL_HANDLE_INVALID)) {
        HAL_TRACE_ERR("Unable to find vrf/l2seg for flow_lkupid : {}",
                      flow_lkupid);
        ret = HAL_RET_FLOW_LKUP_ID_NOT_FOUND;
        goto end;
    }
    if (hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                                      HAL_OBJ_ID_L2SEG) {
        *obj_id = HAL_OBJ_ID_L2SEG;
        *pi_obj = hal_handle_get_obj(entry->handle_id);
    } else if (hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                                             HAL_OBJ_ID_VRF){
        *obj_id = HAL_OBJ_ID_VRF;
        *pi_obj = hal_handle_get_obj(entry->handle_id);
    } else {
        // TODO: Remove assert eventually
        SDK_ASSERT(0);
        ret = HAL_RET_ERR;
        goto end;
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// linking PI <-> PD
//----------------------------------------------------------------------------
static void
l2seg_link_pi_pd (pd_l2seg_t *pd_l2seg, l2seg_t *pi_l2seg)
{
    pd_l2seg->l2seg = pi_l2seg;
    pi_l2seg->pd = pd_l2seg;
}

//----------------------------------------------------------------------------
// un-linking PI <-> PD
//----------------------------------------------------------------------------
static void
l2seg_delink_pi_pd (pd_l2seg_t *pd_l2seg, l2seg_t *pi_l2seg)
{
    if (pd_l2seg) {
        pd_l2seg->l2seg = NULL;
    }
    if (pi_l2seg) {
        pi_l2seg->pd = NULL;
    }
}

//------------------------------------------------------------------------------
// insert a l2segment to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
l2seg_pd_add_to_db (pd_l2seg_t *pd_l2seg, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding to l2seg hwid hash table. hwid : {} => ",
                    pd_l2seg->l2seg_fl_lkup_id);

    // allocate an entry to establish mapping from l2seg hwid to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state_pd->flow_lkupid_ht()->insert_with_key(&pd_l2seg->l2seg_fl_lkup_id,
                                                                entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add hw id to handle mapping, "
                      "err : {}", sdk_ret);
        hal::pd::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a l2seg from hwid database
//------------------------------------------------------------------------------
static inline hal_ret_t
l2seg_pd_del_from_db (pd_l2seg_t *pd_l2seg)
{
    hal_handle_id_ht_entry_t    *entry = NULL;

    HAL_TRACE_DEBUG("Deleting from hwid hash table");
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state_pd->flow_lkupid_ht()->
        remove(&pd_l2seg->l2seg_fl_lkup_id);

    if (entry) {
        // free up
        hal::pd::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// PD l2seg create
//------------------------------------------------------------------------------
hal_ret_t
pd_l2seg_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t     ret;
    pd_l2seg_create_args_t *args = pd_func_args->pd_l2seg_create;
    pd_l2seg_t    *l2seg_pd = NULL;

    HAL_TRACE_DEBUG("Creating pd state for l2seg {}", args->l2seg->seg_id);

    // create l2seg PD
    l2seg_pd = l2seg_pd_alloc_init();
    if (l2seg_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI<->PD
    l2seg_link_pi_pd(l2seg_pd, args->l2seg);

    // allocate resources
    ret = l2seg_pd_alloc_res(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to alloc. resources "
                      "for seg_id: {}", args->l2seg->seg_id);
        goto end;
    }

    // Program HW
    ret = l2seg_pd_program_hw(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program hw");
        goto end;
    }

    // Add to DB. Assuming PI calls PD create at the end
    ret = l2seg_pd_add_to_db(l2seg_pd, ((l2seg_t *)(l2seg_pd->l2seg))->hal_handle);
    if (ret != HAL_RET_OK) {
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to create l2seg PD. ret: {}", ret);
        // l2seg_pd_cleanup(l2seg_pd);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// pd l2seg restore from response
// ----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_restore_data (pd_l2seg_restore_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    l2seg_t         *l2seg = args->l2seg;
    pd_l2seg_t      *l2seg_pd = (pd_l2seg_t *)l2seg->pd;

    auto l2seg_info = args->l2seg_status->epd_info();

    l2seg_pd->l2seg_hw_id = l2seg_info.hw_l2seg_id();
    l2seg_pd->l2seg_fl_lkup_id = l2seg_info.l2seg_lookup_id();
    l2seg_pd->cpu_l2seg_id = l2seg_info.l2seg_vlan_id_cpu();
    l2seg_pd->inp_prop_tbl_cpu_idx = l2seg_info.inp_prop_cpu_idx();
    for (int i = 0; i < l2seg_info.inp_prop_idx_size(); i++) {
        l2seg_pd->inp_prop_tbl_idx[i] = l2seg_info.inp_prop_idx(i);
    }
    for (int i = 0; i < l2seg_info.inp_prop_idx_pr_tag_size(); i++) {
        l2seg_pd->inp_prop_tbl_idx_pri[i] = l2seg_info.inp_prop_idx_pr_tag(i);
    }
    return ret;
}

//------------------------------------------------------------------------------
// PD l2seg restore
//------------------------------------------------------------------------------
hal_ret_t
pd_l2seg_restore (pd_func_args_t *pd_func_args)
{
    hal_ret_t     ret;
    pd_l2seg_restore_args_t *args = pd_func_args->pd_l2seg_restore;
    pd_l2seg_t    *l2seg_pd = NULL;

    HAL_TRACE_DEBUG("Restoring pd state for l2seg {}", args->l2seg->seg_id);

    // create l2seg PD
    l2seg_pd = l2seg_pd_alloc_init();
    if (l2seg_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI<->PD
    l2seg_link_pi_pd(l2seg_pd, args->l2seg);

    // Restore data
    ret = pd_l2seg_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore pd data for l2seg: {}, err: {}",
                      args->l2seg->seg_id, ret);
        goto end;
    }

    // Program HW
    ret = l2seg_pd_program_hw(l2seg_pd, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program hw");
        goto end;
    }

    // Add to DB. Assuming PI calls PD create at the end
    ret = l2seg_pd_add_to_db(l2seg_pd, ((l2seg_t *)(l2seg_pd->l2seg))->hal_handle);
    if (ret != HAL_RET_OK) {
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        l2seg_pd_cleanup(l2seg_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD l2seg Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_l2seg_update_args_t *args = pd_func_args->pd_l2seg_update;

    if (args->swm_change) {
        if (l2seg_is_oob_mgmt(args->l2seg)) {
            // Pkts. from OOB:
            // - UUC: Should go to swm uplink. Make clear_promiscuos_en: 0
            ret = l2seg_pd_repgm_mbr_ifs(args->agg_iflist, args->l2seg);
        }
#if 0
        if (args->shared_mgmt_change) {
            // Shared mgmt change 
            if (args->shared_mgmt) {
                // - Unshared -> Shared
                //   - attachment of l2segs, oifls would have happened in PI
                //   - Reprogram input properties
                ret = l2seg_pd_repgm_mbr_ifs(args->agg_iflist, args->l2seg);
            } else {
                // - Shared -> Unshared
                //   - Detachment of oifls would have happened in PI
                //   - Detachment of l2segs
                //   - Reprogram input properties
                ret = l2seg_pd_depgm_ifs_inp_prop_tbl(args->l2seg);
            }
        } else {
            // Update existing IFs & Program newly added IFs. Setting clear_prom.
            ret = l2seg_pd_repgm_mbr_ifs(args->agg_iflist, args->l2seg);
        }
#endif

#if 0
        // Generally swm change will result change of if list as well.
        // Update existing IFs & Program newly added IFs
        ret = l2seg_pd_repgm_mbr_ifs(args->agg_iflist, args->l2seg);

        // Remove deleted IFs
        ret = l2seg_pd_depgm_mbr_ifs(args->del_iflist, args->l2seg);
        // Reprogram ENIC's on this l2seg for promiscous flag change
        ret = l2seg_pd_repgm_enics(args->l2seg);
#endif

    } else if (args->iflist_change) {
        HAL_TRACE_DEBUG("If list change: Input props table change. "
                        "add_oifs: {}, del_oifs: {}", args->add_iflist->num_elems(),
                        args->del_iflist->num_elems());
        ret = l2seg_pd_repgm_mbr_ifs(args->add_iflist, args->l2seg, false);
        ret = l2seg_pd_depgm_mbr_ifs(args->del_iflist, args->l2seg);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to depgm mbr ifs. err: ret:{}", ret);
        }
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD l2seg Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_l2seg_delete_args_t *args = pd_func_args->pd_l2seg_delete;
    pd_l2seg_t     *l2seg_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->l2seg != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->l2seg->pd != NULL), HAL_RET_INVALID_ARG);
    l2seg_pd = (pd_l2seg_t *)args->l2seg->pd;

    // deprogram HW
    ret = l2seg_pd_deprogram_hw(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw");
    }

    // remove from db
    ret = l2seg_pd_del_from_db(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to delete from db");
        goto end;
    }

    // dealloc resources and free
    ret = l2seg_pd_cleanup(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed pd l2seg delete");
    }

end:
    return ret;
}


hal_ret_t
l2seg_pd_depgm_if_inp_prop_tbl(pd_l2seg_t *l2seg_pd, uint32_t uplink_ifpc_id)
{
    hal_ret_t   ret           = HAL_RET_OK;
    sdk_ret_t   sdk_ret       = sdk::SDK_RET_OK;
    sdk_hash    *inp_prop_tbl = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] != INVALID_INDEXER_INDEX) {
        sdk_ret = inp_prop_tbl->remove(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram "
                          "table:input_properties index:{}",
                          l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        } else {
            HAL_TRACE_DEBUG("De-programmed: l2seg:{}, ifpc_id: {}, Inp.Prop.Idx: {}, "
                            "uplink_ifpc_id: {}",
                            l2seg_keyhandle_to_str((l2seg_t *)l2seg_pd->l2seg),
                            uplink_ifpc_id,
                            l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                            uplink_ifpc_id);
        }
        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = INVALID_INDEXER_INDEX;
    }

    if (l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] !=
            INVALID_INDEXER_INDEX) {
        sdk_ret = inp_prop_tbl->
            remove(l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram pri entry "
                          "table:input_properties index:{}",
                          l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        } else {
            HAL_TRACE_DEBUG("De-programmed: l2seg:{}, ifpc_id: {}, Inp.Prop.Idx: {}, "
                            "uplink_ifpc_id: {}",
                            l2seg_keyhandle_to_str((l2seg_t *)l2seg_pd->l2seg),
                            uplink_ifpc_id,
                            l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id],
                            uplink_ifpc_id);
        }
        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = INVALID_INDEXER_INDEX;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram input propterties table
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_depgm_input_properties_tbl (l2seg_t *l2seg, if_t *hal_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint32_t                    uplink_ifpc_id = 0;
    pd_l2seg_t                  *l2seg_pd =  NULL;

    uplink_ifpc_id = if_get_uplink_ifpc_id(hal_if);
    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);

    ret = l2seg_pd_depgm_if_inp_prop_tbl(l2seg_pd, uplink_ifpc_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to de-pgm inp. prop. tabl for "
                      "(l2seg: {}, if: {}). ifpc_id: {}. ret: {}",
                      l2seg->seg_id, hal_if->if_id, uplink_ifpc_id, ret);
    }

    return ret;
}

hal_ret_t
l2seg_pd_depgm_ifs_inp_prop_tbl(l2seg_t *l2seg)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_l2seg_t      *l2seg_pd =  NULL;
    l2seg_t         *shared_l2seg = NULL;
    hal_handle_t    *p_hdl_id = NULL, shared_l2seg_hdl;
    if_t            *uplink_if;
    uint32_t        if_idx = 0;
    vrf_t           *vrf = NULL;
    pd_add_l2seg_uplink_args_t  up_args = {0};


    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);

    if (l2seg_is_shared_mgmt_attached(l2seg)) {
        // Since its attached, we can't remove the entries.
        // Just reprogram shared_mgmt l2seg's without shared mgmt config
        if (l2seg_is_cust(l2seg)) {
            for (const void *ptr : *l2seg->mbrif_list) {
                p_hdl_id = (hal_handle_t *)ptr;
                uplink_if = find_if_by_handle(*p_hdl_id);
                if_idx = uplink_if_get_idx(uplink_if);
                shared_l2seg_hdl = l2seg->other_shared_mgmt_l2seg_hdl[if_idx];
                shared_l2seg = l2seg_lookup_by_handle(shared_l2seg_hdl);
                shared_l2seg->other_shared_mgmt_l2seg_hdl[0] = HAL_HANDLE_INVALID;
                HAL_TRACE_DEBUG("detaching shared mgmt l2seg: l2seg:{} <-> l2seg:{}::if:{}",
                                shared_l2seg->seg_id, l2seg->seg_id, uplink_if->if_id);
                up_args.l2seg = shared_l2seg;
                up_args.intf = uplink_if;
                ret = l2seg_uplink_upd_input_properties_tbl(&up_args, 0, NULL, 0, NULL);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to update input properties for "
                                  "(l2seg:{}, uplink:{}). ret: {}",
                                  shared_l2seg->seg_id, uplink_if->if_id, ret);
                }
                l2seg_pd->inp_prop_tbl_idx[if_idx] = INVALID_INDEXER_INDEX;
                l2seg_pd->inp_prop_tbl_idx_pri[if_idx] = INVALID_INDEXER_INDEX;
            }
        } else {
            vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
            uplink_if = find_if_by_handle(vrf->designated_uplink);
            if_idx = uplink_if_get_idx(uplink_if);
            shared_l2seg_hdl = l2seg->other_shared_mgmt_l2seg_hdl[0];
            shared_l2seg = l2seg_lookup_by_handle(shared_l2seg_hdl);
            shared_l2seg->other_shared_mgmt_l2seg_hdl[if_idx] = HAL_HANDLE_INVALID;
            HAL_TRACE_DEBUG("detaching shared mgmt l2seg: l2seg:{} <-> l2seg:{}::if:{}",
                            l2seg->seg_id, shared_l2seg->seg_id, uplink_if->if_id);
            up_args.l2seg = shared_l2seg;
            up_args.intf = uplink_if;
            ret = l2seg_uplink_upd_input_properties_tbl(&up_args, 0, NULL, 0, NULL);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to update input properties for "
                              "(l2seg:{}, uplink:{}). ret: {}",
                              shared_l2seg->seg_id, uplink_if->if_id, ret);
            }
            l2seg_pd->inp_prop_tbl_idx[if_idx] = INVALID_INDEXER_INDEX;
            l2seg_pd->inp_prop_tbl_idx_pri[if_idx] = INVALID_INDEXER_INDEX;
        }
    } else {
        for (int i = 0; i < HAL_MAX_UPLINK_IF_PCS; i++) {
            ret = l2seg_pd_depgm_if_inp_prop_tbl(l2seg_pd, i);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to depgm ifs inp. prop. tbl. "
                              "ifpc_id: {}. ret: {}", i, ret);
                break;
            }
        }
    }

    return ret;

}

//-----------------------------------------------------------------------------
// De-program member IFs
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_depgm_mbr_ifs (block_list *if_list, l2seg_t *l2seg)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    *p_hdl = NULL;
    if_t            *hal_if = NULL;
    intf::IfType    if_type;
    // vrf_t           *vrf = NULL;

    if (!if_list) goto end;

    for (const void *ptr : *if_list) {
        p_hdl = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl);
        if_type = hal::intf_get_if_type(hal_if);

        // No need of this check. We will depgm if there is an entry created.
#if 0
        if (is_forwarding_mode_classic_nic() || l2seg_is_mgmt(l2seg)) {
            vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
            if (!vrf_if_is_designated_uplink(vrf, hal_if)) {
                continue;
            }
        }
#endif

        HAL_TRACE_DEBUG("Processing if: {}", hal_if->if_id);

        switch(if_type) {
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            ret = l2seg_uplink_depgm_input_properties_tbl(l2seg,
                                                          hal_if);
        break;
#if 0
        case intf::IF_TYPE_LIF:
        break;
#endif
        default:
            SDK_ASSERT(0);
        }
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// deprogram HW
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_deprogram_hw (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t   ret = HAL_RET_OK;
    l2seg_t     *l2seg = (l2seg_t *)l2seg_pd->l2seg, *l2seg_cust = NULL;

    // de-program Input properties Table
    ret = l2seg_pd_depgm_cpu_tx_inp_prop_tbl(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram cpu tx inp. prop. entry. ret: {}",
                      ret);
    }

    if (l2seg_is_mgmt(l2seg)) {
        l2seg_cust = l2seg_pd_get_shared_mgmt_l2seg(l2seg, NULL);
        if (l2seg_cust && l2seg_num_attached_l2segs(l2seg_cust) == 1) {
            HAL_TRACE_DEBUG("l2seg {} is becoming detached. Delete EPs from reg_mac", 
                            l2seg_cust->seg_id);
            ret = l2seg_program_eps_reg_mac(l2seg_cust, TABLE_OPER_REMOVE);
        }
    }

    // de-program member IFs
    ret = l2seg_pd_depgm_ifs_inp_prop_tbl(l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram ifs inp. prop. tbl. ret: {}",
                      ret);
    }


    return ret;
}

//-----------------------------------------------------------------------------
// deprogram input propterties table for cpu tx traffic
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_depgm_cpu_tx_inp_prop_tbl (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t   ret           = HAL_RET_OK;
    sdk_ret_t   sdk_ret       = sdk::SDK_RET_OK;
    sdk_hash    *inp_prop_tbl = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (l2seg_pd->inp_prop_tbl_cpu_idx != INVALID_INDEXER_INDEX) {
        sdk_ret = inp_prop_tbl->remove(l2seg_pd->inp_prop_tbl_cpu_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to deprogram from cpu entry input "
                          "properties at:{}, ret:{}",
                          l2seg_pd->inp_prop_tbl_cpu_idx, ret);
        } else {
            HAL_TRACE_DEBUG("Deprogrammed from_cpu_entry input properties "
                            "at: {}", l2seg_pd->inp_prop_tbl_cpu_idx);
        }
        l2seg_pd->inp_prop_tbl_cpu_idx = INVALID_INDEXER_INDEX;
    }

    return ret;
}

//----------------------------------------------------------------------------
// Form data for input properties table
//----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_inp_prop_form_data (l2seg_t *l2seg, if_t *hal_if,
                                 uint32_t upd_flags,
                                 nwsec_profile_t *nwsec_prof,
                                 uint32_t num_prom_lifs,
                                 if_t *prom_enic_if,
                                 input_properties_actiondata_t &data)
{
    hal_ret_t           ret = HAL_RET_OK;
    l2seg_t             *cl_l2seg, *hp_l2seg;
    // pd_l2seg_t          *l2seg_pd;
    bool                is_native = FALSE;
    types::encapType    enc_type;
    // if_t                *prom_if = NULL;
    // pd_enicif_t         *pd_enicif = NULL;


    // l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);
    is_native = is_l2seg_native(l2seg, hal_if);

    memset(&data, 0, sizeof(data));

    if (l2seg_is_cust(l2seg)) {
        hp_l2seg = l2seg;
        cl_l2seg = l2seg_pd_get_shared_mgmt_l2seg(l2seg, hal_if);
    } else {
        cl_l2seg = l2seg;
        hp_l2seg = l2seg_pd_get_shared_mgmt_l2seg(l2seg, hal_if);
    }

    ret = l2seg_pd_inp_prop_info(cl_l2seg, hp_l2seg, hal_if, upd_flags,
                                 nwsec_prof,
                                 num_prom_lifs,
                                 prom_enic_if,
                                 data);
#if 0
    // check update flags
    if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NWSEC_PROF)) {
        // no change, take from l2seg
        nwsec_prof = (nwsec_profile_t *)l2seg_get_pi_nwsec((l2seg_t *)l2seg);
    }

    if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NUM_PROM_LIFS)) {
        // no change in prom lifs
        num_prom_lifs = l2seg_pd->num_prom_lifs;
    }
#endif

    // Data
    // inp_prop.vrf = l2seg_pd->l2seg_fl_lkup_id;
    inp_prop.dir = FLOW_DIR_FROM_UPLINK;
    // inp_prop.l4_profile_idx = nwsec_prof ?
    //     nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
    // inp_prop.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;
    inp_prop.src_lport = if_get_uplink_lport_id(hal_if);
    // inp_prop.mdest_flow_miss_action = l2seg_get_bcast_fwd_policy(l2seg);
    // inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(l2seg);
    // TODO get info from QOS class
    inp_prop.flow_miss_qos_class_id = 0x2;

#if 0
    // Classic:
    // - All Vlans are in Classic
    // Hostpin:
    // - OOB uplink is in Classic
    // - Other uplinks are in classic for untagged l2seg.
    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC ||
        (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED &&
        l2seg_is_mgmt(l2seg))) {
        // (hal_if->is_oob_management || is_native))) {
        inp_prop.nic_mode = NIC_MODE_CLASSIC;
        if (l2seg->single_wire_mgmt) {
            inp_prop.clear_promiscuous_repl = 0;
        } else {
            if (num_prom_lifs == 0) {
                // No prom. lifs => no promiscuous repl. needed.
                inp_prop.clear_promiscuous_repl = 1;
            } else if (num_prom_lifs == 1) {
                // 1 prom. lif => Get prom lif from inp. props and no prom replication needed
                SDK_ASSERT(l2seg_pd->prom_if_handle != HAL_HANDLE_INVALID);
                inp_prop.clear_promiscuous_repl = 1;
                prom_if = find_if_by_handle(l2seg_pd->prom_if_handle);
                if (prom_if) {
                    inp_prop.dst_lport = if_get_lport_id(prom_if);
                } else {
                    // if lookup by handle is a miss, prom if has to be passed
                    pd_enicif = (pd_enicif_t *)prom_enic_if->pd_if;
                    inp_prop.dst_lport = pd_enicif->enic_lport_id;
                }
            } else {
                // More than 1 prom. lifs => Have to take prom. replication.
                inp_prop.clear_promiscuous_repl = 0;
            }
        }
    } else {
        inp_prop.nic_mode = NIC_MODE_SMART;
    }
#endif

    HAL_TRACE_DEBUG("clear_prom_repl: {}, dst_lport: {}, NIC_MODE: {}, swm: {}",
                    inp_prop.clear_promiscuous_repl,
                    inp_prop.dst_lport, inp_prop.nic_mode,
                    l2seg->single_wire_mgmt);

#if 0
    if ((is_forwarding_mode_host_pinned()) &&
        (hal::l2seg_get_pinned_uplink(l2seg) != hal::if_get_hal_handle(hal_if))) {
        inp_prop.allow_flood = 0;
    } else {
        inp_prop.allow_flood = 1;
    }
#endif

    if (!is_native) {
        enc_type = l2seg_get_wire_encap_type(l2seg);
        if (enc_type == types::ENCAP_TYPE_VXLAN) {
            /* No IPSG check for packets coming in on tunnel interfaces */
            inp_prop.ipsg_enable = FALSE;
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Update data for input properties table
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_upd_input_properties_tbl (pd_add_l2seg_uplink_args_t *args,
                                       uint32_t upd_flags,
                                       nwsec_profile_t *nwsec_prof,
                                       uint32_t num_prom_lifs,
                                       if_t *prom_enic_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    pd_l2seg_t                  *l2seg_pd;
    bool                        is_native = FALSE;
    uint32_t                    uplink_ifpc_id = 0;
    sdk_hash                    *inp_prop_tbl = NULL;
    input_properties_actiondata_t data;

    l2seg_uplink_inp_prop_form_data(args->l2seg, args->intf, upd_flags, nwsec_prof,
                                    num_prom_lifs, prom_enic_if, data);

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);
    is_native = is_l2seg_native(args->l2seg, args->intf);
    uplink_ifpc_id = if_get_uplink_ifpc_id(args->intf);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (!is_native) {
        // Update one entry
        sdk_ret = inp_prop_tbl->update(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id], &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program for "
                          "(l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(args->l2seg),
                          if_get_if_id(args->intf));
            goto end;
        }
        HAL_TRACE_DEBUG("Programmed: {}, If: {}, Inp.Prop.Idx: {}, uplink_ifpc_id: {}",
                        l2seg_keyhandle_to_str(args->l2seg),
                        if_keyhandle_to_str(args->intf),
                        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                        uplink_ifpc_id);
    } else {
        // Update Priority tagged entry
        sdk_ret = inp_prop_tbl->update(
                l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id], &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program "
                          "prio. entry for (l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(args->l2seg),
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed prio. entry "
                            "table:input_properties index:{} ",
                            l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        }

        // Update Untagged entry
        sdk_ret = inp_prop_tbl->update(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id], &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program "
                          "untag entry for (l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(args->l2seg),
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed untagged entry "
                            "table:input_properties index:{} ",
                            l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        }
        HAL_TRACE_DEBUG("Programmed: {}, If: {}, Inp.Prop.Idx: {}, uplink_ifpc_id: {}",
                        l2seg_keyhandle_to_str(args->l2seg),
                        if_keyhandle_to_str(args->intf),
                        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                        uplink_ifpc_id);
    }

end:
        return ret;
}

// ----------------------------------------------------------------------------
// Update pinned uplink for L2seg
// ----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_update_pinned_uplink (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_l2seg_update_pinned_uplink_args_t *args =
        pd_func_args->pd_l2seg_update_pinned_uplink;
    l2seg_t *l2seg = args->l2seg;
    hal_handle_t *p_hdl_id = NULL;
    intf::IfType if_type;
    pd_add_l2seg_uplink_args_t  up_args = {0};
    if_t *hal_if = NULL;

    for (const void *ptr : *l2seg->mbrif_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl_id);
        HAL_TRACE_DEBUG("Reprogramming IF: {}", hal_if->if_id);
        if_type = hal::intf_get_if_type(hal_if);
        if (if_type == intf::IF_TYPE_UPLINK) {
            up_args.l2seg = l2seg;
            up_args.intf = hal_if;
            ret = l2seg_uplink_upd_input_properties_tbl(&up_args, 0, NULL, 0, NULL);
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Program input properties table
// ----------------------------------------------------------------------------
#define inp_prop data.action_u.input_properties_input_properties
hal_ret_t
l2seg_uplink_pgm_input_properties_tbl(l2seg_t *l2seg, if_t *hal_if,
                                      bool is_upgrade)
{
    uint32_t                                uplink_ifpc_id = 0;
    bool                                    is_native = FALSE;
    types::encapType                        enc_type;
    pd_l2seg_t                              *l2seg_pd;
    sdk_hash                                *inp_prop_tbl = NULL;
    hal_ret_t                               ret = HAL_RET_OK;
    sdk_ret_t                               sdk_ret;
    uint32_t                                hash_idx = 0;
    l2seg_t                                 *infra_pi_l2seg = NULL;
    input_properties_swkey_t                key;
    input_properties_otcam_swkey_mask_t     *key_mask = NULL;
    input_properties_actiondata_t           data;
    bool                                    direct_to_otcam = false;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    HAL_TRACE_DEBUG("Programming Input Props table ...");

#if 0
    // Temporary change to use overflow tcam till we figure out on how
    // to avoid using tunnel_vnid and tunnel_type as key in
    // input_properties table for classic_nic mode.
    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC) {
        key_mask = (input_properties_otcam_swkey_mask_t *)HAL_CALLOC(HAL_MEM_ALLOC_INP_PROP_KEY_MASK,
                              sizeof(input_properties_otcam_swkey_mask_t));
        key_mask->capri_intrinsic_lif_mask = 0xFFFF;
        key_mask->vlan_tag_vid_mask = 0xFFFF;
        key_mask->vlan_tag_valid_mask = 0xFF;
        key_mask->entry_inactive_input_properties_mask = 0xFF;
        direct_to_otcam = true;
    }
#endif

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);
    is_native = is_l2seg_native(l2seg, hal_if);
    uplink_ifpc_id = if_get_uplink_ifpc_id(hal_if);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    key.capri_intrinsic_lif = if_get_hw_lif_id(hal_if);
    key.entry_inactive_input_properties = 0;

    // form data
    l2seg_uplink_inp_prop_form_data(l2seg, hal_if, 0, NULL, 0, NULL, data);

    if (!is_native) {
        // Install one entry
        enc_type = l2seg_get_wire_encap_type(l2seg);
        if (enc_type == types::ENCAP_TYPE_DOT1Q) {
            key.vlan_tag_valid = 1;
            key.vlan_tag_vid = l2seg_get_wire_encap_val(l2seg);
        } else if (enc_type == types::ENCAP_TYPE_VXLAN) {
            infra_pi_l2seg = hal::l2seg_get_infra_l2seg();
            if (!infra_pi_l2seg) {
                HAL_TRACE_ERR("FATAL: Trying to program L2seg(Vxlan Enc.) on Uplink "
                              "without Infra L2seg ... skipping");
                ret = HAL_RET_OK;
                goto end;
            }
            if_l2seg_get_encap(hal_if, infra_pi_l2seg, &key.vlan_tag_valid,
                               &key.vlan_tag_vid);
            // TODO: If infra is native on this uplink, do we have to install two entries ?
            key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;
            key.tunnel_metadata_tunnel_vni = l2seg_get_wire_encap_val(l2seg);
            HAL_TRACE_DEBUG("pd-add-l2seg-upif/pc::encap_type vxlan tunnel_vni: {}",
                            key.tunnel_metadata_tunnel_vni);
        } else {
            HAL_TRACE_ERR("FATAL: Wire encap type is None.");
            ret = HAL_RET_OK;
            goto end;
        }

        if (is_upgrade) {
            sdk_ret = inp_prop_tbl->insert_withid(&key, &data,
                                                  l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                                                  key_mask);
        } else {
            // Insert
            sdk_ret = inp_prop_tbl->insert(&key, &data, &hash_idx, key_mask,
                                           direct_to_otcam);
            l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;
        }
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program for "
                          "(l2seg, upif): ({}, {}). err: {}",
                          hal::l2seg_get_l2seg_id(l2seg),
                          if_get_if_id(hal_if), ret);
            goto end;
        } else {
            HAL_TRACE_DEBUG("Programmed "
                            "table:input_properties index:{} ", hash_idx);
        }

        HAL_TRACE_DEBUG("Programmed: {}, If: {}, Inp.Prop.Idx: {}, uplink_ifpc_id: {}",
                        l2seg_keyhandle_to_str(l2seg),
                        if_keyhandle_to_str(hal_if),
                        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                        uplink_ifpc_id);

    } else {
        // Install two entries. 1. Vlan_v:1 & Vlan:0 , 2. Vlan_v:0 & Vlan:0
        // Entry 1:
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = 0;

        if (is_upgrade) {
            sdk_ret = inp_prop_tbl->insert_withid(&key, &data,
                                                  l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id],
                                                  key_mask);
        } else {
            // Insert
            sdk_ret = inp_prop_tbl->insert(&key, &data, &hash_idx,
                                           key_mask, direct_to_otcam);
            // Add to priority array
            l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = hash_idx;
        }
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program "
                          "prio. entry for (l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(l2seg),
                          if_get_if_id(hal_if));
            goto end;
        } else {
            HAL_TRACE_DEBUG("Programmed prio. entry "
                            "table:input_properties index:{} ",
                            hash_idx);
        }

        HAL_TRACE_DEBUG("Programmed: l2seg:{}, If: {}, Inp.Prop.Idx: {}, uplink_ifpc_id: {}",
                        l2seg_keyhandle_to_str(l2seg),
                        if_keyhandle_to_str(hal_if),
                        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id],
                        uplink_ifpc_id);

        // Entry 2:
        key.vlan_tag_valid = 0;
        key.vlan_tag_vid = 0;

        if (is_upgrade) {
            sdk_ret = inp_prop_tbl->insert_withid(&key, &data,
                                                  l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                                                  key_mask);
        } else {
            // Insert
            sdk_ret = inp_prop_tbl->insert(&key, &data, &hash_idx,
                                           key_mask, direct_to_otcam);
            l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;
        }
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program "
                          "untag entry for (l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(l2seg),
                          if_get_if_id(hal_if));
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed untagged entry "
                            "table:input_properties index:{} ",
                            hash_idx);
        }

        HAL_TRACE_DEBUG("Programmed: l2seg:{}, If:{}, Inp.Prop.Idx:{}, uplink_ifpc_id:{}",
                        l2seg_keyhandle_to_str(l2seg),
                        if_keyhandle_to_str(hal_if),
                        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id],
                        uplink_ifpc_id);
    }

    if (key_mask) {
        HAL_FREE(HAL_MEM_ALLOC_INP_PROP_KEY_MASK, key_mask);
    }

end:
    return ret;
}

#if 0
//-----------------------------------------------------------------------------
// program member IFs
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_pgm_mbr_ifs (block_list *if_list, l2seg_t *l2seg, bool is_upgrade)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    *p_hdl = NULL;
    if_t            *hal_if = NULL;
    intf::IfType    if_type;
    vrf_t           *vrf = NULL;

    for (const void *ptr : *if_list) {
        p_hdl = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl);
        if_type = hal::intf_get_if_type(hal_if);

        // Skip for non-designated 
        if ((is_forwarding_mode_classic_nic() || l2seg_is_mgmt(l2seg)) &&
            l2seg->single_wire_mgmt) {
            vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
            if (!vrf_if_is_designated_uplink(vrf, hal_if)) {
                continue;
            }
        }

        switch(if_type) {
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            ret = l2seg_uplink_pgm_input_properties_tbl(l2seg,
                                                        hal_if,
                                                        is_upgrade);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to program input properties for "
                              "(l2seg:{}, uplink:{}). ret: {}",
                              l2seg->seg_id, hal_if->if_id, ret);
                goto end;
            }
        break;
        default:
            SDK_ASSERT(0);
        }
    }

end:
    return ret;
}
#endif

hal_ret_t
l2seg_pd_repgm_enics (l2seg_t *l2seg)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    *p_hdl_id = NULL;
    if_t            *hal_if = NULL;
    intf::IfType    if_type;

    for (const void *ptr : *l2seg->if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl_id);
        if_type = hal::intf_get_if_type(hal_if);
        HAL_TRACE_DEBUG("Reprogramming swm_change for IF: {} type: {}", 
                        if_keyhandle_to_str(hal_if),
                        if_type);
        switch(if_type) {
            case intf::IF_TYPE_ENIC:
                ret = pd_enicif_upd_inp_prop_l2seg(hal_if, l2seg, 0, 0);
                break;
            default:
                // l2seg's if_list should not have anything except enics
                SDK_ASSERT(0);
        }
    }
    return ret;
}


hal_ret_t
l2seg_pd_repgm_mbr_ifs (block_list *agg_list, l2seg_t *l2seg,
                        bool is_upgrade)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    *p_hdl = NULL;
    if_t            *hal_if = NULL;
    intf::IfType    if_type;
    vrf_t           *vrf = NULL;
    uint32_t        uplink_ifpc_id = 0;
    uint32_t        shared_mgmt_idx = 0;
    pd_l2seg_t      *l2seg_pd =  NULL;
    pd_add_l2seg_uplink_args_t  up_args = {0};

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);

    for (const void *ptr : *agg_list) {
        p_hdl = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl);
        if_type = hal::intf_get_if_type(hal_if);

        switch(if_type) {
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:

            // Skip for non-designated 
            if ((is_forwarding_mode_classic_nic() || l2seg_is_mgmt(l2seg)) &&
                l2seg->single_wire_mgmt) {
                vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
                if (!vrf_if_is_designated_uplink(vrf, hal_if)) {
                    HAL_TRACE_DEBUG("Skipping for non-designated uplink: l2seg: {}, if: {}",
                                    l2seg->seg_id, hal_if->if_id);
                    continue;
                }
            }

            uplink_ifpc_id = if_get_uplink_ifpc_id(hal_if);
            // for shared mgmt l2seg, check if entry already exists
            if (l2seg_is_shared_mgmt_attached(l2seg)) {
                // get shared mgmt idx
                if (l2seg_is_cust(l2seg)) {
                    shared_mgmt_idx = uplink_ifpc_id;
                }
                HAL_TRACE_DEBUG("if_idx: {}, sh_mgmt_idx: {}, sh_mgmt_hdl: {}, tbl_idx: {}",
                                uplink_ifpc_id, shared_mgmt_idx,
                                l2seg->other_shared_mgmt_l2seg_hdl[shared_mgmt_idx],
                                l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
                if (l2seg->other_shared_mgmt_l2seg_hdl[shared_mgmt_idx] != HAL_HANDLE_INVALID &&
                    l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] == INVALID_INDEXER_INDEX) {
                    l2seg_pd_copy_inp_prop_tbl_idx(l2seg, 
                                                   l2seg->other_shared_mgmt_l2seg_hdl[shared_mgmt_idx], 
                                                   uplink_ifpc_id);
                }
            }

            if (l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] ==
                INVALID_INDEXER_INDEX) {
                ret = l2seg_uplink_pgm_input_properties_tbl(l2seg,
                                                            hal_if,
                                                            is_upgrade);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to program input properties for "
                                  "(l2seg:{}, uplink:{}). ret: {}",
                                  l2seg->seg_id, hal_if->if_id, ret);
                    goto end;
                }
            } else {
                up_args.l2seg = l2seg;
                up_args.intf = hal_if;
                ret = l2seg_uplink_upd_input_properties_tbl(&up_args, 0, NULL, 0, NULL);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to update input properties for "
                                  "(l2seg:{}, uplink:{}). ret: {}",
                                  l2seg->seg_id, hal_if->if_id, ret);
                    goto end;
                }
            }
        break;
        default:
            SDK_ASSERT(0);
        }
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// program HW
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_program_hw (pd_l2seg_t *l2seg_pd, bool is_upgrade)
{
    l2seg_t     *l2seg = (l2seg_t *)l2seg_pd->l2seg;
    l2seg_t     *l2seg_cust = NULL;
    hal_ret_t   ret;

    // program Input properties Table for CPU TX traffic
    ret = l2seg_pd_pgm_inp_prop_tbl(l2seg_pd, is_upgrade);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program CPU TX entry for L2seg. ret: {}", ret);
        goto end;
    }

    // program member IFs
    ret = l2seg_pd_repgm_mbr_ifs(l2seg->mbrif_list, l2seg, is_upgrade);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program member IFs. ret: {}", ret);
        goto end;
    }

    // L2seg : attached:
    // - Get customer l2seg and program EPs into reg. mac
    if (l2seg_is_mgmt(l2seg)) {
        l2seg_cust = l2seg_pd_get_shared_mgmt_l2seg(l2seg, NULL);
        if (l2seg_cust && l2seg_num_attached_l2segs(l2seg_cust) == 1) {
            HAL_TRACE_DEBUG("l2seg {} is becoming attached. Creating EPs in reg_mac", 
                            l2seg_cust->seg_id);
            ret = l2seg_program_eps_reg_mac(l2seg_cust, TABLE_OPER_INSERT);
        }
    }

end:
    return ret;
}

hal_ret_t
l2seg_program_eps_reg_mac (l2seg_t *l2seg, table_oper_t oper)
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    *p_hdl_id = NULL, *p_hdl = NULL;
    if_t            *hal_if = NULL;
    ep_t            *ep = NULL;
    pd_ep_t         *pd_ep = NULL;

    // walk-thru enics
    for (const void *ptr : *l2seg->if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        hal_if = find_if_by_handle(*p_hdl_id);
        // walk-thru eps
        if (hal_if->if_type == intf::IF_TYPE_ENIC) {
            for (const void *ptr : *(hal_if->ep_list)) {
                p_hdl = (hal_handle_t *)ptr;
                ep = find_ep_by_handle(*p_hdl);
                pd_ep = (pd_ep_t *)ep->pd;
                if (oper == TABLE_OPER_INSERT) {
                    pd_ep_pgm_registered_mac(pd_ep, oper);
                } else {
                    ep_pd_depgm_registered_mac(pd_ep);
                }
            }
        }
    }
    return ret;
}

//-----------------------------------------------------------------------------
// program input propterties table for cpu tx traffic
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_pgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd, bool is_upgrade)
{
    sdk_ret_t                   sdk_ret;
    hal_ret_t                   ret           = HAL_RET_OK;
    sdk_hash                    *inp_prop_tbl = NULL;
    input_properties_swkey_t    key           = { 0 };
    input_properties_actiondata_t data          = { 0 };
    l2seg_t                     *l2seg        = (l2seg_t *)l2seg_pd->l2seg;
    nwsec_profile_t             *nwsec_prof   = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    nwsec_prof = (nwsec_profile_t *)l2seg_get_pi_nwsec((l2seg_t *)l2seg);

    key.capri_intrinsic_lif = HAL_LIF_CPU;
    inp_prop.dir       = FLOW_DIR_FROM_UPLINK;

    key.vlan_tag_valid = 1;
    key.vlan_tag_vid   = l2seg_pd->cpu_l2seg_id;

    inp_prop.vrf              = l2seg_pd->l2seg_fl_lkup_id;
    inp_prop.l4_profile_idx   = nwsec_prof ?
        nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
    inp_prop.ipsg_enable      = 0;
    inp_prop.src_lport        = 0;
    inp_prop.mdest_flow_miss_action = l2seg_get_bcast_fwd_policy((l2seg_t*)
                                                           (l2seg_pd->l2seg));
    inp_prop.flow_miss_idx    = l2seg_get_bcast_oif_list((l2seg_t*)
                                                         (l2seg_pd->l2seg));
    inp_prop.allow_flood      = 1;

    if (is_upgrade) {
        // insert
        sdk_ret = inp_prop_tbl->insert_withid(&key, &data,
                                              l2seg_pd->inp_prop_tbl_cpu_idx);
    } else {
        // sdk_ret = sdk::SDK_RET_NO_RESOURCE;
        sdk_ret = inp_prop_tbl->insert(&key, &data, &l2seg_pd->inp_prop_tbl_cpu_idx);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program from cpu entry input properties "
                      "for ret: {}", ret);
        goto end;
    } else {
        HAL_TRACE_DEBUG("Programmed from_cpu_entry input properties at:{}",
                        l2seg_pd->inp_prop_tbl_cpu_idx);
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// allocating resources
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_alloc_res (pd_l2seg_t *pd_l2seg)
{
    hal_ret_t    ret = HAL_RET_OK;

    ret = l2seg_pd_alloc_cpuid(pd_l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to alloc cpuid");
        goto end;
    }

    ret = l2seg_pd_alloc_hwid(pd_l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to alloc hwid");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        l2seg_pd_dealloc_res(pd_l2seg);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// de-allocating resources
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_dealloc_res (pd_l2seg_t *pd_l2seg)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = l2seg_pd_dealloc_cpuid(pd_l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to dealloc cpuid");
        goto end;
    }

    ret = l2seg_pd_dealloc_hwid(pd_l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to dealloc hwid");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to dealloc res");
    }
    return ret;
}

//-----------------------------------------------------------------------------
// count of number of uplinks which are added to the l2seg
//-----------------------------------------------------------------------------
uint32_t
l2seg_pd_l2seguplink_count (pd_l2seg_t *l2seg_pd)
{
    uint32_t    count  = 0;

    // check if l2seg is brought UP on any uplinks
    for (int i = 0; i < HAL_MAX_UPLINK_IF_PCS; i++) {
        if (l2seg_pd->inp_prop_tbl_idx[i] != INVALID_INDEXER_INDEX ||
                l2seg_pd->inp_prop_tbl_idx_pri[i] != INVALID_INDEXER_INDEX) {
            HAL_TRACE_DEBUG("index {} used tbl_idx : {}, "
                            "tbl_idx_pri : {}",
                            i, l2seg_pd->inp_prop_tbl_idx[i],
                            l2seg_pd->inp_prop_tbl_idx_pri[i]);
            count++;
        }
    }

    return count;
}

//-----------------------------------------------------------------------------
// PD L2seg Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Vrf
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_cleanup (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint32_t        l2seguplink_count = 0;

    if (!l2seg_pd) {
        goto end;
    }

    // check if there are no add_l2seg_on_uplinks referrals.
    // This should never happen as the refs will be checked in PI itself
    l2seguplink_count = l2seg_pd_l2seguplink_count(l2seg_pd);
    SDK_ASSERT_RETURN((l2seguplink_count == 0), HAL_RET_ERR);

    // releasing resources
    ret = l2seg_pd_dealloc_res(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to dealloc res for l2seg: {}",
                      ((l2seg_t *)(l2seg_pd->l2seg))->seg_id);
        goto end;
    }

    // delinking PI<->PD
    l2seg_delink_pi_pd(l2seg_pd, (l2seg_t *)l2seg_pd->l2seg);

    // freeing PD
    l2seg_pd_free(l2seg_pd);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// allocating hwid for l2segment
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_alloc_hwid (pd_l2seg_t *pd_l2seg)
{
    pd_vrf_t     *ten_pd = NULL;
    hal_ret_t    ret = HAL_RET_OK;

    ten_pd = pd_l2seg_get_pd_vrf(pd_l2seg);
    SDK_ASSERT_RETURN(ten_pd != NULL, HAL_RET_VRF_NOT_FOUND);

    ret = vrf_pd_alloc_l2seg_hw_id(ten_pd, (uint32_t *)&pd_l2seg->l2seg_hw_id);
    if (ret != HAL_RET_OK) {
        goto end;
    }
    pd_l2seg->l2seg_fl_lkup_id = ten_pd->vrf_hw_id << HAL_PD_VRF_SHIFT |
                                pd_l2seg->l2seg_hw_id;
    HAL_TRACE_DEBUG("HWIDs l2seg_hwid:{}, vrf_hwid:{}, l2seg_fl_lkup_id:{}",
                    pd_l2seg->l2seg_hw_id,
                    ten_pd->vrf_hw_id, pd_l2seg->l2seg_fl_lkup_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// de-allocate hwid
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_dealloc_hwid(pd_l2seg_t *l2seg_pd)
{
    hal_ret_t    ret = HAL_RET_OK;
    pd_vrf_t     *ten_pd = NULL;

    if (l2seg_pd->l2seg_hw_id != INVALID_INDEXER_INDEX) {
        ten_pd = pd_l2seg_get_pd_vrf(l2seg_pd);
        SDK_ASSERT_RETURN(ten_pd != NULL, HAL_RET_VRF_NOT_FOUND);

        ret = vrf_pd_free_l2seg_hw_id(ten_pd, l2seg_pd->l2seg_hw_id);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        HAL_TRACE_DEBUG("Freed l2seg_hwid: {}", l2seg_pd->l2seg_hw_id);
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// allocating cpuid used as vlan id for traffic coming from cpu
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_alloc_cpuid (pd_l2seg_t *pd_l2seg)
{
    hal_ret_t ret      = HAL_RET_OK;
    indexer::status rs = indexer::SUCCESS;

    // allocate from cpu id
    rs = g_hal_state_pd->l2seg_cpu_idxr()->
             alloc((uint32_t *)&pd_l2seg->cpu_l2seg_id);
    if (rs != indexer::SUCCESS) {
        pd_l2seg->cpu_l2seg_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("Allocated from_cpu_id: {}", pd_l2seg->cpu_l2seg_id);
    return ret;
}

//-----------------------------------------------------------------------------
// de-allocate cpuid
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_dealloc_cpuid (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (l2seg_pd->cpu_l2seg_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->l2seg_cpu_idxr()->free(l2seg_pd->cpu_l2seg_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free cpuid err: {}", l2seg_pd->cpu_l2seg_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }
        HAL_TRACE_DEBUG("freed from_cpu_id: {}", l2seg_pd->cpu_l2seg_id);
        l2seg_pd->cpu_l2seg_id = INVALID_INDEXER_INDEX;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// get l4 prof idx
//-----------------------------------------------------------------------------
uint32_t
pd_l2seg_get_l4_prof_idx (pd_l2seg_t *pd_l2seg)
{
    l2seg_t      *pi_l2seg = NULL;
    vrf_t        *pi_vrf = NULL;

    pi_l2seg = (l2seg_t *)pd_l2seg->l2seg;
    SDK_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_vrf = l2seg_get_pi_vrf(pi_l2seg);
    SDK_ASSERT_RETURN(pi_vrf != NULL, 0);

    return ten_get_nwsec_prof_hw_id(pi_vrf);
}

//-----------------------------------------------------------------------------
// get vrf
//-----------------------------------------------------------------------------
pd_vrf_t *
pd_l2seg_get_pd_vrf (pd_l2seg_t *pd_l2seg)
{
    l2seg_t      *pi_l2seg = NULL;
    vrf_t        *pi_vrf = NULL;

    pi_l2seg = (l2seg_t *)pd_l2seg->l2seg;
    SDK_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_vrf = l2seg_get_pi_vrf(pi_l2seg);
    SDK_ASSERT_RETURN(pi_vrf != NULL, 0);

    return (pd_vrf_t *)pi_vrf->pd;
}

//-----------------------------------------------------------------------------
// makes a clone
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_make_clone (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_l2seg_make_clone_args_t *args = pd_func_args->pd_l2seg_make_clone;
    pd_l2seg_t         *pd_l2seg_clone = NULL;
    l2seg_t            *l2seg, *clone;

    l2seg = args->l2seg;
    clone = args->clone;

    pd_l2seg_clone = l2seg_pd_alloc_init();
    if (pd_l2seg_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_l2seg_clone, l2seg->pd, sizeof(pd_l2seg_t));

    l2seg_link_pi_pd(pd_l2seg_clone, clone);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// get flow lookup id
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_get_flow_lkupid (pd_func_args_t *pd_func_args)
{
    pd_l2seg_get_flow_lkupid_args_t *args = pd_func_args->pd_l2seg_get_flow_lkupid;
    l2seg_t *l2seg = args->l2seg;
    args->hwid = ((pd_l2seg_t *)l2seg->pd)->l2seg_fl_lkup_id;
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// returns the internal vlan of l2seg (used for input_properites lookup of
// reinjected packets) returns false if vlan is not valid
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_get_fromcpu_vlanid (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_l2seg_get_fromcpu_vlanid_args_t *args = pd_func_args->pd_l2seg_get_fromcpu_vlanid;
    l2seg_t     *l2seg = args->l2seg;
    uint16_t    *vid = args->vid;

    if (vid) {
        *vid = ((pd_l2seg_t *)l2seg->pd)->cpu_l2seg_id;
    }
    return ret;
}

// ----------------------------------------------------------------------------
// pd l2seg get
// ----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_l2seg_get_args_t *args = pd_func_args->pd_l2seg_get;
    l2seg_t                 *l2seg = args->l2seg;
    pd_l2seg_t              *l2seg_pd = (pd_l2seg_t *)l2seg->pd;
    L2SegmentGetResponse    *rsp = args->rsp;

    auto l2seg_info = rsp->mutable_status()->mutable_epd_info();
    l2seg_info->set_hw_l2seg_id(l2seg_pd->l2seg_hw_id);
    l2seg_info->set_l2seg_lookup_id(l2seg_pd->l2seg_fl_lkup_id);
    l2seg_info->set_l2seg_vlan_id_cpu(l2seg_pd->cpu_l2seg_id);
    l2seg_info->set_inp_prop_cpu_idx(l2seg_pd->inp_prop_tbl_cpu_idx);
    for (int i = 0; i < HAL_MAX_UPLINK_IF_PCS; i++) {
        if (l2seg_pd->inp_prop_tbl_idx[i] != INVALID_INDEXER_INDEX) {
            l2seg_info->add_inp_prop_idx(l2seg_pd->inp_prop_tbl_idx[i]);
        } else {
            l2seg_info->add_inp_prop_idx(0);
        }
        if (l2seg_pd->inp_prop_tbl_idx_pri[i] != INVALID_INDEXER_INDEX) {
            l2seg_info->add_inp_prop_idx_pr_tag(l2seg_pd->inp_prop_tbl_idx_pri[i]);
        } else {
            l2seg_info->add_inp_prop_idx_pr_tag(0);
        }
    }

    return ret;
}

//----------------------------------------------------------------------------
// frees PD memory without indexer free.
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_mem_free (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_l2seg_mem_free_args_t *args = pd_func_args->pd_l2seg_mem_free;
    pd_l2seg_t     *l2seg_pd;

    l2seg_pd = (pd_l2seg_t *)args->l2seg->pd;
    l2seg_pd_mem_free(l2seg_pd);

    return ret;
}

hal_ret_t
pd_l2seg_update_prom_lifs(pd_l2seg_t *pd_l2seg,
                          if_t *prom_enic_if,
                          bool inc,
                          bool skip_hw_pgm)
{
    hal_ret_t                   ret = HAL_RET_OK;
    bool                        trigger_inp_prp_pgm = false;
    l2seg_t                     *l2seg = (l2seg_t *)pd_l2seg->l2seg;
    if_t                        *hal_if = NULL, *eff_prom_enic_if = NULL;
    hal_handle_t                *p_hdl_id = NULL;
    intf::IfType                if_type;
    pd_add_l2seg_uplink_args_t  up_args = {0};
    lif_t                       *lif = NULL;
    pd_enicif_t                 *pd_enicif = (pd_enicif_t *)prom_enic_if->pd_if;

    if (inc) {
        pd_l2seg->num_prom_lifs++;
        if (pd_l2seg->num_prom_lifs == 1) {
            pd_l2seg->prom_if_handle = prom_enic_if->hal_handle;
            pd_l2seg->prom_if_dest_lport = pd_enicif->enic_lport_id;
            eff_prom_enic_if = prom_enic_if;
            trigger_inp_prp_pgm = true;
        } else if (pd_l2seg->num_prom_lifs == 2) {
            pd_l2seg->prom_if_handle = HAL_HANDLE_INVALID;
            pd_l2seg->prom_if_dest_lport = 0;
            // pd_l2seg->prom_if_dest_lport = pd_enicif->enic_lport_id;
            trigger_inp_prp_pgm = true;
        }
        HAL_TRACE_DEBUG("L2seg id: {}, Incrementing prom lifs to: {}, "
                        "trig_inp_prop: {}",
                        l2seg->seg_id, pd_l2seg->num_prom_lifs,
                        trigger_inp_prp_pgm);
    } else {
        pd_l2seg->num_prom_lifs--;
        if (pd_l2seg->num_prom_lifs == 1) {
            // Walk l2seg's enics and pick up the one enic which has prom. lif
            for (const void *ptr : *l2seg->if_list) {
                p_hdl_id = (hal_handle_t *)ptr;
                hal_if = find_if_by_handle(*p_hdl_id);
                if_type = hal::intf_get_if_type(hal_if);
                if (if_type == intf::IF_TYPE_ENIC) {
                    lif = if_get_lif(hal_if);
                    if (lif->packet_filters.receive_promiscuous) {
                        eff_prom_enic_if = hal_if;

                    }
                }
            }
            SDK_ASSERT(eff_prom_enic_if != NULL);
            pd_enicif = (pd_enicif_t *)eff_prom_enic_if->pd_if;
            pd_l2seg->prom_if_handle = eff_prom_enic_if->hal_handle;
            pd_l2seg->prom_if_dest_lport = pd_enicif->enic_lport_id;
            trigger_inp_prp_pgm = true;
        } else if (pd_l2seg->num_prom_lifs == 0) {
            pd_l2seg->prom_if_handle = HAL_HANDLE_INVALID;
            pd_l2seg->prom_if_dest_lport = 0;
            // pd_l2seg->prom_if_dest_lport = pd_enicif->enic_lport_id;
            trigger_inp_prp_pgm = true;
        }
        HAL_TRACE_DEBUG("L2seg id: {}, Decrementing prom lifs to: {}, "
                        "trig_inp_prop: {}",
                        l2seg->seg_id, pd_l2seg->num_prom_lifs,
                        trigger_inp_prp_pgm);
    }

    if (trigger_inp_prp_pgm && !skip_hw_pgm) {
        // Will be deprecated when we move classic enics to l2seg
        for (const void *ptr : *l2seg->if_list) {
            p_hdl_id = (hal_handle_t *)ptr;
            HAL_TRACE_DEBUG("Processing IF: {}", *p_hdl_id);
            hal_if = find_if_by_handle(*p_hdl_id);
            if_type = hal::intf_get_if_type(hal_if);
            HAL_TRACE_DEBUG("Processing IF: {}, type: {}", *p_hdl_id, if_type);
            switch(if_type) {
            case intf::IF_TYPE_ENIC:
                ret = pd_enicif_upd_inp_prop_l2seg(hal_if, l2seg,
                                                   ENICIF_UPD_FLAGS_NUM_PROM_LIFS,
                                                   pd_l2seg->num_prom_lifs);
                break;
            case intf::IF_TYPE_UPLINK:
                // SDK_ASSERT(0);
                up_args.l2seg = l2seg;
                up_args.intf = hal_if;
                ret = l2seg_uplink_upd_input_properties_tbl(&up_args,
                                                            L2SEG_UPLINK_UPD_FLAGS_NUM_PROM_LIFS,
                                                            NULL,
                                                            pd_l2seg->num_prom_lifs,
                                                            eff_prom_enic_if);
                break;
            case intf::IF_TYPE_UPLINK_PC:
                // Handle Uplink PCs
                break;
            default:
                SDK_ASSERT(0);
            }
        }
        for (const void *ptr : *l2seg->mbrif_list) {
            p_hdl_id = (hal_handle_t *)ptr;
            HAL_TRACE_DEBUG("Processing IF: {}", *p_hdl_id);
            hal_if = find_if_by_handle(*p_hdl_id);
            if_type = hal::intf_get_if_type(hal_if);
            HAL_TRACE_DEBUG("Processing IF: {}, type: {}", *p_hdl_id, if_type);
            switch(if_type) {
            case intf::IF_TYPE_ENIC:
                SDK_ASSERT(0);
                ret = pd_enicif_upd_inp_prop_l2seg(hal_if, l2seg,
                                                   ENICIF_UPD_FLAGS_NUM_PROM_LIFS,
                                                   pd_l2seg->num_prom_lifs);
                break;
            case intf::IF_TYPE_UPLINK:
                up_args.l2seg = l2seg;
                up_args.intf = hal_if;
                ret = l2seg_uplink_upd_input_properties_tbl(&up_args,
                                                            L2SEG_UPLINK_UPD_FLAGS_NUM_PROM_LIFS,
                                                            NULL,
                                                            pd_l2seg->num_prom_lifs,
                                                            eff_prom_enic_if);
                break;
            case intf::IF_TYPE_UPLINK_PC:
                // Handle Uplink PCs
                break;
            default:
                SDK_ASSERT(0);
            }
        }
    }


    return ret;
}

hal_ret_t
l2seg_pd_copy_inp_prop_tbl_idx (l2seg_t *l2seg, 
                                hal_handle_t other_l2seg_hdl,
                                uint32_t up_ifpc_id)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_l2seg_t  *other_l2seg_pd = NULL, *l2seg_pd = NULL;;
    l2seg_t *other_l2seg = l2seg_lookup_by_handle(other_l2seg_hdl);

    if (other_l2seg == NULL) {
        ret = HAL_RET_ERR;
        goto end;
    }

    other_l2seg_pd = (pd_l2seg_t *)other_l2seg->pd;
    l2seg_pd = (pd_l2seg_t *)l2seg->pd;

    l2seg_pd->inp_prop_tbl_idx[up_ifpc_id] = 
        other_l2seg_pd->inp_prop_tbl_idx[up_ifpc_id];
    l2seg_pd->inp_prop_tbl_idx_pri[up_ifpc_id] = 
        other_l2seg_pd->inp_prop_tbl_idx_pri[up_ifpc_id];

end:
    return ret;
}

l2seg_t *
l2seg_pd_get_shared_mgmt_l2seg (l2seg_t *l2seg, 
                                if_t *hal_if)
{
    uint32_t        uplink_ifpc_id = 0;
    l2seg_t         *shared_mgmt_l2seg = NULL;
    hal_handle_t    shared_mgmt_l2seg_hdl;

    uplink_ifpc_id = if_get_uplink_ifpc_id(hal_if);
    if (l2seg_is_shared_mgmt_attached(l2seg)) {
        if (l2seg_is_cust(l2seg)) {
            shared_mgmt_l2seg_hdl = l2seg->other_shared_mgmt_l2seg_hdl[uplink_ifpc_id];
        } else {
            shared_mgmt_l2seg_hdl = l2seg->other_shared_mgmt_l2seg_hdl[0];
        }
        shared_mgmt_l2seg = l2seg_lookup_by_handle(shared_mgmt_l2seg_hdl);
        return shared_mgmt_l2seg;
    }
    return NULL;
}

uint8_t
get_nic_mode (l2seg_t *l2seg)
{
    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC ||
        (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED &&
        l2seg_is_mgmt(l2seg))) {
        return NIC_MODE_CLASSIC;
    } else {
        return NIC_MODE_SMART;
    }
}

void
get_clear_prom_repl (l2seg_t *l2seg, uint32_t num_prom_lifs,
                     if_t *prom_enic_if, 
                     uint8_t *clear_prom, uint16_t *dst_lport)
{
    pd_l2seg_t          *l2seg_pd;
    if_t                *prom_if = NULL;
    pd_enicif_t         *pd_enicif = NULL;

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);
    *clear_prom = 0;
    *dst_lport = 0;

    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC ||
        (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED &&
        l2seg_is_mgmt(l2seg))) {
        if (l2seg->single_wire_mgmt && l2seg_is_oob_mgmt(l2seg)) {
            *clear_prom = 0;
        } else {
            if (num_prom_lifs == 0) {
                // No prom. lifs => no promiscuous repl. needed.
                *clear_prom = 1;
            } else if (num_prom_lifs == 1) {
                // 1 prom. lif => Get prom lif from inp. props and no prom replication needed
                *clear_prom = 1;
                prom_if = find_if_by_handle(l2seg_pd->prom_if_handle);
                if (prom_if) {
                    *dst_lport = if_get_lport_id(prom_if);
                } else {
                    // if lookup by handle is a miss, prom if has to be passed
                    pd_enicif = (pd_enicif_t *)prom_enic_if->pd_if;
                    *dst_lport = pd_enicif->enic_lport_id;
                }
            } else {
                // More than 1 prom. lifs => Have to take prom. replication.
                *clear_prom = 0;
            }
        }
    }
}


hal_ret_t
l2seg_pd_inp_prop_info(l2seg_t *cl_l2seg, l2seg_t *hp_l2seg, if_t *hal_if, 
                       uint32_t upd_flags,
                       nwsec_profile_t *nwsec_prof,
                       uint32_t num_prom_lifs,
                       if_t *prom_enic_if,
                       input_properties_actiondata_t &data) 
{
    pd_l2seg_t *cl_l2seg_pd, *hp_l2seg_pd;

    cl_l2seg_pd = cl_l2seg ? (pd_l2seg_t *)cl_l2seg->pd : NULL;
    hp_l2seg_pd = hp_l2seg ? (pd_l2seg_t *)hp_l2seg->pd : NULL;

    if (cl_l2seg && hp_l2seg) {
        inp_prop.nic_mode = NIC_MODE_SMART;
        inp_prop.mode_switch_en = 1;
        inp_prop.vrf = hp_l2seg_pd->l2seg_fl_lkup_id;
        inp_prop.classic_vrf = cl_l2seg_pd->l2seg_fl_lkup_id;
        inp_prop.mdest_flow_miss_action = l2seg_get_bcast_fwd_policy(hp_l2seg);
        inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(cl_l2seg);
        if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NWSEC_PROF)) {
            // no change, take from l2seg
            nwsec_prof = (nwsec_profile_t *)l2seg_get_pi_nwsec((l2seg_t *)hp_l2seg);
        }
        if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NUM_PROM_LIFS)) {
            // no change in prom lifs
            num_prom_lifs = cl_l2seg_pd->num_prom_lifs;
        }
        get_clear_prom_repl(cl_l2seg, num_prom_lifs, prom_enic_if, 
                            &inp_prop.clear_promiscuous_repl,
                            &inp_prop.dst_lport);
        if (hal::l2seg_get_pinned_uplink(hp_l2seg) != hal::if_get_hal_handle(hal_if)) {
            inp_prop.allow_flood = 0;
        } else {
            inp_prop.allow_flood = 1;
        }
    } else if (cl_l2seg) {
        inp_prop.nic_mode = get_nic_mode(cl_l2seg);
        inp_prop.vrf = cl_l2seg_pd->l2seg_fl_lkup_id;
        inp_prop.classic_vrf = cl_l2seg_pd->l2seg_fl_lkup_id;
        inp_prop.mdest_flow_miss_action = l2seg_get_bcast_fwd_policy(cl_l2seg);
        inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(cl_l2seg);
        if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NWSEC_PROF)) {
            // no change, take from l2seg
            nwsec_prof = (nwsec_profile_t *)l2seg_get_pi_nwsec((l2seg_t *)cl_l2seg);
        }
        if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NUM_PROM_LIFS)) {
            // no change in prom lifs
            num_prom_lifs = cl_l2seg_pd->num_prom_lifs;
        }
        get_clear_prom_repl(cl_l2seg, num_prom_lifs, prom_enic_if, 
                            &inp_prop.clear_promiscuous_repl,
                            &inp_prop.dst_lport);
        inp_prop.allow_flood = 1;
    } else {
        inp_prop.nic_mode = get_nic_mode(hp_l2seg);
        inp_prop.vrf = hp_l2seg_pd->l2seg_fl_lkup_id;
        inp_prop.classic_vrf = 0;
        inp_prop.mdest_flow_miss_action = l2seg_get_bcast_fwd_policy(hp_l2seg);
        inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(hp_l2seg);
        if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NWSEC_PROF)) {
            // no change, take from l2seg
            nwsec_prof = (nwsec_profile_t *)l2seg_get_pi_nwsec((l2seg_t *)hp_l2seg);
        }
        if (!(upd_flags & L2SEG_UPLINK_UPD_FLAGS_NUM_PROM_LIFS)) {
            // no change in prom lifs
            num_prom_lifs = hp_l2seg_pd->num_prom_lifs;
        }
        get_clear_prom_repl(hp_l2seg, num_prom_lifs, prom_enic_if, 
                            &inp_prop.clear_promiscuous_repl,
                            &inp_prop.dst_lport);
        if (hal::l2seg_get_pinned_uplink(hp_l2seg) != hal::if_get_hal_handle(hal_if)) {
            inp_prop.allow_flood = 0;
        } else {
            inp_prop.allow_flood = 1;
        }
    }
    inp_prop.l4_profile_idx = nwsec_prof ?
        nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
    inp_prop.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;

    return HAL_RET_OK;
}





}    // namespace pd
}    // namespace hal
