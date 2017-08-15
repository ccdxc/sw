/*
 * ACL TCAM HW Table Library
 * --------------------------
 *
 * acl_tcam.cc
 * Vasanth Kumar (Pensando Systems)
 */
#include <cstring>

#include "tcam_entry.hpp"
#include "acl_tcam.hpp"
#include "p4pd_api.hpp"

using hal::pd::utils::TcamEntry;
using hal::pd::utils::acl_tcam;
using hal::pd::utils::priority_t;
using hal::pd::utils::prio_range_t;
using hal::pd::utils::move_chain_t;

bool priority_0_lowest_compare(priority_t p1, priority_t p2) { return p1>p2; }
bool priority_0_highest_compare(priority_t p1, priority_t p2) { return p1>p2; }

hal_ret_t
acl_tcam::init(std::string table_name, uint32_t table_id, uint32_t table_size,
               uint32_t swkey_len, uint32_t swdata_len, bool priority_0_lowest)
{
    table_name_ = table_name;
    table_id_ = table_id;
    tcam_size_ = table_size;
    swkey_len_ = swkey_len;
    swdata_len_ = swdata_len;

    tcam_entries_ = (TcamEntry **)HAL_CALLOC(HAL_MEM_ALLOC_LIB_ACL_TCAM,
                                             tcam_size_ * sizeof(TcamEntry*));
    if (!tcam_entries_) {
        return HAL_RET_OOM;
    }

    inuse_bmp_ = bitmap::factory(tcam_size_, true);
    if (!inuse_bmp_) {
        HAL_FREE(HAL_MEM_ALLOC_LIB_ACL_TCAM, tcam_entries_); 
        return HAL_RET_OOM;
    }

    tcam_prio_map_ = new tcam_prio_map(priority_0_lowest ?
                                       priority_0_lowest_compare : priority_0_highest_compare);
    if (!tcam_prio_map_) {
        HAL_FREE(HAL_MEM_ALLOC_LIB_ACL_TCAM, tcam_entries_); 
        delete inuse_bmp_;
        return HAL_RET_OOM;
    }

    handle_allocator = 0;
    hwkey_len_ = 0;
    hwkeymask_len_ = 0;
    hwdata_len_ = 0;
    p4pd_hwentry_query(table_id_, &hwkey_len_, &hwkeymask_len_,
                       &hwdata_len_);

    hwkey_len_ = (hwkey_len_+7) >> 3;
    hwkeymask_len_ = (hwkeymask_len_ + 7) >> 3;
    hwdata_len_ = (hwdata_len_ + 7) >> 3;

    HAL_TRACE_DEBUG("ACL-TCAM::{:<30}: tableid: {:<3} swkey_len: {:<4} "
                    "hwkey_len_: {:<4} hwkeymask_len_: {:<4} "
                    "hwdata_len_: {:<4}",
                    table_name.c_str(), table_id, swkey_len,
                    hwkey_len_, hwkeymask_len_, hwdata_len_);

    return HAL_RET_OK;
}

acl_tcam *
acl_tcam::factory(std::string table_name, uint32_t table_id, uint32_t table_size,
                  uint32_t swkey_len, uint32_t swdata_len,
                  bool priority_0_lowest)
{
    hal_ret_t ret;
    acl_tcam   *new_acl_tcam;

    new_acl_tcam= new acl_tcam();
    if (new_acl_tcam == NULL) {
        return NULL;
    }
    ret = new_acl_tcam->init(table_name, table_id, table_size,
                             swkey_len, swdata_len,
                             priority_0_lowest) ;
    if (ret != HAL_RET_OK) {
        delete new_acl_tcam;
        return NULL;
    }

    return new_acl_tcam;
}

acl_tcam::~acl_tcam()
{
    delete inuse_bmp_;
    delete tcam_prio_map_;
    if (tcam_entries_) { HAL_FREE(HAL_MEM_ALLOC_LIB_ACL_TCAM, tcam_entries_); }
}

