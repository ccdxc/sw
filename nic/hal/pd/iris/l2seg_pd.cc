#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/l2seg_pd.hpp"
#include "nic/hal/pd/iris/vrf_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/src/proxy.hpp"

namespace hal {
namespace pd {

#define inp_prop data.input_properties_action_u.input_properties_input_properties
void *
flow_lkupid_get_hw_key_func(void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry = NULL;
    l2seg_t                  *l2seg    = NULL;
    pd_l2seg_t               *l2seg_pd = NULL;
    vrf_t                    *vrf      = NULL;
    pd_vrf_t                 *vrf_pd   = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
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

uint32_t
flow_lkupid_compute_hw_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(l2seg_hw_id_t)) % ht_size;
}

bool
flow_lkupid_compare_hw_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(uint32_t *)key1 == *(uint32_t *)key2) {
        return true;
    }
    return false;
}

// Deprecated: Remove it once FTE uses new API
extern "C" hal_ret_t
pd_find_l2seg_by_hwid (pd_find_l2seg_by_hwid_args_t *args)
{
    pd_l2seg_t *l2seg_pd = find_l2seg_pd_by_hwid(args->hwid);
    args->l2seg =  l2seg_pd ? (l2seg_t*) l2seg_pd->l2seg : NULL;


    return HAL_RET_OK;
}
#if 0
l2seg_t *
find_l2seg_by_hwid (l2seg_hw_id_t hwid)
{
    pd_l2seg_t *l2seg_pd = find_l2seg_pd_by_hwid(hwid);
    return l2seg_pd ? (l2seg_t*) l2seg_pd->l2seg : NULL;
}
#endif
 
//-----------------------------------------------------------------------------
// Get the PI vrf or l2seg given the flow lookup id
//-----------------------------------------------------------------------------
#if 0
hal_ret_t pd_get_object_from_flow_lkupid(uint32_t flow_lkupid, 
                                         hal_obj_id_t *obj_id,
                                         void **pi_obj)
