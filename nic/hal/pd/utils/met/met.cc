#include "nic/hal/pd/utils/met/met.hpp"
#include "nic/hal/pd/utils/met/repl_list.hpp"

using hal::pd::utils::Met;
using hal::pd::utils::ReplList;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
Met *
Met::factory(std::string table_name, uint32_t table_id,
             uint32_t repl_table_capacity, uint32_t num_repl_entries,
             uint32_t repl_entry_data_len, uint32_t mtrack_id)
{
    void        *mem = NULL;
    Met   *met = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(Met));
    if (!mem) {
        return NULL;
    }

    met = new (mem) Met(table_name, table_id, repl_table_capacity,
                        num_repl_entries, repl_entry_data_len);

    HAL_TRACE_DEBUG("met: table_name: {}, tableid: {}, repl_table_capacity:{}"
                    "num_repl_entries:{}, repl_entry_data_len:{}",
                    met->table_name_, met->table_id_, met->repl_table_capacity_,
                    met->max_num_repls_per_entry_,
                    met->repl_entry_data_len_);
    return met;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
Met::destroy(Met *met, uint32_t mtrack_id)
{
    if (met) {
        met->~Met();
        HAL_FREE(mtrack_id, met);
    }
}

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

    // repl_table_indexer_         = new indexer(repl_table_capacity_, TRUE, TRUE);
#if 0
    repl_table_indexer_ = indexer::factory(repl_table_capacity_, TRUE, TRUE,
                                           HAL_MEM_ALLOC_MET_REPL_TABLE_INDEXER);
#endif
    repl_table_indexer_ = indexer::factory(repl_table_capacity_, TRUE, TRUE);

    // Initialize for Stats
    // stats_ = new uint64_t[STATS_MAX]();
    stats_ = (uint64_t *)HAL_CALLOC(HAL_MEM_ALLOC_MET_STATS,
                                    sizeof(uint64_t) * STATS_MAX);
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
Met::~Met()
{
    // delete repl_table_indexer_;
#if 0
    indexer::destroy(repl_table_indexer_,
                     HAL_MEM_ALLOC_MET_REPL_TABLE_INDEXER);
#endif
    indexer::destroy(repl_table_indexer_);
    HAL_FREE(HAL_MEM_ALLOC_MET_STATS, stats_);
}

// ----------------------------------------------------------------------------
// Create a Replication List
// ----------------------------------------------------------------------------
hal_ret_t
Met::create_repl_list(uint32_t *repl_list_idx)
{
    hal_ret_t       rs         = HAL_RET_OK;
    ReplList        *repl_list = NULL;

    // Allocate an index in repl. table
    rs = alloc_repl_table_index(repl_list_idx);
    if (rs != HAL_RET_OK) {
        goto end;
    }

    // repl_list = new ReplList(*repl_list_idx, this);
    repl_list = ReplList::factory(*repl_list_idx, this);

    repl_list->initialize_first_repl_table_entry();

    repl_list_map_[*repl_list_idx] = repl_list;

    // TODO: Only for debugging
    trace_met();
end:
    stats_update(INSERT, rs);
    return rs;
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

    // repl_list = new ReplList(repl_list_idx, this);
    repl_list = ReplList::factory(repl_list_idx, this);

    repl_list->initialize_first_repl_table_entry();

    repl_list_map_[repl_list_idx] = repl_list;

    // TODO: Only for debugging
    trace_met();
    end:
    return rs;
}

