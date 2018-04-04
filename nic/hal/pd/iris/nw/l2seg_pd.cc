// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/src/internal/proxy.hpp"

namespace hal {
namespace pd {

#define inp_prop data.input_properties_action_u.input_properties_input_properties

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
hal_ret_t l2seg_pd_program_hw(pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_pd_pgm_inp_prop_tbl(pd_l2seg_t *l2seg_pd);
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

    HAL_ASSERT(entry != NULL);
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
        HAL_ASSERT(0);
        return NULL;
    }
}

//-----------------------------------------------------------------------------
// hash function for flow lkupid hash table
//-----------------------------------------------------------------------------
uint32_t
flow_lkupid_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(l2seg_hw_id_t)) % ht_size;
}

//-----------------------------------------------------------------------------
// compare key function for flow lkupid hash table
//-----------------------------------------------------------------------------
bool
flow_lkupid_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(uint32_t *)key1 == *(uint32_t *)key2) {
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// get the PI vrf or l2seg given the flow lookup id
//-----------------------------------------------------------------------------
hal_ret_t pd_get_object_from_flow_lkupid(pd_get_object_from_flow_lkupid_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
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
        HAL_ASSERT(0);
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
pd_l2seg_create (pd_l2seg_create_args_t *args)
{
    hal_ret_t     ret;
    pd_l2seg_t    *l2seg_pd = NULL;

    HAL_TRACE_DEBUG("Creating pd state for l2seg", args->l2seg->seg_id);

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
        l2seg_pd_cleanup(l2seg_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD l2seg Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_update (pd_l2seg_update_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD l2seg Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_delete (pd_l2seg_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_l2seg_t     *l2seg_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->l2seg != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->l2seg->pd != NULL), HAL_RET_INVALID_ARG);
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

//-----------------------------------------------------------------------------
// deprogram HW
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_deprogram_hw (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program Input properties Table
    ret = l2seg_pd_depgm_inp_prop_tbl(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// deprogram input propterties table for cpu tx traffic
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_depgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t   ret           = HAL_RET_OK;
    sdk_ret_t   sdk_ret       = sdk::SDK_RET_OK;
    sdk_hash    *inp_prop_tbl = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

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

//-----------------------------------------------------------------------------
// program HW
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_program_hw (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t            ret;

    // program Input properties Table
    ret = l2seg_pd_pgm_inp_prop_tbl(l2seg_pd);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

//-----------------------------------------------------------------------------
// program input propterties table for cpu tx traffic
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_pgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd)
{
    sdk_ret_t                   sdk_ret;
    hal_ret_t                   ret           = HAL_RET_OK;
    sdk_hash                    *inp_prop_tbl = NULL;
    input_properties_swkey_t    key           = { 0 };
    input_properties_actiondata data          = { 0 };
    vrf_t                       *vrf;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    key.capri_intrinsic_lif = SERVICE_LIF_CPU;
    vrf = l2seg_get_pi_vrf((l2seg_t *)l2seg_pd->l2seg);
    if (vrf->vrf_type == types::VRF_TYPE_INFRA) {
        key.vlan_tag_valid = 0;
        key.vlan_tag_vid   = 0;
        inp_prop.dir       = FLOW_DIR_FROM_ENIC;
    } else {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid   = l2seg_pd->cpu_l2seg_id;
        inp_prop.dir       = FLOW_DIR_FROM_UPLINK;
    }

    inp_prop.vrf              = l2seg_pd->l2seg_fl_lkup_id;
    inp_prop.l4_profile_idx   = 0;
    inp_prop.ipsg_enable      = 0;
    inp_prop.src_lport        = 0;
    inp_prop.flow_miss_action = l2seg_get_bcast_fwd_policy((l2seg_t*)
                                                           (l2seg_pd->l2seg));
    inp_prop.flow_miss_idx    = l2seg_get_bcast_oif_list((l2seg_t*)
                                                         (l2seg_pd->l2seg));
    inp_prop.allow_flood      = 1;

    sdk_ret = inp_prop_tbl->insert(&key, &data, &l2seg_pd->inp_prop_tbl_cpu_idx);
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
    HAL_ASSERT_RETURN((l2seguplink_count == 0), HAL_RET_ERR);

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
    HAL_ASSERT_RETURN(ten_pd != NULL, HAL_RET_VRF_NOT_FOUND);

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
        HAL_ASSERT_RETURN(ten_pd != NULL, HAL_RET_VRF_NOT_FOUND);

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
    HAL_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_vrf = l2seg_get_pi_vrf(pi_l2seg);
    HAL_ASSERT_RETURN(pi_vrf != NULL, 0);

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
    HAL_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_vrf = l2seg_get_pi_vrf(pi_l2seg);
    HAL_ASSERT_RETURN(pi_vrf != NULL, 0);

    return (pd_vrf_t *)pi_vrf->pd;
}

//-----------------------------------------------------------------------------
// makes a clone
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_make_clone (pd_l2seg_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
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
pd_l2seg_get_flow_lkupid (pd_l2seg_get_flow_lkupid_args_t *args)
{
    l2seg_t *l2seg = args->l2seg;
    args->hwid = ((pd_l2seg_t *)l2seg->pd)->l2seg_fl_lkup_id;
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// returns the internal vlan of l2seg (used for input_properites lookup of
// reinjected packets) returns false if vlan is not valid
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_get_fromcpu_vlanid (pd_l2seg_get_fromcpu_vlanid_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    l2seg_t     *l2seg = args->l2seg;
    uint16_t    *vid = args->vid;
    vrf_t       *vrf;

    vrf = l2seg_get_pi_vrf(l2seg);
    if (vrf->vrf_type == types::VRF_TYPE_INFRA) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (vid) {
        *vid = ((pd_l2seg_t *)l2seg->pd)->cpu_l2seg_id;
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// frees PD memory without indexer free.
//-----------------------------------------------------------------------------
hal_ret_t
pd_l2seg_mem_free (pd_l2seg_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_l2seg_t     *l2seg_pd;

    l2seg_pd = (pd_l2seg_t *)args->l2seg->pd;
    l2seg_pd_mem_free(l2seg_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