#endif
extern "C" hal_ret_t pd_get_object_from_flow_lkupid(pd_get_object_from_flow_lkupid_args_t *args)
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
    if (entry == NULL) {
        HAL_TRACE_ERR("{}:unable to find vrf/l2seg for flow_lkupid:{}",
                      __FUNCTION__, flow_lkupid);
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
// Linking PI <-> PD
//----------------------------------------------------------------------------
static void
l2seg_link_pi_pd (pd_l2seg_t *pd_l2seg, l2seg_t *pi_l2seg)
{
    pd_l2seg->l2seg = pi_l2seg;
    pi_l2seg->pd = pd_l2seg;
}

//----------------------------------------------------------------------------
// Un-Linking PI <-> PD
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

    HAL_TRACE_DEBUG("pd-l2seg:{}:adding to l2seg hwid hash table. hwid:{} => ",
                    __FUNCTION__, pd_l2seg->l2seg_fl_lkup_id);

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
        HAL_TRACE_ERR("pd-l2seg:{}:failed to add hw id to handle mapping, "
                      "err : {}", __FUNCTION__, sdk_ret);
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

    HAL_TRACE_DEBUG("pd-l2seg:{}:removing from hwid hash table", __FUNCTION__);
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
// pd l2seg create
//------------------------------------------------------------------------------
extern "C" hal_ret_t
pd_l2seg_create (pd_l2seg_create_args_t *args)
{
    hal_ret_t     ret;
    pd_l2seg_t    *l2seg_pd = NULL;

    HAL_TRACE_DEBUG("pd-l2seg:{}:creating pd state for l2seg", 
                    __FUNCTION__, args->l2seg->seg_id);

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
        HAL_TRACE_ERR("pd-l2seg:{}: Unable to alloc. resources "
                "for seg_id: {}", __FUNCTION__, args->l2seg->seg_id);
        goto end;
    }

    // Program HW
    ret = l2seg_pd_program_hw(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to program hw", __FUNCTION__);
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
extern "C" hal_ret_t
pd_l2seg_update (pd_l2seg_update_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD l2seg Delete
//-----------------------------------------------------------------------------
extern "C" hal_ret_t
pd_l2seg_delete (pd_l2seg_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_l2seg_t     *l2seg_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->l2seg != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->l2seg->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-l2seg:{}:deleting pd state for l2seg {}",
                    __FUNCTION__, args->l2seg->seg_id);
    l2seg_pd = (pd_l2seg_t *)args->l2seg->pd;

    // deprogram HW
    ret = l2seg_pd_deprogram_hw(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:unable to deprogram hw", __FUNCTION__);
    }

    // remove from db
    ret = l2seg_pd_del_from_db(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:unable to delete from db", __FUNCTION__);
        goto end;
    }

    // dealloc resources and free
    ret = l2seg_pd_cleanup(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd l2seg delete");
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_deprogram_hw (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program Input properties Table
    ret = l2seg_pd_depgm_inp_prop_tbl(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:unable to deprogram hw", __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram input propterties table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_depgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t   ret           = HAL_RET_OK;
    sdk_ret_t   sdk_ret;
    sdk_hash    *inp_prop_tbl = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);
    
    sdk_ret = inp_prop_tbl->remove(l2seg_pd->inp_prop_tbl_cpu_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg::{}:unable to deprogram from cpu entry "
                "input properties for seg_id:{}",
                __FUNCTION__, ((l2seg_t*)(l2seg_pd->l2seg))->seg_id);
    } else {
        HAL_TRACE_DEBUG("pd-l2seg::{}:deprogrammed from_cpu_entry "
                "input properties for seg_id:{}",
                __FUNCTION__, ((l2seg_t*)(l2seg_pd->l2seg))->seg_id);
    }

    return ret;
}



// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_program_hw (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t            ret;

    // Program Input properties Table
    ret = l2seg_pd_pgm_inp_prop_tbl(l2seg_pd);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

// ----------------------------------------------------------------------------
// Program input propterties table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_pgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t                   ret           = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *inp_prop_tbl = NULL;
    input_properties_swkey_t    key           = { 0 };
    input_properties_actiondata data          = { 0 };

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    key.capri_intrinsic_lif = SERVICE_LIF_CPU;
    if (((l2seg_t *)(l2seg_pd->l2seg))->segment_type == types::L2_SEGMENT_TYPE_INFRA) {
        key.vlan_tag_valid = 0;
        key.vlan_tag_vid   = 0;
        inp_prop.dir       = FLOW_DIR_FROM_ENIC;
    } else {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid   = l2seg_pd->l2seg_fromcpu_id;
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

    // Insert
    sdk_ret = inp_prop_tbl->insert(&key, &data, &l2seg_pd->inp_prop_tbl_cpu_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg::{}: Unable to program from cpu entry "
                      "input properties for seg_id:{}",
                      __FUNCTION__, ((l2seg_t*)(l2seg_pd->l2seg))->seg_id);
        goto end;
    } else {
        HAL_TRACE_DEBUG("pd-l2seg::{}: Programmed from_cpu_entry "
                        "input properties for seg_id:{}",
                        __FUNCTION__, ((l2seg_t*)(l2seg_pd->l2seg))->seg_id);
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Allocating resources
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_alloc_res (pd_l2seg_t *pd_l2seg)
{
    hal_ret_t    ret = HAL_RET_OK;

    ret = l2seg_pd_alloc_cpuid(pd_l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to alloc cpuid", __FUNCTION__);
        goto end;
    }

    ret = l2seg_pd_alloc_hwid(pd_l2seg); 
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to alloc hwid", __FUNCTION__);
        goto end;
    }

end:

    if (ret != HAL_RET_OK) {
        l2seg_pd_dealloc_res(pd_l2seg);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// De-allocating resources
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_dealloc_res (pd_l2seg_t *pd_l2seg)
{
    hal_ret_t           ret = HAL_RET_OK;

    ret = l2seg_pd_dealloc_cpuid(pd_l2seg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to dealloc cpuid", __FUNCTION__);
        goto end;
    }

    ret = l2seg_pd_dealloc_hwid(pd_l2seg); 
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to dealloc hwid", __FUNCTION__);
        goto end;
    }

end:

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to dealloc res", __FUNCTION__);
    }
    return ret;
}

uint32_t
l2seg_pd_l2seguplink_count (pd_l2seg_t *l2seg_pd)
{
    uint32_t    count  = 0;

    for (int i = 0; i < HAL_MAX_UPLINK_IF_PCS; i++) {
        if (l2seg_pd->inp_prop_tbl_idx[i] != INVALID_INDEXER_INDEX ||
                l2seg_pd->inp_prop_tbl_idx_pri[i] != INVALID_INDEXER_INDEX) {
            HAL_TRACE_DEBUG("pd-l2seg:{}:index {} used tbl_idx:{}, "
                            "tbl_idx_pri:{}", 
                            __FUNCTION__, i, l2seg_pd->inp_prop_tbl_idx[i],
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
        // Nothing to do
        goto end;
    }

    // check if there are no add_l2seg_on_uplinks referrals.
    l2seguplink_count = l2seg_pd_l2seguplink_count(l2seg_pd);
    if (l2seguplink_count) {
        HAL_TRACE_ERR("pd-l2seg:{}:failed to cleanup. still has {} uplink ifs",
                __FUNCTION__, l2seguplink_count);
        ret = HAL_RET_INVALID_OP;
        // TODO: Uncomment this once ifs are migrated to modify scheme
        // goto end;
    }

    // Releasing resources
    ret = l2seg_pd_dealloc_res(l2seg_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-l2seg:{}: unable to dealloc res for l2seg: {}", 
                      __FUNCTION__, 
                      ((l2seg_t *)(l2seg_pd->l2seg))->seg_id);
        goto end;
    }

    // Delinking PI<->PD
    l2seg_delink_pi_pd(l2seg_pd, (l2seg_t *)l2seg_pd->l2seg);

    // Freeing PD
    l2seg_pd_free(l2seg_pd);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Allocating hwid for l2segment
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
    HAL_TRACE_DEBUG("pd-l2seg:{}:l2seg_hwid: {},ten_hwid: {}, "
                    "l2seg_fl_lkup_id: {} ", 
                    __FUNCTION__, pd_l2seg->l2seg_hw_id, ten_pd->vrf_hw_id, 
                    pd_l2seg->l2seg_fl_lkup_id);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate hwid
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

        HAL_TRACE_DEBUG("pd-l2seg:{}:freed l2seg_hwid: {}", 
                        __FUNCTION__, l2seg_pd->l2seg_hw_id);
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Allocating cpuid used as vlan id for traffic coming from cpu
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_alloc_cpuid (pd_l2seg_t *pd_l2seg)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate from cpu id
    rs = g_hal_state_pd->l2seg_cpu_idxr()->
             alloc((uint32_t *)&pd_l2seg->l2seg_fromcpu_id);
    if (rs != indexer::SUCCESS) {
        pd_l2seg->l2seg_fromcpu_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("pd-l2seg:{}: seg_id:{} allocated from_cpu_id: {}", 
                    __FUNCTION__, ((l2seg_t*)(pd_l2seg->l2seg))->seg_id,
                    pd_l2seg->l2seg_fromcpu_id);

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate cpuid
//-----------------------------------------------------------------------------
hal_ret_t
l2seg_pd_dealloc_cpuid (pd_l2seg_t *l2seg_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (l2seg_pd->l2seg_fromcpu_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->l2seg_cpu_idxr()->free(l2seg_pd->l2seg_fromcpu_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-l2seg:{}:failed to free cpuid err: {}", 
                          __FUNCTION__, l2seg_pd->l2seg_fromcpu_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-l2seg:{}:freed from_cpu_id: {}", 
                        __FUNCTION__, l2seg_pd->l2seg_fromcpu_id);
    }

end:

    return ret;
}

uint32_t
pd_l2seg_get_l4_prof_idx (pd_l2seg_t *pd_l2seg)
{
    l2seg_t         *pi_l2seg = NULL;
    vrf_t        *pi_vrf = NULL;

    pi_l2seg = (l2seg_t *)pd_l2seg->l2seg;
    HAL_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_vrf = l2seg_get_pi_vrf(pi_l2seg);
    HAL_ASSERT_RETURN(pi_vrf != NULL, 0);

    return ten_get_nwsec_prof_hw_id(pi_vrf);
}

pd_vrf_t *
pd_l2seg_get_pd_vrf (pd_l2seg_t *pd_l2seg)
{
    l2seg_t         *pi_l2seg = NULL;
    vrf_t        *pi_vrf = NULL;

    pi_l2seg = (l2seg_t *)pd_l2seg->l2seg;
    HAL_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_vrf = l2seg_get_pi_vrf(pi_l2seg);
    HAL_ASSERT_RETURN(pi_vrf != NULL, 0);

    return (pd_vrf_t *)pi_vrf->pd;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
extern "C" hal_ret_t
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
// Returns the vrf hwid of the l2seg (used as lkp_vrf in flow key)
//-----------------------------------------------------------------------------
#if 0
l2seg_hw_id_t
pd_l2seg_get_flow_lkupid (l2seg_t *l2seg)
{
    return ((pd_l2seg_t *)l2seg->pd)->l2seg_fl_lkup_id;
}
#endif

extern "C" hal_ret_t 
pd_l2seg_get_flow_lkupid(pd_l2seg_get_flow_lkupid_args_t *args)
{
    l2seg_t *l2seg = args->l2seg;
    args->hwid = ((pd_l2seg_t *)l2seg->pd)->l2seg_fl_lkup_id;
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Returns the internal vlan of l2seg (used for input_properites lookup of
// reinjected packets) returns false if vlan is not valid
//-----------------------------------------------------------------------------
extern "C" hal_ret_t
pd_l2seg_get_fromcpu_vlanid (pd_l2seg_get_fromcpu_vlanid_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    l2seg_t *l2seg = args->l2seg;
    uint16_t *vid = args->vid;

    if (l2seg->segment_type == types::L2_SEGMENT_TYPE_INFRA) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (vid) {
        *vid = ((pd_l2seg_t *)l2seg->pd)->l2seg_fromcpu_id;
    }

end:

    return ret;
}

//----------------------------------------------------------------------------
// Frees PD memory without indexer free.
//----------------------------------------------------------------------------
extern "C" hal_ret_t
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
