#include <cstring>

#include "tcam_entry.hpp"
#include "tcam.hpp"
#include "p4pd_api.hpp"

using hal::pd::utils::TcamEntry;
using hal::pd::utils::Tcam;

// ---------------------------------------------------------------------------
// Constructor - Tcam
// ---------------------------------------------------------------------------
Tcam::Tcam(std::string table_name, uint32_t table_id, 
           uint32_t tcam_capacity, uint32_t swkey_len, uint32_t swdata_len,
           bool allow_dup_insert)
{
    table_name_         = table_name;
    table_id_           = table_id;
    tcam_capacity_      = tcam_capacity;
    swkey_len_          = swkey_len;
    swdata_len_         = swdata_len;
    allow_dup_insert_   = allow_dup_insert;

    tcam_indexer_       = new indexer(tcam_capacity_);

    hwkey_len_ = 0;
    hwkeymask_len_ = 0;
    hwdata_len_ = 0;
    p4pd_hwentry_query(table_id_, &hwkey_len_, &hwkeymask_len_, 
                       &hwdata_len_);

    hwkey_len_ = (hwkey_len_ >> 3) + ((hwkey_len_ & 0x7) ? 1 : 0);
    hwkeymask_len_ = (hwkeymask_len_ >> 3) + ((hwkeymask_len_ & 0x7) ? 1 : 0);
    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);

    HAL_TRACE_DEBUG("Tcam::{:<30}: tableid: {:<3} swkey_len: {:<4} "
                    "hwkey_len_: {:<4} hwkeymask_len_: {:<4} "
                    "hwdata_len_: {:<4}",
                    table_name.c_str(), table_id, swkey_len, 
                    hwkey_len_, hwkeymask_len_, hwdata_len_);

    // TODO: Remove once P4-PD APIs are implemented
    // hwkey_len_ = hwkeymask_len_ = swkey_len;
    // hwdata_len_ = swdata_len;

    // Initialize for Stats
    stats_ = new uint64_t[STATS_MAX]();
}

// ---------------------------------------------------------------------------
// Destructor - Tcam
// ---------------------------------------------------------------------------
Tcam::~Tcam() 
{
    delete tcam_indexer_;
    delete[] stats_;
}