// ----------------------------------------------------------------------------
// Create a block of Replication Lists with consecutive IDs
// ----------------------------------------------------------------------------
hal_ret_t
Met::create_repl_list_block(uint32_t *repl_list_idx, uint32_t size)
{
    hal_ret_t   rs = HAL_RET_OK;
    ReplList    *repl_list = NULL;

    // Allocate the index block in repl. table
    indexer::status irs = repl_table_indexer_->alloc_block(repl_list_idx, size);
    if (irs != indexer::SUCCESS) {
        rs =  HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("Met:{}: Alloc repl table id: {}[+{}]", __FUNCTION__,
                    *repl_list_idx, size);

    for (uint32_t i = 0; i < size; i++) {
        repl_list = ReplList::factory((*repl_list_idx) + i, this);

        repl_list->initialize_first_repl_table_entry();

        repl_list_map_[(*repl_list_idx) + i] = repl_list;
    }

    // TODO: Only for debugging
    trace_met();
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
    ReplList                *rep_list = NULL;
    ReplListMap::iterator   itr;

    // Get Repl. List
    itr = repl_list_map_.find(repl_list_idx);
    if (itr == repl_list_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    rep_list = itr->second;
    if (rep_list) {
        rep_list->cleanup_last_repl_table_entry();
        ReplList::destroy(itr->second);
    }

    repl_list_map_.erase(repl_list_idx);
    rs = free_repl_table_index(repl_list_idx);

    // TODO: Only for debugging
    trace_met();
end:
    stats_update(REMOVE, rs);
    return rs;

}

// ----------------------------------------------------------------------------
// Delete a block of Replication Lists
// ----------------------------------------------------------------------------
hal_ret_t
Met::delete_repl_list_block(uint32_t repl_list_idx, uint32_t size)
{
    hal_ret_t rs = HAL_RET_OK;

    for (uint32_t i = 0; i < size; i++) {
        rs = delete_repl_list(repl_list_idx + i);
        if (rs != HAL_RET_OK) {
            HAL_TRACE_ERR("Met: {}: failed to delete rep list {}!", __FUNCTION__,
                          repl_list_idx + i);
            goto end;
        }
    }

end:
    return rs;
}
// ----------------------------------------------------------------------------
// Attach an existing Replication List to another existing Replication List
// This is useful for jumping to (*, G) entries at the end of (S, G) entries
// Also helpful in jumping to all-multicast list at the end of specific lists
// ----------------------------------------------------------------------------
hal_ret_t
Met::attach_repl_lists(uint32_t frm_list_idx, uint32_t to_list_idx)
{
    ReplListMap::iterator   itr, temp_itr;
    hal_ret_t               rs = HAL_RET_OK;
    ReplList                *frm_list = NULL, *temp_list = NULL;

    HAL_TRACE_DEBUG("Met:{}: from {} to {}", __FUNCTION__,
                    frm_list_idx, to_list_idx);

    // Get Repl. List
    itr = repl_list_map_.find(frm_list_idx);
    if (itr == repl_list_map_.end()) {
        HAL_TRACE_ERR("Met: {}: from list not found!", __FUNCTION__);
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    frm_list = itr->second;
    if (!frm_list) {
        HAL_TRACE_ERR("Met: {}: from list null!", __FUNCTION__);
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    // Get Repl. List
    itr = repl_list_map_.find(to_list_idx);
    if (itr == repl_list_map_.end()) {
        HAL_TRACE_ERR("Met: {}: to list not found!", __FUNCTION__);
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    temp_list = itr->second;
    while (temp_list->get_attached_list_index()) {
        // The next list is attached to another list
        if (temp_list->get_attached_list_index() == frm_list->get_repl_tbl_index()) {
            HAL_TRACE_ERR("{}: circular attachment not allowed!",
                          __FUNCTION__);
            rs = HAL_RET_INVALID_ARG;
            goto end;
        }

        temp_itr = repl_list_map_.find(temp_list->get_attached_list_index());
        if (temp_itr == repl_list_map_.end()) {
            HAL_TRACE_ERR("Met: {}: list not found in attached chain!",
                          __FUNCTION__);
            rs = HAL_RET_ENTRY_NOT_FOUND;
            goto end;
        }

        temp_list = temp_itr->second;
    }

    rs = frm_list->attach_to_repl_list(itr->second);

    if (rs != HAL_RET_OK){
        HAL_TRACE_ERR("Met: {}: Failed with ret {}", __FUNCTION__, rs);
        goto end;
    }

end:
    return rs;
}

// ----------------------------------------------------------------------------
// This is the detach function See Comments for attach_repl_lists()
// ----------------------------------------------------------------------------
hal_ret_t
Met::detach_repl_lists(uint32_t frm_list_idx)
{
    hal_ret_t               rs = HAL_RET_OK;
    ReplList                *frm_list = NULL;
    ReplListMap::iterator   itr;

    HAL_TRACE_DEBUG("Met:{}: from {}", __FUNCTION__, frm_list_idx);

    // Get Repl. List
    itr = repl_list_map_.find(frm_list_idx);
    if (itr == repl_list_map_.end()) {
        HAL_TRACE_ERR("Met: {}: from list not found!", __FUNCTION__);
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    frm_list = itr->second;
    if (!frm_list) {
        HAL_TRACE_ERR("Met: {}: from list null!", __FUNCTION__);
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    rs = frm_list->detach_frm_repl_list();

    if (rs != HAL_RET_OK){
        HAL_TRACE_ERR("Met: {}: Failed with ret {}", __FUNCTION__, rs);
        goto end;
    }

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
    return repl_table_indexer_->num_indices_allocated();
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
// Number of update operations attempted
// ----------------------------------------------------------------------------
uint32_t
Met::table_num_updates(void)
{
    return stats_[STATS_UPD_SUCCESS] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND];
}

// ----------------------------------------------------------------------------
// Number of failed update operations
// ----------------------------------------------------------------------------
uint32_t
Met::table_num_update_errors(void)
{
    return stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND];
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
