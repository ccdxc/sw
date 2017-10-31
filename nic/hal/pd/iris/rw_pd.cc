#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/rw_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include <netinet/ether.h>

namespace hal {
namespace pd {


//-----------------------------------------------------------------------------
// get key function
//-----------------------------------------------------------------------------
void *
rw_entry_pd_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_rw_entry_t*)entry)->rw_key);
}

//-----------------------------------------------------------------------------
// compute hash func
//-----------------------------------------------------------------------------
uint32_t
rw_entry_pd_compute_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(pd_rw_entry_key_t)) % ht_size;
}

//-----------------------------------------------------------------------------
// compare func.
//-----------------------------------------------------------------------------
bool
rw_entry_pd_compare_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
	if (!memcmp(key1, key2, sizeof(pd_rw_entry_key_t))) {
		return true;
	}
    return false;
}

//-----------------------------------------------------------------------------
// Find an entry 
//-----------------------------------------------------------------------------
hal_ret_t
rw_entry_find(pd_rw_entry_key_t *rw_key, pd_rw_entry_t **rwe)
{
	hal_ret_t 		    ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;

	if (!rw_key || !rwe) {
		ret = HAL_RET_ENTRY_NOT_FOUND;
		goto end;
	}

	*rwe = find_rw_entry_by_key(rw_key);
	if ((*rwe) == NULL) {
		ret = HAL_RET_ENTRY_NOT_FOUND;
	} else {
        buf.write("PD-RW: Found rw_id: {} for [ rw_act: {}, ",
                (*rwe)->rw_idx, 
                rw_key->rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rw_key->mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rw_key->mac_da));
        HAL_TRACE_DEBUG(buf.c_str());

    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate an entry 
//-----------------------------------------------------------------------------
hal_ret_t
rw_entry_alloc(pd_rw_entry_key_t *rw_key, pd_rw_entry_info_t *rw_info, 
               uint32_t *rw_idx)
{
	hal_ret_t 			ret = HAL_RET_OK;
	uint32_t			tmp_rw_idx = 0;
    indexer::status     rs = indexer::SUCCESS;
    pd_rw_entry_t       *rwe = NULL;
    fmt::MemoryWriter   buf;
	
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
            HAL_TRACE_ERR("PD-RW: Indexer err: {}", rs);
            goto end;
        }
	} else {
		rs = g_hal_state_pd->rw_tbl_idxr()->alloc(&tmp_rw_idx);
        if (rs != indexer::SUCCESS) {
            buf.write("PD-RW: Resource Exhaustion Usage: {} [ rw_act: {}, ",
                    g_hal_state_pd->rw_tbl_idxr()->usage(), rw_key->rw_act);
            buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rw_key->mac_sa));
            buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rw_key->mac_da));
            HAL_TRACE_ERR(buf.c_str());
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

	buf.write("PD-RW: Usage: {} ref_cnt: {} Allocated rw_id: {} for [ rw_act: {}, ",
					g_hal_state_pd->rw_tbl_idxr()->usage(),
                    rwe->ref_cnt,
				    tmp_rw_idx, 
					rw_key->rw_act);
    buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rw_key->mac_sa));
    buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rw_key->mac_da));
    HAL_TRACE_DEBUG(buf.c_str());

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
    fmt::MemoryWriter   buf;

    if (!rw_key) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = rw_entry_find(rw_key, &rwe);
    if (ret == HAL_RET_OK) {
        *rw_idx = rwe->rw_idx;
        // Increment the ref count
        rwe->ref_cnt++;
        buf.write("PD-RW: Usage: {} ref_cnt: {} Find/Alloc rw_id: {} for [ rw_act: {}, ",
                  g_hal_state_pd->rw_tbl_idxr()->usage(),
                  rwe->ref_cnt,
                  rwe->rw_idx, 
                  rw_key->rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rw_key->mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rw_key->mac_da));
        HAL_TRACE_DEBUG(buf.c_str());
        goto end;
    }
    
    ret = rw_entry_alloc(rw_key, NULL, rw_idx);
 end:
    return ret;
}

hal_ret_t
pd_rw_entry_find_or_alloc(pd_rw_entry_args_t *args, uint32_t *rw_idx)
{
    pd_rw_entry_key_t   rw_key{};
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
rw_entry_delete(pd_rw_entry_key_t *rw_key) 
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

    buf.write("PD-RW: Usage: {} ref_cnt: {} Delete rw_id: {} for [ rw_act: {}, ",
            g_hal_state_pd->rw_tbl_idxr()->usage(),
            rwe->ref_cnt,
            rwe->rw_idx, 
            rw_key->rw_act);
    buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rw_key->mac_sa));
    buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rw_key->mac_da));
    HAL_TRACE_DEBUG(buf.c_str());

    if (rwe->ref_cnt == 0) {

        // Deprogram HW
        ret = rw_pd_depgm_rw_tbl(rwe);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Delete from DB
        ret = del_rw_entry_pd_from_db(rwe);
        if (ret != HAL_RET_OK) {
            HAL_ASSERT_RETURN(0, HAL_RET_ERR);
        }

        // Free the index
        rs = g_hal_state_pd->rw_tbl_idxr()->free(rwe->rw_idx);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("PD-RW: Failed to free Indexer err: {}", rs);
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
rw_pd_depgm_rw_tbl(pd_rw_entry_t *rwe) 
{
    hal_ret_t            ret = HAL_RET_OK;
    DirectMap            *rw_tbl = NULL;
    fmt::MemoryWriter    buf;

    rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    HAL_ASSERT_RETURN((rw_tbl != NULL), HAL_RET_ERR);

    ret = rw_tbl->remove(rwe->rw_idx);
    if (ret != HAL_RET_OK) {
        buf.write("PD-RW: Unable to de-program at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx, 
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    } else {
        buf.write("PD-RW: De-Programmed at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx, 
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    }
    HAL_TRACE_DEBUG(buf.c_str());

    return ret;
}

//-----------------------------------------------------------------------------
// Programming the hw entry
//-----------------------------------------------------------------------------
hal_ret_t
rw_pd_pgm_rw_tbl(pd_rw_entry_t *rwe)
{
    hal_ret_t            ret = HAL_RET_OK;
    rewrite_actiondata   data;
    DirectMap            *rw_tbl = NULL;
    fmt::MemoryWriter    buf;

    memset(&data, 0, sizeof(data));
    

    rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    HAL_ASSERT_RETURN((rw_tbl != NULL), HAL_RET_ERR);

    memcpy(data.rewrite_action_u.rewrite_rewrite.mac_sa, rwe->rw_key.mac_sa, 6);
    memrev(data.rewrite_action_u.rewrite_rewrite.mac_sa, 6);
    memcpy(data.rewrite_action_u.rewrite_rewrite.mac_da, rwe->rw_key.mac_da, 6);
    memrev(data.rewrite_action_u.rewrite_rewrite.mac_da, 6);
    data.actionid = rwe->rw_key.rw_act;
    ret = rw_tbl->insert_withid(&data, rwe->rw_idx);
    if (ret != HAL_RET_OK) {
        buf.write("PD-RW: Unable to program at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx, 
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    } else {
        buf.write("PD-RW: Programmed at rw_id: {} for [ rw_act: {}, ",
                  rwe->rw_idx, 
                  rwe->rw_key.rw_act);
        buf.write("mac_sa: {}, ", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_sa));
        buf.write("mac_da: {} ]", ether_ntoa((struct ether_addr*)&rwe->rw_key.mac_da));
    }
    HAL_TRACE_DEBUG(buf.c_str());

    return ret;
}

}    // namespace pd
}    // namespace hal
