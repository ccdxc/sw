#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/iris/tnnl_rw_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include <netinet/ether.h>

namespace hal {
namespace pd {


//-----------------------------------------------------------------------------
// get key function
//-----------------------------------------------------------------------------
void *
tnnl_rw_entry_pd_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_tnnl_rw_entry_t*)entry)->tnnl_rw_key);
}

//-----------------------------------------------------------------------------
// compute hash func
//-----------------------------------------------------------------------------
uint32_t
tnnl_rw_entry_pd_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(pd_tnnl_rw_entry_key_t)) % ht_size;
}

//-----------------------------------------------------------------------------
// compare func.
//-----------------------------------------------------------------------------
bool
tnnl_rw_entry_pd_compare_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(pd_tnnl_rw_entry_key_t))) {
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// Tunnel rw entry key trace
//-----------------------------------------------------------------------------
hal_ret_t
tnnl_rw_entry_key_trace(pd_tnnl_rw_entry_key_t *tnnl_rw_key)
{
    fmt::MemoryWriter   buf;

    buf.write("[ rw_act: {}, ", tnnl_rw_key->tnnl_rw_act);
    buf.write("mac_sa: {}, ", macaddr2str(tnnl_rw_key->mac_sa));
    buf.write("mac_da: {}, ", macaddr2str(tnnl_rw_key->mac_da));
    buf.write("ip_sa: {}, ", ipaddr2str(&tnnl_rw_key->ip_sa));
    buf.write("ip_da: {}, ", ipaddr2str(&tnnl_rw_key->ip_da));
    buf.write("ip_type: {}, ", tnnl_rw_key->ip_type);
    buf.write("vlan_v: {}, vlan_id {} ]", tnnl_rw_key->vlan_valid, 
              tnnl_rw_key->vlan_id);
    HAL_TRACE_DEBUG("{}", buf.c_str());

    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// Find an entry 
//-----------------------------------------------------------------------------
hal_ret_t
tnnl_rw_entry_find(pd_tnnl_rw_entry_key_t *tnnl_rw_key, pd_tnnl_rw_entry_t **tnnl_rwe)
{
    hal_ret_t             ret = HAL_RET_OK;

    if (!tnnl_rw_key || !tnnl_rwe) {
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    *tnnl_rwe = find_tnnl_rw_entry_by_key(tnnl_rw_key);
    if ((*tnnl_rwe) == NULL) {
        ret = HAL_RET_ENTRY_NOT_FOUND;
    } else {
        HAL_TRACE_DEBUG("pd-tnnl_rw: Found tnnl_rw_id: {} ",
                        (*tnnl_rwe)->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(tnnl_rw_key);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate an entry 
//-----------------------------------------------------------------------------
hal_ret_t
tnnl_rw_entry_alloc(pd_tnnl_rw_entry_key_t *tnnl_rw_key, 
                    pd_tnnl_rw_entry_info_t *tnnl_rw_info, 
                    uint32_t *tnnl_rw_idx)
{
    hal_ret_t             ret = HAL_RET_OK;
    uint32_t            tmp_tnnl_rw_idx = 0;
    indexer::status     rs = indexer::SUCCESS;
    pd_tnnl_rw_entry_t  *tnnl_rwe = NULL;
    fmt::MemoryWriter   buf;
    
    if (!tnnl_rw_key || !tnnl_rw_idx) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = tnnl_rw_entry_find(tnnl_rw_key, &tnnl_rwe);
    if (ret == HAL_RET_OK) {
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }
    ret = HAL_RET_OK;
    
    // TODO: Revisit if we have to maintain an indexer at all. Why can't we use the 
    //       direct map's indexer. May be do the insert into hash table and ref count
    //       increments after the directmap API goes through.

    // Allocate an id for this entry
    if (tnnl_rw_info && tnnl_rw_info->with_id) {
        // Set the id
        tmp_tnnl_rw_idx = tnnl_rw_info->tnnl_rw_idx;
        rs = g_hal_state_pd->tnnl_rw_tbl_idxr()->
            alloc_withid(tmp_tnnl_rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-tnnl_rw: Indexer err: {}", rs);
            goto end;
        }
    } else {
        rs = g_hal_state_pd->tnnl_rw_tbl_idxr()->alloc(&tmp_tnnl_rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-tnnl_rw: Resource Exhaustion Usage: {} for ",
                          g_hal_state_pd->tnnl_rw_tbl_idxr()->num_indices_allocated());
            tnnl_rw_entry_key_trace(tnnl_rw_key);
            ret = HAL_RET_NO_RESOURCE;
        }
    }

    // Insert into hash table
    tnnl_rwe = tnnl_rw_entry_pd_alloc_init();
    if (!tnnl_rwe) {
        ret = HAL_RET_OOM;
        goto end;
    }
    memcpy(&tnnl_rwe->tnnl_rw_key, tnnl_rw_key, sizeof(pd_tnnl_rw_entry_key_t));
    tnnl_rwe->tnnl_rw_idx = tmp_tnnl_rw_idx;
    ret = add_tnnl_rw_entry_pd_to_db(tnnl_rwe);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Increment the ref count
    tnnl_rwe->ref_cnt++;

    HAL_TRACE_DEBUG("pd-tnnl_rw: Usage: {} ref_cnt: {} Allocated tnnl_rw_id: {} for ",
                    g_hal_state_pd->tnnl_rw_tbl_idxr()->num_indices_allocated(),
                    tnnl_rwe->ref_cnt,
                    tmp_tnnl_rw_idx);
    tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);

    // Program HW
    if ((uint8_t)tnnl_rwe->tnnl_rw_key.tnnl_rw_act == (uint8_t)TUNNEL_REWRITE_ENCAP_VXLAN_ID) {
        ret = tnnl_rw_pd_pgm_tnnl_rw_tbl_vxlan(tnnl_rwe);
    } else if ((uint8_t)tnnl_rwe->tnnl_rw_key.tnnl_rw_act == (uint8_t)TUNNEL_REWRITE_ENCAP_ERSPAN_ID) {
        ret = tnnl_rw_pd_pgm_tnnl_rw_tbl_erspan(tnnl_rwe);
    } else if ((uint8_t)tnnl_rwe->tnnl_rw_key.tnnl_rw_act == (uint8_t)TUNNEL_REWRITE_ENCAP_VLAN_ID) {
        ret = tnnl_rw_pd_pgm_tnnl_rw_tbl_vlan(tnnl_rwe);
    }
    if (ret != HAL_RET_OK) {
        del_tnnl_rw_entry_pd_from_db(tnnl_rwe);
        tnnl_rw_entry_pd_free(tnnl_rwe);
    } else {
        *tnnl_rw_idx = tmp_tnnl_rw_idx;
    }

end:
    return ret;
} 

//-----------------------------------------------------------------------------
// Find or Allocate an entry 
// - Check if entry already exists. If it exists, returns the allocated rw_idx.
// - If it doesnt exist, allocated a new entry and allocates an index from
//   the indexer.
//-----------------------------------------------------------------------------
hal_ret_t
tnnl_rw_entry_find_or_alloc(pd_tnnl_rw_entry_key_t *tnnl_rw_key, uint32_t *tnnl_rw_idx)
{
    hal_ret_t         ret = HAL_RET_OK;
    pd_tnnl_rw_entry_t       *tnnl_rwe = NULL;
    fmt::MemoryWriter   buf;

    if (!tnnl_rw_key) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = tnnl_rw_entry_find(tnnl_rw_key, &tnnl_rwe);
    if (ret == HAL_RET_OK) {
        *tnnl_rw_idx = tnnl_rwe->tnnl_rw_idx;
        // Increment the ref count
        tnnl_rwe->ref_cnt++;
        HAL_TRACE_DEBUG("pd-tnnl_rw: Usage: {} ref_cnt: {} Find/Alloc "
                        "tnnl_rw_id: {} for ",
                        g_hal_state_pd->tnnl_rw_tbl_idxr()->num_indices_allocated(),
                        tnnl_rwe->ref_cnt,
                        tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(tnnl_rw_key);
        goto end;
    }
    
    ret = tnnl_rw_entry_alloc(tnnl_rw_key, NULL, tnnl_rw_idx);
 end:
    return ret;
}

#if 0
hal_ret_t
pd_tnnl_rw_entry_find_or_alloc(pd_tnnl_rw_entry_args_t *args, uint32_t *tnnl_rw_idx)
{
    pd_tnnl_rw_entry_key_t   tnnl_rw_key{};
    if (!args) {
        return HAL_RET_INVALID_ARG;
    }

    tnnl_rw_key.tnnl_rw_act = args->tnnl_rw_act;
    memcpy(tnnl_rw_key.mac_sa, args->mac_sa, sizeof(args->mac_sa));
    memcpy(tnnl_rw_key.mac_da, args->mac_da, sizeof(args->mac_da));

    return tnnl_rw_entry_find_or_alloc(&tnnl_rw_key, tnnl_rw_idx);
}
#endif

//-----------------------------------------------------------------------------
// Delete the rw entry
//-----------------------------------------------------------------------------
hal_ret_t
tnnl_rw_entry_delete(pd_tnnl_rw_entry_key_t *tnnl_rw_key) 
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_tnnl_rw_entry_t       *tnnl_rwe = NULL;
    indexer::status     rs = indexer::SUCCESS;

    if (!tnnl_rw_key) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = tnnl_rw_entry_find(tnnl_rw_key, &tnnl_rwe);
    if (ret != HAL_RET_OK) {
        // entry not found
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    // Decrement the ref count
    tnnl_rwe->ref_cnt--;

    HAL_TRACE_DEBUG("pd-tnnl_rw: Usage: {} ref_cnt: {} Delete tnnl_rw_id: {} for ",
                    g_hal_state_pd->tnnl_rw_tbl_idxr()->num_indices_allocated(),
                    tnnl_rwe->ref_cnt,
                    tnnl_rwe->tnnl_rw_idx);
    tnnl_rw_entry_key_trace(tnnl_rw_key);

    if (tnnl_rwe->ref_cnt == 0) {

        // Deprogram HW
        ret = tnnl_rw_pd_depgm_tnnl_rw_tbl(tnnl_rwe);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Delete from DB
        ret = del_tnnl_rw_entry_pd_from_db(tnnl_rwe);
        if (ret != HAL_RET_OK) {
            HAL_ASSERT_RETURN(0, HAL_RET_ERR);
        }

        // Free the index
        rs = g_hal_state_pd->tnnl_rw_tbl_idxr()->free(tnnl_rwe->tnnl_rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("PD-tnnl_rw: Failed to free Indexer err: {}", rs);
        }

        tnnl_rw_entry_pd_free(tnnl_rwe);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Deprogramming the hw entry
//-----------------------------------------------------------------------------
hal_ret_t
tnnl_rw_pd_depgm_tnnl_rw_tbl(pd_tnnl_rw_entry_t *tnnl_rwe) 
{
    hal_ret_t            ret = HAL_RET_OK;
    sdk_ret_t            sdk_ret;
    directmap            *tnnl_rw_tbl = NULL;

    tnnl_rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT_RETURN((tnnl_rw_tbl != NULL), HAL_RET_ERR);

    sdk_ret = tnnl_rw_tbl->remove(tnnl_rwe->tnnl_rw_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("pd-tnnl_rw: Unable to de-program at tnnl_rw_id: {} "
                        "ret: {}, for ",
                        tnnl_rwe->tnnl_rw_idx, ret);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    } else {
        HAL_TRACE_DEBUG("pd-tnnl_rw: De-Programmed at tnnl_rw_id: {} for ",
                  tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Programming the hw entry
//-----------------------------------------------------------------------------
#define data_erspan data.tunnel_rewrite_action_u.tunnel_rewrite_encap_erspan
hal_ret_t
tnnl_rw_pd_pgm_tnnl_rw_tbl_erspan (pd_tnnl_rw_entry_t *tnnl_rwe)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    tunnel_rewrite_actiondata   data;
    directmap                   *tnnl_rw_tbl = NULL;

    memset(&data, 0, sizeof(data));

    tnnl_rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT_RETURN((tnnl_rw_tbl != NULL), HAL_RET_ERR);

    memcpy(data_erspan.mac_sa, tnnl_rwe->tnnl_rw_key.mac_sa, ETH_ADDR_LEN);
    memrev(data_erspan.mac_sa, ETH_ADDR_LEN);
    memcpy(data_erspan.mac_da, tnnl_rwe->tnnl_rw_key.mac_da, ETH_ADDR_LEN);
    memrev(data_erspan.mac_da, ETH_ADDR_LEN);
    memcpy(&data_erspan.ip_sa, &tnnl_rwe->tnnl_rw_key.ip_sa.addr, sizeof(uint32_t));
    memcpy(&data_erspan.ip_da, &tnnl_rwe->tnnl_rw_key.ip_da.addr, sizeof(uint32_t));
    data_erspan.ip_type = tnnl_rwe->tnnl_rw_key.ip_type;
    data_erspan.vlan_valid = tnnl_rwe->tnnl_rw_key.vlan_valid;
    data_erspan.vlan_id = tnnl_rwe->tnnl_rw_key.vlan_id;
    data.actionid = tnnl_rwe->tnnl_rw_key.tnnl_rw_act;
    sdk_ret = tnnl_rw_tbl->insert_withid(&data, tnnl_rwe->tnnl_rw_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-tnnl_rw: Unable to program at tnnl_rw_id: {} for ",
                      tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    } else {
        HAL_TRACE_DEBUG("pd-tnnl_rw: Programmed at tnnl_rw_id: {} for ",
                        tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    }

    return ret;
}

#define data_vxlan data.tunnel_rewrite_action_u.tunnel_rewrite_encap_vxlan
hal_ret_t
tnnl_rw_pd_pgm_tnnl_rw_tbl_vxlan (pd_tnnl_rw_entry_t *tnnl_rwe)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    tunnel_rewrite_actiondata   data;
    directmap                   *tnnl_rw_tbl = NULL;

    memset(&data, 0, sizeof(data));

    tnnl_rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT_RETURN((tnnl_rw_tbl != NULL), HAL_RET_ERR);

    memcpy(data_vxlan.mac_sa, tnnl_rwe->tnnl_rw_key.mac_sa, ETH_ADDR_LEN);
    memrev(data_vxlan.mac_sa, ETH_ADDR_LEN);
    memcpy(data_vxlan.mac_da, tnnl_rwe->tnnl_rw_key.mac_da, ETH_ADDR_LEN);
    memrev(data_vxlan.mac_da, ETH_ADDR_LEN);
    memcpy(&data_vxlan.ip_sa, &tnnl_rwe->tnnl_rw_key.ip_sa.addr, sizeof(uint32_t));
    memcpy(&data_vxlan.ip_da, &tnnl_rwe->tnnl_rw_key.ip_da.addr, sizeof(uint32_t));
    data_vxlan.ip_type = tnnl_rwe->tnnl_rw_key.ip_type;
    data_vxlan.vlan_valid = tnnl_rwe->tnnl_rw_key.vlan_valid;
    data_vxlan.vlan_id = tnnl_rwe->tnnl_rw_key.vlan_id;
    data.actionid = tnnl_rwe->tnnl_rw_key.tnnl_rw_act;
    sdk_ret = tnnl_rw_tbl->insert_withid(&data, tnnl_rwe->tnnl_rw_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-tnnl_rw: Unable to program at tnnl_rw_id: {} for ",
                      tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    } else {
        HAL_TRACE_DEBUG("pd-tnnl_rw: Programmed at tnnl_rw_id: {} for ",
                        tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    }

    return ret;
}

hal_ret_t
tnnl_rw_pd_pgm_tnnl_rw_tbl_vlan (pd_tnnl_rw_entry_t *tnnl_rwe)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    tunnel_rewrite_actiondata   data;
    directmap                   *tnnl_rw_tbl = NULL;

    memset(&data, 0, sizeof(data));

    tnnl_rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT_RETURN((tnnl_rw_tbl != NULL), HAL_RET_ERR);

    data.actionid = tnnl_rwe->tnnl_rw_key.tnnl_rw_act;
    sdk_ret = tnnl_rw_tbl->insert_withid(&data, tnnl_rwe->tnnl_rw_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-tnnl_rw: Unable to program at tnnl_rw_id: {} for ",
                      tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    } else {
        HAL_TRACE_DEBUG("pd-tnnl_rw: Programmed at tnnl_rw_id: {} for ",
                        tnnl_rwe->tnnl_rw_idx);
        tnnl_rw_entry_key_trace(&tnnl_rwe->tnnl_rw_key);
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
