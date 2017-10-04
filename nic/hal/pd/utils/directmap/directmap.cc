#include "nic/hal/pd/utils/directmap/directmap.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/utils/thread/thread.hpp"

using hal::pd::utils::DirectMap;

namespace hal {
    // thread_local utils::thread *t_curr_thread;
    // bool      gl_super_user = false;
}
// ----------------------------------------------------------------------------
// Constructor - DirectMap
// ----------------------------------------------------------------------------
DirectMap::DirectMap(std::string table_name, uint32_t table_id, 
                     uint32_t num_entries, uint32_t swdata_len, 
                     bool thread_safe)
{
    uint32_t hwkey_len, hwkeymask_len;

    table_name_     = table_name;
    table_id_       = table_id;
    num_entries_    = num_entries;
    swdata_len_     = swdata_len;
    thread_safe_    = thread_safe;

    hwdata_len_ = 0;
    hwkey_len = 0;
    hwkeymask_len = 0;

    // Returns in bits
    p4pd_global_hwentry_query(table_id_, &hwkey_len, &hwkeymask_len, &hwdata_len_);

    // Rounding off to upper byte
    hwkey_len = (hwkey_len >> 3) + ((hwkey_len & 0x7) ? 1 : 0);
    hwkeymask_len = (hwkeymask_len >> 3) + ((hwkeymask_len & 0x7) ? 1 : 0);
    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);
        
    HAL_TRACE_DEBUG("DirectMap::{:<30}: tableid: {:<4} swdata_len: {:<4} hwdata_len_: {:<4}",
            table_name.c_str(), table_id, swdata_len_, hwdata_len_);

    // Initialize the indexer
    dm_indexer_ = new indexer(num_entries, thread_safe = thread_safe_);
    
    // Initialize for stats
    stats_ = new uint64_t[STATS_MAX]();
}

// ----------------------------------------------------------------------------
// Destructor - DirectMap
// ----------------------------------------------------------------------------
DirectMap::~DirectMap()
{
    delete dm_indexer_;
    delete[] stats_;
}

// ----------------------------------------------------------------------------
// Insert Entry in HW
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::insert(void *data, uint32_t *index)
{
    hal_ret_t       rs = HAL_RET_OK;
    p4pd_error_t    pd_err = P4PD_SUCCESS;

    rs = alloc_index_(index);
    if (rs != HAL_RET_OK) {
        goto end;
    }

    // Print entry
    // entry_trace_(data, *index);


    // P4-API: Write API
    pd_err = p4pd_global_entry_write(table_id_, *index, NULL, NULL, data); 
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

end:
    stats_update(INSERT, rs);
    return rs;
}

// ----------------------------------------------------------------------------
// Insert Entry in HW at index
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::insert_withid(void *data, uint32_t index)
{
    hal_ret_t rs = HAL_RET_OK;
    p4pd_error_t pd_err = P4PD_SUCCESS;

    rs = alloc_index_withid_(index);
    if (rs != HAL_RET_OK) {
        goto end;
    }

    // Print entry
    // entry_trace_(data, index);

    // P4-API: Write API
    pd_err = p4pd_global_entry_write(table_id_, index, NULL, NULL, data); 
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

end:
    stats_update(INSERT_WITHID, rs);
    return rs;
}

// ----------------------------------------------------------------------------
// Update Entry in HW
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::update(uint32_t index, void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    p4pd_error_t pd_err = P4PD_SUCCESS;

    if (index > num_entries_) {
        rs = HAL_RET_INVALID_ARG;
        goto end;
    }
    if (!dm_indexer_->is_alloced(index)) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    // Print entry
    // entry_trace_(data, index);

    // P4-API: Write API
    pd_err = p4pd_global_entry_write(table_id_, index, NULL, NULL, data); 
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

end:
    stats_update(UPDATE, rs);
    return rs;
}

// ----------------------------------------------------------------------------
// Remove Entry from HW
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::remove(uint32_t index)
{
    hal_ret_t       rs = HAL_RET_OK;
    p4pd_error_t    pd_err = P4PD_SUCCESS;
    void            *tmp_data = NULL;

    if (index > num_entries_) {
        rs = HAL_RET_INVALID_ARG;
        goto end;
    }
    if (!dm_indexer_->is_alloced(index)) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    tmp_data = ::operator new(swdata_len_);
    memset(tmp_data, 0, swdata_len_);
    HAL_TRACE_DEBUG("DirectMap::{}:index: {}", __FUNCTION__, index); 

    // Print entry
    // entry_trace_(tmp_data, index);

    // P4-API: Write API
    pd_err = p4pd_global_entry_write(table_id_, index, NULL, NULL, tmp_data); 
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

    rs = free_index_(index);
    if (rs != HAL_RET_OK) {
       goto end;
    }

end:
    if (tmp_data) ::operator delete(tmp_data);

    stats_update(REMOVE, rs);
    return rs;
}

