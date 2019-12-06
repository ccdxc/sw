#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/nw/rw_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include <netinet/ether.h>

namespace hal {
namespace pd {


//-----------------------------------------------------------------------------
// get key function
//-----------------------------------------------------------------------------
void *
rw_entry_pd_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_rw_entry_t*)entry)->rw_key);
}

//-----------------------------------------------------------------------------
// compute hash key size
//-----------------------------------------------------------------------------
uint32_t
rw_entry_pd_key_size ()
{
    return sizeof(pd_rw_entry_key_t);
}

//-----------------------------------------------------------------------------
// Find an entry
//-----------------------------------------------------------------------------
hal_ret_t
rw_entry_find (pd_rw_entry_key_t *rw_key, pd_rw_entry_t **rwe)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (!rw_key || !rwe) {
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    *rwe = find_rw_entry_by_key(rw_key);
    if ((*rwe) == NULL) {
        ret = HAL_RET_ENTRY_NOT_FOUND;
    } else {
        HAL_TRACE_VERBOSE("Found rw_id: {} for [ rw_act: {}, mac_sa: {}, mac_da: {} ]",
                          (*rwe)->rw_idx, rw_key->rw_act,
                          ether_ntoa((struct ether_addr*)&rw_key->mac_sa),
                          ether_ntoa((struct ether_addr*)&rw_key->mac_da));
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Allocate an entry
//-----------------------------------------------------------------------------
hal_ret_t
rw_entry_alloc (pd_rw_entry_key_t *rw_key, pd_rw_entry_info_t *rw_info,
                uint32_t *rw_idx)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            tmp_rw_idx = 0;
    indexer::status     rs = indexer::SUCCESS;
    pd_rw_entry_t       *rwe = NULL;

    if (!rw_key || !rw_idx) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = rw_entry_find(rw_key, &rwe);
    if (ret == HAL_RET_OK) {
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }
    ret = HAL_RET_OK;

    // Allocate an id for this entry
    if (rw_info && rw_info->with_id) {
        // Set the id
        tmp_rw_idx = rw_info->rw_idx;
        rs = g_hal_state_pd->rw_tbl_idxr()->alloc_withid(tmp_rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Indexer err: {}", rs);
            goto end;
        }
    } else {
        rs = g_hal_state_pd->rw_tbl_idxr()->alloc(&tmp_rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Resource Exhaustion num_indices_allocated: {} [ rw_act: {}, mac_sa: {}, mac_da: {} ]",
                          g_hal_state_pd->rw_tbl_idxr()->num_indices_allocated(), rw_key->rw_act,
                          ether_ntoa((struct ether_addr*)&rw_key->mac_sa),
                          ether_ntoa((struct ether_addr*)&rw_key->mac_da));
            ret = HAL_RET_NO_RESOURCE;
        }
    }

    // Insert into hash table
    rwe = rw_entry_pd_alloc_init();
    if (!rwe) {
        ret = HAL_RET_OOM;
        goto end;
    }
    memcpy(&rwe->rw_key, rw_key, sizeof(pd_rw_entry_key_t));
    rwe->rw_idx = tmp_rw_idx;
    ret = add_rw_entry_pd_to_db(rwe);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Increment the ref count
    rwe->ref_cnt++;

    HAL_TRACE_DEBUG("Usage: {} ref_cnt: {} Allocated rw_id: {} for [ rw_act: {}, mac_sa: {}, mac_da: {} ]",
                    g_hal_state_pd->rw_tbl_idxr()->num_indices_allocated(),
                    rwe->ref_cnt, tmp_rw_idx, rw_key->rw_act,
                    ether_ntoa((struct ether_addr*)&rw_key->mac_sa),
                    ether_ntoa((struct ether_addr*)&rw_key->mac_da));
    // Program HW
    ret = rw_pd_pgm_rw_tbl(rwe);

    if (ret != HAL_RET_OK) {
        del_rw_entry_pd_from_db(rwe);
        rw_entry_pd_free(rwe);
    } else {
        *rw_idx = tmp_rw_idx;
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
rw_entry_find_or_alloc(pd_rw_entry_key_t *rw_key, uint32_t *rw_idx)
{
    hal_ret_t 		ret = HAL_RET_OK;
    pd_rw_entry_t       *rwe = NULL;

    if (!rw_key) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = rw_entry_find(rw_key, &rwe);
    if (ret == HAL_RET_OK) {
        *rw_idx = rwe->rw_idx;
        // Increment the ref count
        rwe->ref_cnt++;
        HAL_TRACE_VERBOSE("Usage: {} ref_cnt: {} Find/Alloc rw_id: {} "
                          "for [ rw_act: {}, mac_sa: {}, mac_da: {} ]",
                          g_hal_state_pd->rw_tbl_idxr()->num_indices_allocated(),
                          rwe->ref_cnt, rwe->rw_idx, rw_key->rw_act,
                          ether_ntoa((struct ether_addr*)&rw_key->mac_sa),
                          ether_ntoa((struct ether_addr*)&rw_key->mac_da));
        goto end;
    }

    ret = rw_entry_alloc(rw_key, NULL, rw_idx);

end:
    return ret;
}

hal_ret_t
pd_rw_entry_find_or_alloc (pd_func_args_t *pd_func_args)
{
    pd_rw_entry_key_t   rw_key{};
    pd_rw_entry_find_or_alloc_args_t *rw_args = pd_func_args->pd_rw_entry_find_or_alloc;
    pd_rw_entry_args_t *args = rw_args->args;
    uint32_t *rw_idx = rw_args->rw_idx;

    if (!args) {
        return HAL_RET_INVALID_ARG;
    }

    rw_key.rw_act = args->rw_act;
    memcpy(rw_key.mac_sa, args->mac_sa, sizeof(args->mac_sa));
    memcpy(rw_key.mac_da, args->mac_da, sizeof(args->mac_da));

    return rw_entry_find_or_alloc(&rw_key, rw_idx);
}

//-----------------------------------------------------------------------------
// Delete the rw entry
//-----------------------------------------------------------------------------
hal_ret_t
rw_entry_delete (pd_rw_entry_key_t *rw_key)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_rw_entry_t       *rwe = NULL;
    indexer::status     rs = indexer::SUCCESS;
    fmt::MemoryWriter   buf;

    if (!rw_key) {
		ret = HAL_RET_INVALID_ARG;
		goto end;
    }

	ret = rw_entry_find(rw_key, &rwe);
    if (ret != HAL_RET_OK) {
        // entry not found
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    // Decrement the ref count
    rwe->ref_cnt--;

    buf.write("Usage: {} ref_cnt: {} Delete rw_id: {} for [ rw_act: {}, ",
            g_hal_state_pd->rw_tbl_idxr()->num_indices_allocated(),
            rwe->ref_cnt,
            rwe->rw_idx,
            rw_key->rw_act);
    buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rw_key->mac_sa));
    buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rw_key->mac_da));
    HAL_TRACE_DEBUG("{}", buf.c_str());

    if (rwe->ref_cnt == 0) {

        // Deprogram HW
        ret = rw_pd_depgm_rw_tbl(rwe);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Delete from DB
        ret = del_rw_entry_pd_from_db(rwe);
        if (ret != HAL_RET_OK) {
            SDK_ASSERT_RETURN(0, HAL_RET_ERR);
        }

        // Free the index
        rs = g_hal_state_pd->rw_tbl_idxr()->free(rwe->rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free Indexer err: {}", rs);
        }

        rw_entry_pd_free(rwe);
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Deprogramming the hw entry
//-----------------------------------------------------------------------------
hal_ret_t
rw_pd_depgm_rw_tbl (pd_rw_entry_t *rwe)
{
    hal_ret_t            ret = HAL_RET_OK;
    sdk_ret_t            sdk_ret;
    directmap            *rw_tbl = NULL;
    fmt::MemoryWriter    buf;

    rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    SDK_ASSERT_RETURN((rw_tbl != NULL), HAL_RET_ERR);

    sdk_ret = rw_tbl->remove(rwe->rw_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        buf.write("Unable to de-program at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx,
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    } else {
        buf.write("De-Programmed at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx,
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    }
    HAL_TRACE_DEBUG("{}", buf.c_str());

    return ret;
}

//-----------------------------------------------------------------------------
// Programming the hw entry
//-----------------------------------------------------------------------------
hal_ret_t
rw_pd_pgm_rw_tbl (pd_rw_entry_t *rwe)
{
    hal_ret_t            ret = HAL_RET_OK;
    sdk_ret_t            sdk_ret;
    rewrite_actiondata_t   data;
    directmap            *rw_tbl = NULL;
    fmt::MemoryWriter    buf;

    memset(&data, 0, sizeof(data));


    rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    SDK_ASSERT_RETURN((rw_tbl != NULL), HAL_RET_ERR);

    memcpy(data.action_u.rewrite_rewrite.mac_sa, rwe->rw_key.mac_sa, 6);
    memrev(data.action_u.rewrite_rewrite.mac_sa, 6);
    memcpy(data.action_u.rewrite_rewrite.mac_da, rwe->rw_key.mac_da, 6);
    memrev(data.action_u.rewrite_rewrite.mac_da, 6);
    data.action_id = rwe->rw_key.rw_act;
    sdk_ret = rw_tbl->insert_withid(&data, rwe->rw_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        buf.write("Unable to program at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx,
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    } else {
        buf.write("Programmed at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx,
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    }
    HAL_TRACE_DEBUG("{}", buf.c_str());

    return ret;
}

}    // namespace pd
}    // namespace hal