hal_ret_t
acl_tcam::insert(void *key, void *key_mask, void *data,
                 priority_t priority, acl_tcam_entry_handle_t *handle_p)
{
    // Follow the below steps to insert an entry
    // 1. Find the start of the next priority and the end
    //                     of previous priority
    // 2. Find a free spot within the above 2
    // 3. If a free spot is found, no moves are required
    //
    // If moves are required:
    //
    // 1. Find free spots in both the directions (up and down where index 0 is the
    //    top most entry and size -1 is the bottom most entry)
    // 2. Figure out the best fit based on some heuristics
    // 3. Find out the priority of the entry one above/below the free space
    //    (based on if we're moving up/down accordingly)
    // 4. Move the "start/end" entry of the priority block to the free space
    // 5. Percolate upwards/downwards until the free space bubbles down to the
    //    space occupied by the target priority entry
    //
    // While moving,
    // - update the tcam_prio_map_ with the new start/end
    // - update the tcam_entries_ array and move the entries accordingly
    // - update the inuse_bmp_
    //
    // After adding the new entry
    // - Allocate a handle for the entry
    // - update the tcam_entry_map_ with the new entry and handle
    // - update the tcam_prio_map_ with the new start/end
    // - update the tcam_entries_ array and move the entries accordingly
    // - update the inuse_bmp_
    hal_ret_t ret = HAL_RET_OK;
    bool prev_exists = false, next_exists = false;
    uint32_t prev_end, next_start;
    acl_tcam_entry_handle_t handle;
    uint32_t target_up = 0;
    uint32_t target_down = tcam_size_ - 1;
    move_chain_t *move_chain = NULL;
    TcamEntry *tentry = NULL;
    uint32_t free_spot = tcam_size_;
    uint32_t target_spot;
    uint32_t num_moves = 0;

    // Allocate a handle and the tcam_entry
    handle = alloc_handle_();
    tentry = new TcamEntry(key, key_mask, swkey_len_, data, swdata_len_,
                           TCAM_ENTRY_INVALID_INDEX,
                           priority = priority);

    // Find the range of indexes allowed for this priority
    // by figuring out the prev and next priority entry indexes
    ret = find_allowed_range_(priority, &prev_exists, &prev_end,
                              &next_exists, &next_start);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} insert failed due to internal "
                      "error finding allowed range. Err {}",
                      __func__, __LINE__, table_name_.c_str(), ret);
        goto end;
    }

    // Find a free spot between prev_end and next_start
    if (prev_exists) {
        ret = inuse_bmp_->next_free(prev_end, &free_spot);
        if (ret != HAL_RET_OK) {
            // There is no space after prev_end. Maybe there's space
            // before
            free_spot = tcam_size_;
        }
    } else {
        ret = inuse_bmp_->first_free(&free_spot);
        if (ret != HAL_RET_OK) {
            // There is no space at all!
            HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} insert failed due to no space",
                          __func__, __LINE__, table_name_.c_str());
            goto end;
        }
    }

    if (next_exists) {
        target_down = next_start;
    } else if (prev_exists) {
        target_up = prev_end;
    }

    if ((free_spot >= tcam_size_) ||
        (next_exists && (free_spot > next_start)) ||
        (prev_exists && (free_spot < prev_end))) {
        // We need to move some entries
        // Create a move chain
        move_chain = create_move_chain_(target_up, target_down, &free_spot, &num_moves);
        if (move_chain == NULL) {
            // There is no space at all
            HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} insert failed due to no space",
                          __func__, __LINE__, table_name_.c_str());
            goto end;
        }
    }

    // Move the entries in the move chain
    ret = move_entries_(move_chain, free_spot, num_moves);
    if (ret != HAL_RET_OK) {
        // Oops moving failed
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} insert failed due to failure "
                      " in moving entries. Err: {}",
                      __func__, __LINE__, table_name_.c_str(), ret);
        goto end;
    }

    if (move_chain) {
        // If we moved entries, then the new entry has to be programmed
        // at the location we moved last
        target_spot = move_chain[num_moves-1];
    } else {
        // Program the entry at the free_spot we found
        target_spot = free_spot;
    }

    // Program the new entry
    ret = program_table_(tentry, target_spot);
    if (ret != HAL_RET_OK) {
        // Programming new entry failed. Roll back the moves
        unroll_moves_(move_chain, num_moves, num_moves);
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} insert failed due to failure "
                      " in programming. Err: {}",
                      __func__, __LINE__, table_name_.c_str(), ret);
        goto end;
    }

    // Update the state of the moved entries
    move_chain_state_update_(move_chain, free_spot, num_moves);

    // Update the state for the new entry
    set_entry_(target_spot, tentry);
    tcam_entry_map_[handle] = tentry;

    *handle_p = handle;