// ----------------------------------------------------------------------------
// Retrieve Entry from HW
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::retrieve(uint32_t index, void *data)
{
    hal_ret_t       rs = HAL_RET_OK;
    p4pd_error_t    pd_err = P4PD_SUCCESS;

    if (index > num_entries_) {
        rs = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (!dm_indexer_->is_alloced(index)) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    // P4-API: Read API
    pd_err = p4pd_global_entry_read(table_id_, index, NULL, NULL, data); 
    // HAL_ASSERT_RETURN((pd_err == P4PD_SUCCESS), HAL_RET_HW_FAIL);
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

end:
    stats_update(RETRIEVE, rs);
    return rs;
}

// ----------------------------------------------------------------------------
// Iterate every entry and gives a call back
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::iterate(direct_map_iterate_func_t iterate_func, const void *cb_data)
{
    p4pd_error_t    pd_err = P4PD_SUCCESS;
    hal_ret_t       rs = HAL_RET_OK;
    void            *tmp_data = NULL;

    tmp_data = ::operator new(hwdata_len_);

    for (uint32_t i = 0; i < num_entries_; i++) {
        if (dm_indexer_->is_alloced(i)) {
            // P4-API: Read API
            pd_err = p4pd_global_entry_read(table_id_, i, NULL, NULL, tmp_data); 
            HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);
            iterate_func(i, tmp_data, cb_data);
        }
    }

end:
    ::operator delete(tmp_data);
    // return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

    stats_update(ITERATE, rs);
    return rs;
}


// ----------------------------------------------------------------------------
// Return Stats Pointer
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::fetch_stats(const uint64_t **stats)
{
    hal_ret_t   rs = HAL_RET_OK;

    *stats = stats_;

    return rs;
}



// ----------------------------------------------------------------------------
// Allocate an index 
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::alloc_index_(uint32_t *idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    // Allocate an index in repl. table
    indexer::status irs = dm_indexer_->alloc(idx);
    if (irs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    return rs;
}

// ----------------------------------------------------------------------------
// Allocate an index with id 
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::alloc_index_withid_(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    // Allocate an index in repl. table
    indexer::status irs = dm_indexer_->alloc_withid(idx);
    if (irs != indexer::SUCCESS) {
        rs = (irs == indexer::DUPLICATE_ALLOC) ? HAL_RET_DUP_INS_FAIL : HAL_RET_OOB;  
    }

    return rs;
}


// ----------------------------------------------------------------------------
// Free an index 
// ----------------------------------------------------------------------------
hal_ret_t
DirectMap::free_index_(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    indexer::status irs = dm_indexer_->free(idx);
    if (irs != indexer::SUCCESS) {
        rs = (irs == indexer::DUPLICATE_FREE) ? HAL_RET_DUP_FREE : HAL_RET_OOB;  
    }

    return rs;
}

// ----------------------------------------------------------------------------
// Increment Stats
// ----------------------------------------------------------------------------
void
DirectMap::stats_incr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    HAL_ATOMIC_INC_UINT32(&stats_[stat], 1);
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
DirectMap::stats_decr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    HAL_ATOMIC_DEC_UINT32(&stats_[stat], 1);
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
DirectMap::stats_update(DirectMap::api ap, hal_ret_t rs)
{
    switch (ap) {
        case INSERT:
            if(rs == HAL_RET_OK) stats_incr(STATS_INS_SUCCESS);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_INS_FAIL_HW);
            else if(rs == HAL_RET_NO_RESOURCE) stats_incr(STATS_INS_FAIL_NO_RES);
            break;
        case INSERT_WITHID:
            if(rs == HAL_RET_OK) stats_incr(STATS_INS_WITHID_SUCCESS);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_INS_WITHID_FAIL_HW);
            else if(rs == HAL_RET_DUP_INS_FAIL) stats_incr(STATS_INS_WITHID_FAIL_DUP_INS);
            else if(rs == HAL_RET_OOB) stats_incr(STATS_INS_WITHID_FAIL_OOB);
            break;
        case UPDATE:
            if(rs == HAL_RET_OK) stats_incr(STATS_UPD_SUCCESS);
            else if(rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
            else if(rs == HAL_RET_INVALID_ARG) stats_incr(STATS_UPD_FAIL_INV_ARG);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_UPD_FAIL_HW);
            break;
        case REMOVE:
            if (rs == HAL_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if (rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if (rs == HAL_RET_INVALID_ARG) stats_incr(STATS_REM_FAIL_INV_ARG);
            else if (rs == HAL_RET_HW_FAIL) stats_incr(STATS_REM_FAIL_HW);
            break;
        case RETRIEVE:
            if (rs == HAL_RET_OK) stats_incr(STATS_RETR_SUCCESS);
            else if (rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_RETR_FAIL_ENTRY_NOT_FOUND);
            else if (rs == HAL_RET_INVALID_ARG) stats_incr(STATS_RETR_FAIL_INV_ARG);
            else if (rs == HAL_RET_HW_FAIL) stats_incr(STATS_RETR_FAIL_HW);
            break;
        case ITERATE:
            if (rs == HAL_RET_HW_FAIL) stats_incr(STATS_ITER_FAIL_HW);
            else if (rs == HAL_RET_OK) stats_incr(STATS_ITER_SUCCESS);
            break;
        default:
            HAL_ASSERT(0);
    }
}


hal_ret_t
DirectMap::entry_trace_(void *data, uint32_t index)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_global_table_ds_decoded_string_get(table_id_,
            NULL, NULL, data, buff, sizeof(buff));
    HAL_ASSERT(p4_err == P4PD_SUCCESS);

    HAL_TRACE_DEBUG("Index: {} \n {}", index, buff);

    return HAL_RET_OK;
}

