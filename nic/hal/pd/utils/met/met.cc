#include "met.hpp"
#include "repl_list.hpp"

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
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
Met::~Met() 
{
    delete repl_table_indexer_;
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

end:
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

end:
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

    // Get Repl. List
    itr = repl_list_map_.find(repl_list_idx);
    if (itr == repl_list_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    repl_list = itr->second;
    repl_list->add_replication(data);

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
    repl_list->del_replication(data);

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

    HAL_TRACE_DEBUG("Met:{}: Alloc table id: {}", __FUNCTION__, *idx);
    return rs;
}

// ----------------------------------------------------------------------------
// Free an index in replication table
// ----------------------------------------------------------------------------
hal_ret_t
Met::free_repl_table_index(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    HAL_TRACE_DEBUG("Met:{}: Freeing table id: {}", __FUNCTION__, idx);
    indexer::status irs = repl_table_indexer_->free(idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return HAL_RET_DUP_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return HAL_RET_ERR;
    }

    return rs;
}
