#include "nic/hal/pd/utils/met/met.hpp"
#include "nic/hal/pd/utils/met/repl_list.hpp"

using hal::pd::utils::Met;
using hal::pd::utils::ReplList;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
Met::Met(std::string table_name, uint32_t table_id, 
         uint32_t repl_table_capacity, uint32_t max_num_repls_per_entry,
         uint32_t repl_entry_data_len)
{
    table_name_                 = table_name;
    table_id_                   = table_id;
    repl_table_capacity_        = repl_table_capacity;
    max_num_repls_per_entry_    = max_num_repls_per_entry;
    repl_entry_data_len_        = repl_entry_data_len;

    repl_table_indexer_         = new indexer(repl_table_capacity_);
    
    // Initialize for Stats
    stats_ = new uint64_t[STATS_MAX]();
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
Met::~Met() 
{
    delete repl_table_indexer_;
}

// ----------------------------------------------------------------------------
// Create a Replication List with a known ID
// ----------------------------------------------------------------------------
hal_ret_t
Met::create_repl_list_with_id(uint32_t repl_list_idx)
{
    hal_ret_t   rs = HAL_RET_OK;
    ReplList    *repl_list = NULL;

    // Allocate the index in repl. table
    indexer::status irs = repl_table_indexer_->alloc_withid(repl_list_idx);
    if (irs != indexer::SUCCESS) {
        rs =  HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("Met:{}: Alloc repl table id: {}", __FUNCTION__, repl_list_idx);

    repl_list = new ReplList(repl_list_idx, this);

    repl_list_map_[repl_list_idx] = repl_list;

    // TODO: Only for debugging
    trace_met();
    end:
    return rs;
}


// ----------------------------------------------------------------------------
// Create a Replication List
// ----------------------------------------------------------------------------
hal_ret_t
Met::create_repl_list(uint32_t *repl_list_idx)
{
    hal_ret_t   rs = HAL_RET_OK;
    ReplList    *repl_list = NULL;

    // Allocate an index in repl. table
    rs = alloc_repl_table_index(repl_list_idx);
    if (rs != HAL_RET_OK) {
        goto end;
    }

    repl_list = new ReplList(*repl_list_idx, this);

    repl_list_map_[*repl_list_idx] = repl_list;

    // TODO: Only for debugging
    trace_met();
end:
    stats_update(INSERT, rs);
    return rs;
}

// ----------------------------------------------------------------------------
// Delete a Replication List
// ----------------------------------------------------------------------------
hal_ret_t
Met::delete_repl_list(uint32_t repl_list_idx)
{
    hal_ret_t               rs = HAL_RET_OK;
    ReplListMap::iterator   itr; 

    // Get Repl. List
    itr = repl_list_map_.find(repl_list_idx);
    if (itr == repl_list_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    delete itr->second;
    repl_list_map_.erase(repl_list_idx);
    rs = free_repl_table_index(repl_list_idx);

    // TODO: Only for debugging
    trace_met();
end:
    stats_update(REMOVE, rs);
    return rs;

}

// ----------------------------------------------------------------------------
// Add replication entry
// ----------------------------------------------------------------------------
hal_ret_t
Met::add_replication(uint32_t repl_list_idx, void *data)
{
    hal_ret_t               rs = HAL_RET_OK;
    ReplListMap::iterator   itr; 
    ReplList                *repl_list;

    HAL_TRACE_DEBUG("{}: Adding replication entry to : {}",
                    __FUNCTION__, repl_list_idx);

    // Get Repl. List
    itr = repl_list_map_.find(repl_list_idx);
    if (itr == repl_list_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    repl_list = itr->second;
    repl_list->add_replication(data);

    // TODO: Only for debugging
    trace_met();
end:
    return rs;
}


// ----------------------------------------------------------------------------
// Delete replication entry
// ----------------------------------------------------------------------------
hal_ret_t
Met::del_replication(uint32_t repl_list_idx, void *data)
{
    hal_ret_t               rs = HAL_RET_OK;
    ReplListMap::iterator   itr; 
    ReplList                *repl_list;

    // Get Repl. List
    itr = repl_list_map_.find(repl_list_idx);
    if (itr == repl_list_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    repl_list = itr->second;
    rs = repl_list->del_replication(data);

    // TODO: Only for debugging
    trace_met();
end:
    return rs;
}


// ----------------------------------------------------------------------------
// Allocate an index in replication table
// ----------------------------------------------------------------------------
hal_ret_t
Met::alloc_repl_table_index(uint32_t *idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    // Allocate an index in repl. table
    indexer::status irs = repl_table_indexer_->alloc(idx);
    if (irs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    HAL_TRACE_DEBUG("Met:{}: Alloc repl table id: {}", __FUNCTION__, *idx);
    return rs;
}

// ----------------------------------------------------------------------------
// Free an index in replication table
// ----------------------------------------------------------------------------
hal_ret_t
Met::free_repl_table_index(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    HAL_TRACE_DEBUG("Met:{}: Freeing repl table id: {}", __FUNCTION__, idx);
    indexer::status irs = repl_table_indexer_->free(idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return HAL_RET_DUP_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return HAL_RET_ERR;
    }

    return rs;
}

// ----------------------------------------------------------------------------
// Increment Stats
// ----------------------------------------------------------------------------
void
Met::stats_incr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
Met::stats_decr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
Met::stats_update(Met::api ap, hal_ret_t rs)
{
    switch (ap) {
        case INSERT:
            if(rs == HAL_RET_OK) stats_incr(STATS_INS_SUCCESS);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_INS_FAIL_HW);
            else if(rs == HAL_RET_NO_RESOURCE) stats_incr(STATS_INS_FAIL_NO_RES);
            else HAL_ASSERT(0);
            break;
        case UPDATE:
            if(rs == HAL_RET_OK) stats_incr(STATS_UPD_SUCCESS);
            else if(rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
            else HAL_ASSERT(0);
            break;
        case REMOVE:
            if (rs == HAL_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if (rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if (rs == HAL_RET_HW_FAIL) stats_incr(STATS_REM_FAIL_HW);
            else HAL_ASSERT(0);
            break;
        default:
            HAL_ASSERT(0);
    }
}

// ----------------------------------------------------------------------------
// Number of entries in use.
// ----------------------------------------------------------------------------
uint32_t
Met::table_num_entries_in_use(void)
{
    return repl_table_indexer_->usage();
}

// ----------------------------------------------------------------------------
// Number of insert operations attempted
// ----------------------------------------------------------------------------
uint32_t 
Met::table_num_inserts(void)
{
    return stats_[STATS_INS_SUCCESS] + 
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed insert operations
// ----------------------------------------------------------------------------
uint32_t 
Met::table_num_insert_errors(void)
{
    return stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of delete operations attempted
// ----------------------------------------------------------------------------
uint32_t 
Met::table_num_deletes(void)
{
    return stats_[STATS_REM_SUCCESS] + 
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed delete operations
// ----------------------------------------------------------------------------
uint32_t 
Met::table_num_delete_errors(void)
{
    return stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}


// ----------------------------------------------------------------------------
// Trace Met
// ----------------------------------------------------------------------------
hal_ret_t
Met::trace_met()
{
    hal_ret_t       ret = HAL_RET_OK;
    ReplList        *tmp_repl_list = NULL;

    HAL_TRACE_DEBUG("------------------------------------------------------");
    HAL_TRACE_DEBUG("Num. of Repl_lists: {}", repl_list_map_.size());

    for (std::map<uint32_t, ReplList*>::iterator it = repl_list_map_.begin(); 
            it != repl_list_map_.end(); ++it) {

        tmp_repl_list = it->second;

        tmp_repl_list->trace_repl_list();
    }
    HAL_TRACE_DEBUG("------------------------------------------------------");

    return ret;
}