// ---------------------------------------------------------------------------
// Insert
//
// Return Codes:
//
//      - HAL_RET_OK            : Insert Successfully
//      - HAL_RET_NO_RESOURCE    : No more space 
//      - HAL_RET_DUP_INS_FAIL  : Duplicate Insert
//
// ---------------------------------------------------------------------------
hal_ret_t 
Tcam::insert(void *key, void *key_mask, void *data, uint32_t *index, bool lowest)
{
    hal_ret_t rs        = HAL_RET_OK;
    TcamEntry *te       = NULL;
    
    // check if entry already exists
    if (!allow_dup_insert_ && tcam_entry_exists_(key, key_mask, swkey_len_)) {
        return HAL_RET_DUP_INS_FAIL;
    }

    // alloc index
    rs = alloc_index_(index, lowest);
    if (rs != HAL_RET_OK) {
        goto end;
    }

    HAL_TRACE_DEBUG("TCAM: Table: {} Insert at {}", table_name_.c_str(), *index);
    te = new TcamEntry(key, key_mask, swkey_len_, data, swdata_len_, *index); 

    // program hw
    rs = program_table_(te);

    if (rs == HAL_RET_OK) {
        // insert in sw DS
        tcam_entry_map_[*index] = te;
    } else {
        delete te;
        free_index_(*index);
    }

end:
    stats_update(INSERT, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Insert withid
//
// Return Codes:
//
//      - HAL_RET_OK            : Insert Successfully
//      - HAL_RET_NO_RESOURCE    : No more space 
//      - HAL_RET_DUP_INS_FAIL  : Duplicate Insert
//
// ---------------------------------------------------------------------------
hal_ret_t 
Tcam::insert_withid(void *key, void *key_mask, void *data, uint32_t index)
{
    hal_ret_t rs        = HAL_RET_OK;
    TcamEntry *te       = NULL;
    
    // check if entry already exists
    if (!allow_dup_insert_ && tcam_entry_exists_(key, key_mask, swkey_len_)) {
        HAL_TRACE_DEBUG("Tcam::{}: Keys Match!!", table_name_.c_str());
        return HAL_RET_DUP_INS_FAIL;
    }

    // alloc index
    rs = alloc_index_withid_(index);
    if (rs != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Tcam::{}: Index already alloced!!", table_name_.c_str());
        goto end;
    }

    te = new TcamEntry(key, key_mask, swkey_len_, data, swdata_len_, index); 

    // program hw
    rs = program_table_(te);

    if (rs == HAL_RET_OK) {
        // insert in sw DS
        tcam_entry_map_[index] = te;
    } else {
        delete te;
        free_index_(index);
    }

end:
    stats_update(INSERT_WITHID, rs);
    return rs;
}
// ---------------------------------------------------------------------------
// Update
//
// Return Codes:
//
//        - HAL_RET_OK                 : Updated Succesfully
//        - HAL_RET_OOB                : Out of bound index
//        - HAL_RET_ENTRY_NOT_FOUND     : Entry not found
// ---------------------------------------------------------------------------
hal_ret_t 
Tcam::update(uint32_t tcam_idx, void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    TcamEntryMap::iterator itr;

    // check if idx is OOB
    if (tcam_idx >= tcam_capacity_) {
        rs = HAL_RET_OOB;
        goto end;
    }

    // check if entry exists
    itr = tcam_entry_map_.find(tcam_idx);
    if (itr == tcam_entry_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    // update sw DS
    itr->second->update_data(data);

    // program hw
    rs = program_table_(itr->second);

end:
    stats_update(UPDATE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Tcam Remove
//
// Return Codes:
//    
//        - HAL_RET_OK                : Removed Successfully
//        - HAL_RET_OOB                : Out of Bound Index
//        - HAL_RET_ENTRY_NOT_FOUND    : Entry not found
// ---------------------------------------------------------------------------
hal_ret_t 
Tcam::remove(uint32_t tcam_idx)
{
    hal_ret_t rs = HAL_RET_OK;
    TcamEntryMap::iterator itr;

    // check if idx is OOB
    if (tcam_idx >= tcam_capacity_) {
        rs = HAL_RET_OOB;
        goto end;
    }

    // check if entry exists
    itr = tcam_entry_map_.find(tcam_idx);
    if (itr == tcam_entry_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }
    
    // de-program hw
    rs = deprogram_table_(itr->second);

    if (rs == HAL_RET_OK) {
        // free & remove from sw DS
        delete itr->second;
        tcam_entry_map_.erase(itr);

        // free index
        rs = free_index_(tcam_idx);
        if (rs != HAL_RET_OK) {
            goto end;
        }
    }

end:
    stats_update(REMOVE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Tcam Retrieve
// ---------------------------------------------------------------------------
hal_ret_t
Tcam::retrieve(uint32_t tcam_idx, void *key, void *key_mask, void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    TcamEntryMap::iterator itr;
    TcamEntry *te = NULL;

    // check if idx is OOB
    if (tcam_idx >= tcam_capacity_) {
        rs = HAL_RET_OOB;
        goto end;
    }

    // check if entry exists
    itr = tcam_entry_map_.find(tcam_idx);
    if (itr == tcam_entry_map_.end()) {
        rs = HAL_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    te = itr->second;

    if (key) {
        memcpy(key, te->get_key(), te->get_key_len());
    }
    if (key_mask) {
        memcpy(key_mask, te->get_key_mask(), te->get_key_len());
    }
    if (data) {
        memcpy(data, te->get_data(), te->get_data_len());
    }


#if 0
    if (key) *key           = te->get_key();
    if (key_mask) *key_mask = te->get_key_mask();
    // if (key_len) *key_len   = te->get_key_len();
    if (data) *data         = te->get_data();
    // if (data_len) *data_len = te->get_data_len();
#endif

end:
    stats_update(RETRIEVE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Tcam Iterate
// ---------------------------------------------------------------------------
hal_ret_t
Tcam::iterate(tcam_iterate_func_t cb, const void *cb_data)
{
    hal_ret_t rs = HAL_RET_OK;
    TcamEntryMap::iterator itr;
    TcamEntry *te = NULL;

    for (itr = tcam_entry_map_.begin(); itr != tcam_entry_map_.end(); ++itr) {
        te = itr->second;
        cb(te->get_key(), te->get_key_mask(),
           te->get_data(), itr->first, cb_data);
    }

#if 0
    for (uint32_t i = 0; i < tcam_capacity_; i++) {
        if (tcam_table[i] != NULL) {
            cb(tcam_table[i]->get_key(), tcam_table[i]->get_key_mask(), 
               tcam_table[i]->get_key_len(), tcam_table[i]->get_data(), 
               tcam_table[i]->get_data_len(), i, cb_data);
        }
    }
#endif
    return rs;
}


// ---------------------------------------------------------------------------
// True: Tcam Entry is present.
// ---------------------------------------------------------------------------
bool
Tcam::tcam_entry_exists_(void *key, void *key_mask, uint32_t key_len)
{
    TcamEntryMap::iterator itr;
    TcamEntry *tmp_te = NULL;
    for (itr = tcam_entry_map_.begin(); itr != tcam_entry_map_.end(); ++itr) {
        tmp_te = itr->second;
        if (key_len == tmp_te->get_key_len() &&
                !std::memcmp(key, tmp_te->get_key(), 
                             key_len) &&
                !std::memcmp(key_mask, tmp_te->get_key_mask(), 
                             key_len)) {
            return TRUE;
        }

    }
    return FALSE;
}


// ----------------------------------------------------------------------------
// Program HW table
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::program_table_(TcamEntry *te) 
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;
    void *hwkeymask     = NULL;

    if (!te) {
        return HAL_RET_INVALID_ARG;
    }

    // build hw key & mask
    hwkey       = ::operator new(hwkey_len_);
    hwkeymask   = ::operator new(hwkeymask_len_);

    std::memset(hwkey, 0, hwkey_len_);
    std::memset(hwkeymask, 0, hwkeymask_len_);

    pd_err = p4pd_hwkey_hwmask_build(table_id_, te->get_key(), te->get_key_mask(), 
                                     (uint8_t *)hwkey, (uint8_t *)hwkeymask);

    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

    // Entry trace
    entry_trace_(te);

    // P4-API: write
    pd_err = p4pd_entry_write(table_id_, te->get_index(), (uint8_t *)hwkey, 
                              (uint8_t *)hwkeymask, te->get_data());
    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    if (hwkey)         ::operator delete(hwkey);
    if (hwkeymask)     ::operator delete(hwkeymask);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// DeProgram HW table
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::deprogram_table_(TcamEntry *te) 
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;
    void *hwkeymask     = NULL;

    if (!te) {
        return HAL_RET_INVALID_ARG;
    }

    // Build Hw Keys/KeyMasks
    hwkey       = ::operator new(hwkey_len_);
    hwkeymask   = ::operator new(hwkeymask_len_);

    std::memset(hwkey, 0xFF, hwkey_len_);
    std::memset(hwkeymask, 0xFf, hwkeymask_len_);
    std::memset(te->get_data(), 0, swdata_len_);

    // P4-API: write
    pd_err = p4pd_entry_write(table_id_, te->get_index(), (uint8_t *)hwkey, 
                              (uint8_t *)hwkeymask, te->get_data());
    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    if (hwkey)         ::operator delete(hwkey);
    if (hwkeymask)     ::operator delete(hwkeymask);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
}


// ----------------------------------------------------------------------------
// Return Stats Pointer
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::fetch_stats(const uint64_t **stats)
{
    hal_ret_t   rs = HAL_RET_OK;

    *stats = stats_;

    return rs;
}

// ----------------------------------------------------------------------------
// Allocate an index
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::alloc_index_(uint32_t *idx, bool lowest)
{
    hal_ret_t   rs = HAL_RET_OK;
    
    // Allocate an index in repl. table
    indexer::status irs = tcam_indexer_->alloc(idx, lowest, 1);
    if (irs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    return rs;
}

// ----------------------------------------------------------------------------
// Allocate an index with id
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::alloc_index_withid_(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;
    
    // Allocate an index in repl. table
    indexer::status irs = tcam_indexer_->alloc_withid(idx);
    if (irs != indexer::SUCCESS) {
        rs = (irs == indexer::DUPLICATE_ALLOC) ? HAL_RET_DUP_INS_FAIL : HAL_RET_OOB;
    }

    return rs;
}


// ----------------------------------------------------------------------------
// Free an index
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::free_index_(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    indexer::status irs = tcam_indexer_->free(idx);
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
Tcam::stats_incr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
Tcam::stats_decr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
Tcam::stats_update(Tcam::api ap, hal_ret_t rs)
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
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_UPD_FAIL_HW);
            break;
        case REMOVE:
            if (rs == HAL_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if (rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if (rs == HAL_RET_HW_FAIL) stats_incr(STATS_REM_FAIL_HW);
            break;
        case RETRIEVE:
            if (rs == HAL_RET_OK) stats_incr(STATS_RETR_SUCCESS);
            else if (rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_RETR_FAIL_ENTRY_NOT_FOUND);
            break;
        default:
            HAL_ASSERT(0);
    }
}

// ----------------------------------------------------------------------------
// Print entry
// ----------------------------------------------------------------------------
hal_ret_t
Tcam::entry_trace_(TcamEntry *te)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(table_id_,
            te->get_key(), te->get_key_mask(), te->get_data(), 
            buff, sizeof(buff));
    HAL_ASSERT(p4_err == P4PD_SUCCESS);

    HAL_TRACE_DEBUG("Index: {} \n {}", te->get_index(), buff);

    return HAL_RET_OK;
}