end:
    if (move_chain) {
        HAL_FREE(HAL_MEM_ALLOC_LIB_ACL_TCAM, move_chain);
    }
    if (ret != HAL_RET_OK) {
        tentry ? delete tentry : HAL_NOP;
    }
    return ret;
}

// ---------------------------------------------------------------------------
// Update
//
// Return Codes:
//
//		- HAL_RET_OK 				: Updated Succesfully
//		- HAL_RET_ENTRY_NOT_FOUND 	: Entry not found
// ---------------------------------------------------------------------------
hal_ret_t
acl_tcam::update(acl_tcam_entry_handle_t handle, void *data)
{
    hal_ret_t ret = HAL_RET_OK;
    TcamEntry *tentry = NULL;
    tcam_entry_map::iterator itr;

    // check if entry exists
    itr = tcam_entry_map_.find(handle);
    if (itr == tcam_entry_map_.end()) {
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} entry {} not found",
                      __func__, __LINE__, table_name_.c_str(), handle);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    tentry = itr->second;

    ret = program_table_(tentry, tentry->get_index(), data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Error in programming Table {} entry {} "
                      "Err: {}",
                      __func__, __LINE__, table_name_.c_str(), handle, ret);
        return ret;
    }

    // update the entry
    tentry->update_data(data);

    return ret;
}

// ---------------------------------------------------------------------------
// Tcam Remove
//
// Return Codes:
//
//		- HAL_RET_OK				: Removed Successfully
//		- HAL_RET_ENTRY_NOT_FOUND	: Entry not found
// ---------------------------------------------------------------------------
hal_ret_t
acl_tcam::remove(acl_tcam_entry_handle_t handle)
{
    hal_ret_t ret = HAL_RET_OK;
    TcamEntry *tentry = NULL;
    tcam_entry_map::iterator itr;

    // check if entry exists
    itr = tcam_entry_map_.find(handle);
    if (itr == tcam_entry_map_.end()) {
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} entry {} not found",
                      __func__, __LINE__, table_name_.c_str(), handle);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    tentry = itr->second;

    ret = deprogram_table_(tentry->get_index());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Error in programming Table {} entry {} "
                      "Err: {}",
                      __func__, __LINE__, table_name_.c_str(), handle, ret);
        return ret;
    }

    // Clean up the software data structures
    clear_entry_(tentry);
    delete itr->second;
    tcam_entry_map_.erase(itr);

    return ret;
}

// ---------------------------------------------------------------------------
// Tcam Retrieve
// ---------------------------------------------------------------------------
hal_ret_t
acl_tcam::retrieve(acl_tcam_entry_handle_t handle, void *key, void *key_mask,
                   void *data)
{
    hal_ret_t ret = HAL_RET_OK;
    TcamEntry *tentry = NULL;
    tcam_entry_map::iterator itr;

    // check if entry exists
    itr = tcam_entry_map_.find(handle);
    if (itr == tcam_entry_map_.end()) {
        HAL_TRACE_ERR("ACL-TCAM:{}:{}:: Table {} entry {} not found",
                      __func__, __LINE__, table_name_.c_str(), handle);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    tentry = itr->second;

    if (key) {
        memcpy(key, tentry->get_key(), tentry->get_key_len());
    }
    if (key_mask) {
        memcpy(key_mask, tentry->get_key_mask(), tentry->get_key_len());
    }
    if (data) {
        memcpy(data, tentry->get_data(), tentry->get_data_len());
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Tcam Iterate
// ---------------------------------------------------------------------------
hal_ret_t
acl_tcam::iterate(acl_tcam_iterate_func_t cb, const void *cb_data)
{
    hal_ret_t ret = HAL_RET_OK;
    TcamEntry *tentry = NULL;
    tcam_entry_map::iterator itr;

    for (itr = tcam_entry_map_.begin(); itr != tcam_entry_map_.end(); ++itr) {
        tentry = itr->second;
        cb(tentry->get_key(), tentry->get_key_mask(),
           tentry->get_data(), itr->first, tentry->get_index(), cb_data);
    }

    return ret;
}

hal_ret_t
acl_tcam::get_index(acl_tcam_entry_handle_t handle, uint32_t *index_p)
{
    TcamEntry *tentry = NULL;
    tcam_entry_map::iterator itr;

    // check if entry exists
    itr = tcam_entry_map_.find(handle);
    if (itr == tcam_entry_map_.end()) {
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    tentry = itr->second;

    *index_p = tentry->get_index();
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// DeProgram HW table
// ----------------------------------------------------------------------------
hal_ret_t
acl_tcam::deprogram_table_(uint32_t index)
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;
    void *hwkeymask     = NULL;
    void *swdata        = NULL;

    // Build Hw Keys/KeyMasks
    hwkey       = ::operator new(hwkey_len_);
    hwkeymask   = ::operator new(hwkeymask_len_);
    swdata      = ::operator new(swdata_len_);

    std::memset(hwkey, 0, hwkey_len_);
    std::memset(hwkeymask, 0, hwkeymask_len_);
    std::memset(swdata, 0, swdata_len_);

    // P4-API: write
    pd_err = p4pd_entry_write(table_id_, index, (uint8_t *)hwkey,
                              (uint8_t *)hwkeymask, swdata);
    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    if (hwkey) 		::operator delete(hwkey);
    if (hwkeymask) 	::operator delete(hwkeymask);
    if (swdata) 	::operator delete(swdata);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Program HW table
// ----------------------------------------------------------------------------
hal_ret_t
acl_tcam::program_table_(TcamEntry *te, uint32_t index, void *data)
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

    // P4-API: write
    pd_err = p4pd_entry_write(table_id_, index, (uint8_t *)hwkey,
                              (uint8_t *)hwkeymask, data ? data : te->get_data());
    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    if (hwkey) 		::operator delete(hwkey);
    if (hwkeymask) 	::operator delete(hwkeymask);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Create move chains. Find a way to create a free spot in either target_up
// or target_down and return the chain of moves that will help in coming there
// ----------------------------------------------------------------------------
move_chain_t *
acl_tcam::create_move_chain_(uint32_t target_up,
                             uint32_t target_down,
                             uint32_t *free_spot_p,
                             uint32_t *num_moves_p)
{
    bool up_full = false, down_full = false;
    uint32_t up_free, down_free;
    uint32_t move_count_down, move_count_up;
    uint32_t target;
    uint32_t num_moves;
    move_chain_t *move_chain = NULL;
    hal_ret_t ret = HAL_RET_OK;
    bool move_up = false;

    ret = inuse_bmp_->prev_free(target_up, &up_free);
    if (ret != HAL_RET_OK) {
        up_full = true;
    }

    ret = inuse_bmp_->next_free(target_down, &down_free);
    if (ret != HAL_RET_OK) {
        down_full = true;
    }

    if (up_full && down_full) {
        // Both directions are full
        return NULL;
    }

    // Pick a target free index based on the count of moves needed to reach
    // there
    if (up_full) {
        move_count_down = count_moves_down_(down_free, target_down);
        move_count_up = tcam_size_;
    } else if (down_full) {
        move_count_up = count_moves_up_(up_free, target_up);
        move_count_down = tcam_size_;
    } else {
        move_count_up = count_moves_up_(up_free, target_up);
        move_count_down = count_moves_down_(down_free, target_down);
    }

    // Pick either of the up_free/down_free based on number of moves needed
    if (move_count_up < move_count_down) {
        target = up_free;
        num_moves = move_count_up;
        move_up = true;
    } else {
        target = down_free;
        num_moves = move_count_down;
    }

    move_chain = (move_chain_t *)HAL_CALLOC(HAL_MEM_ALLOC_LIB_ACL_TCAM,
                                            num_moves * sizeof(move_chain_t));

    ret = populate_move_chain_(move_chain, num_moves, move_up, target);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    // The last move should be from either target_up or target_down so that
    // the new entry goes here
    if (move_up) {
        HAL_ASSERT_GOTO(target_up == move_chain[num_moves-1], cleanup);
    } else {
        HAL_ASSERT_GOTO(target_down == move_chain[num_moves-1], cleanup);
    }

    *num_moves_p = num_moves;
    *free_spot_p = target;
    return move_chain;

cleanup:
    if (move_chain) {
        HAL_FREE(HAL_MEM_ALLOC_LIB_ACL_TCAM, move_chain);
    }
    return NULL;
}

hal_ret_t
acl_tcam::move_entries_(move_chain_t *move_chain,
                        uint32_t free_spot,
                        uint32_t num_moves)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t i;
    uint32_t next_free;
    TcamEntry *tentry;

    next_free = free_spot;
    for (i = 0; i < num_moves; i++) {
        tentry = get_entry_(move_chain[i]);
        ret = program_table_(tentry, next_free);
        if (ret != HAL_RET_OK) {
            goto cleanup;
        }
        next_free = move_chain[i];
    }
    return HAL_RET_OK;
cleanup:
    unroll_moves_(move_chain, num_moves, i);
    // Now clear the entry at free spot
    deprogram_table_(free_spot);
    return ret;
}

void
acl_tcam::unroll_moves_(move_chain_t *move_chain,
                        uint32_t num_moves,
                        uint32_t completed_moves)
{
    uint32_t i;
    TcamEntry *tentry;

    // Unroll the hardware moves
    for (i = completed_moves; i > 0; i--) {
        tentry = get_entry_(move_chain[i-1]);
        program_table_(tentry, tentry->get_index());
    }
}

void
acl_tcam::move_chain_state_update_(move_chain_t *move_chain,
                                   uint32_t free_spot,
                                   uint32_t num_moves)
{
    // Update the state of the moved entries
    // - tentry index
    // - tcam_entries_
    // - tcam_prio_map_
    // - inuse_bmp_

    uint32_t i;
    TcamEntry *tentry;
    uint32_t next_free;

    next_free = free_spot;
    for (i = 0; i < num_moves; i++) {
        tentry = get_entry_(move_chain[i]);

        clear_entry_(tentry);
        set_entry_(next_free, tentry);
        next_free = move_chain[i];
    }
}

void
acl_tcam::set_entry_(uint32_t index,
                     TcamEntry *tentry)
{
    prio_range_t *prio_range = NULL;
    prio_range = get_prio_range_(tentry);

    if (prio_range == NULL) {
        prio_range = (prio_range_t*)HAL_CALLOC(HAL_MEM_ALLOC_LIB_ACL_TCAM,
                                               sizeof(prio_range_t));
        prio_range->start = index;
        prio_range->end = index;

        set_prio_range_(tentry, prio_range);
    } else {
        if (index < prio_range->start) {
            prio_range->start = index;
        }
        if (index > prio_range->end) {
            prio_range->end = index;
        }
    }

    tentry->set_index(index);
    inuse_bmp_->set(index);
    HAL_ASSERT(tcam_entries_[index] == NULL);
    tcam_entries_[index] = tentry;
}

void
acl_tcam::clear_entry_(TcamEntry *tentry)
{
    prio_range_t *prio_range;
    hal_ret_t ret;
    uint32_t index = tentry->get_index();

    prio_range = get_prio_range_(tentry);
    HAL_ASSERT((index >= prio_range->start) &&
               (index <= prio_range->end));
    if (prio_range->start == prio_range->end) {
        // We have only one entry in this priority range
        HAL_FREE(HAL_MEM_ALLOC_LIB_ACL_TCAM, prio_range);
        tcam_prio_map_->erase(tentry->get_priority());
    } else if (prio_range->start == index) {
        // New start is the next in use bit
        ret = inuse_bmp_->next_set(index, &prio_range->start);
        HAL_ASSERT(ret == HAL_RET_OK);
    } else if (prio_range->end == index) {
        // New end is the prev in use bit
        ret = inuse_bmp_->prev_set(index, &prio_range->end);
        HAL_ASSERT(ret == HAL_RET_OK);
    }

    inuse_bmp_->clear(index);
    tcam_entries_[index] = NULL;
}

hal_ret_t
acl_tcam::find_allowed_range_(priority_t priority,
                              bool *prev_exists_p, uint32_t *prev_end_p,
                              bool *next_exists_p, uint32_t *next_start_p)
{
    tcam_prio_map::iterator itr;
    bool prev_exists = false, next_exists = false;
    uint32_t prev_end, next_start;

    // Find the next higher priorities start
    itr = tcam_prio_map_->upper_bound(priority);
    if (itr != tcam_prio_map_->end()) {
        next_exists = true;
        next_start = itr->second->start;
    }

    // Find the lower priorities end index
    if (itr != tcam_prio_map_->begin()) {
        itr--;

        if (itr->first == priority) {
            if (itr != tcam_prio_map_->begin()) {
                itr--;

                prev_exists = true;
                prev_end = itr->second->end;
            }
        } else {
            prev_exists = true;
            prev_end = itr->second->end;
        }
    }

    *prev_exists_p = prev_exists;
    *prev_end_p = prev_end;
    *next_exists_p = next_exists;
    *next_start_p = next_start;
    return HAL_RET_OK;
}

uint32_t
acl_tcam::count_moves_up_(uint32_t up_free, uint32_t target)
{
    tcam_prio_map::iterator up;
    tcam_prio_map::iterator down;
    up = tcam_prio_map_->find(get_entry_(up_free+1)->get_priority());
    down = tcam_prio_map_->find(get_entry_(target)->get_priority());

    return std::distance(up, down) + 1;
}

uint32_t
acl_tcam::count_moves_down_(uint32_t down_free, uint32_t target)
{
    tcam_prio_map::iterator up;
    tcam_prio_map::iterator down;
    up = tcam_prio_map_->find(get_entry_(target)->get_priority());
    down = tcam_prio_map_->find(get_entry_(down_free-1)->get_priority());

    return std::distance(up, down) + 1;
}

hal_ret_t
acl_tcam::populate_move_chain_(move_chain_t *move_chain, uint32_t num_moves,
                               bool move_up, uint32_t target)
{
    TcamEntry *tentry;
    uint32_t i;

    // Populate a move chain to move towards target
    // num_moves is already calculated. So src is not needed
    if (move_up) {
        tentry = get_entry_(target+1);
    } else {
        tentry = get_entry_(target-1);
    }

    tcam_prio_map::iterator itr;
    itr = tcam_prio_map_->find(tentry->get_priority());
    HAL_ASSERT_GOTO(itr != tcam_prio_map_->end()
                    && "ACL-TCAM:Priority map entry doesn't exist", cleanup);

    if (!move_up) {
        itr++;
    }

    for (i = 0; i < num_moves; i++) {
        if (move_up) {
            HAL_ASSERT_GOTO(itr != tcam_prio_map_->end() &&
                            "ACL-TCAM:Priority map iter encountered end", cleanup);
            move_chain[i] = itr->second->end;
            itr++;
        } else {
            HAL_ASSERT_GOTO(itr != tcam_prio_map_->begin() &&
                            "ACL-TCAM:Priority map iter encountered begin", cleanup);
            itr--;
            move_chain[i] = itr->second->start;
        }
    }
    return HAL_RET_OK;
cleanup:
    return HAL_RET_ERR;
}

void
acl_tcam::set_prio_range_(TcamEntry *tentry, prio_range_t *prio_range)
{
    tcam_prio_map_->insert(std::make_pair(tentry->get_priority(), prio_range));
}

prio_range_t *
acl_tcam::get_prio_range_(TcamEntry *tentry)
{

    tcam_prio_map::iterator itr;
    itr = tcam_prio_map_->find(tentry->get_priority());
    if (itr != tcam_prio_map_->end()) {
        return itr->second;
    }
    return NULL;
}
