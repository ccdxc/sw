//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/mem.hpp"
#include "platform/utils/lif_mgr/lif_mgr.hpp"
#include "platform/capri/capri_qstate.hpp"

namespace sdk {
namespace platform {
namespace utils {

//-----------------------------------------------------------------------------
// Factory method to instantiate the class
//-----------------------------------------------------------------------------
lif_mgr *
lif_mgr::factory(uint32_t num_lifs, mpartition *mp, const char *kHBMLabel)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem     = NULL;
    lif_mgr *lm   = NULL;

    mem = (lif_mgr *) SDK_CALLOC(SDK_MEM_ALLOC_LIF_MGR, sizeof(lif_mgr));
    if (mem) {
        lm = new (mem) lif_mgr();
        ret = lm->init_(num_lifs, mp, kHBMLabel);
        if (ret != SDK_RET_OK) {
            lm->~lif_mgr();
            SDK_FREE(SDK_MEM_ALLOC_LIF_MGR, mem);
            lm = NULL;
        }
    } else {
        ret = SDK_RET_OOM;
    }
    return lm;
}

//-----------------------------------------------------------------------------
// Initializing fields in the class
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::init_(uint32_t num_lifs, mpartition *mp, const char *kHBMLabel)
{
    uint64_t hbm_addr = 0;
    uint32_t size = 0;
    uint32_t num_units;

    num_lifs_ = num_lifs;
    mp_ = mp;
    indexer_ = indexer::factory(num_lifs, true /* thread safe */,
                                true /* skip_zero */);
    if (mp_) {
        hbm_addr = mp_->start_addr(kHBMLabel);
        size = mp_->size(kHBMLabel);
    }
    num_units = (size) / kAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFFull) & ~0xFFFull;
        num_units--;
    }

    hbm_base_ = hbm_addr;
    hbm_indexer_ = indexer::factory(num_units, true /* thread safe */,
                                    true /* skip_zero */);
    SDK_TRACE_DEBUG("lif_mgr: Lif Mgr Init: num_lifs: %d, hbm_label: %s",
                    num_lifs_, kHBMLabel);
    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// Destroy lif manager
//-----------------------------------------------------------------------------
void
lif_mgr::destroy(lif_mgr *lm)
{
    lm->~lif_mgr();
    SDK_FREE(SDK_MEM_ALLOC_LIF_MGR, lm);
}

//-----------------------------------------------------------------------------
// Allocate the first free index
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::alloc_id(uint32_t *idx, uint32_t count)
{
    sdk_ret_t ret = SDK_RET_OK;

    indexer::status irs = indexer_->alloc_block(idx, count);
    if (irs != indexer::SUCCESS) {
        ret = SDK_RET_NO_RESOURCE;
        goto end;
    }
    SDK_TRACE_DEBUG("lif_mgr: Allocated lif_id: %u with count: %u",
                    *idx, count);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Reserve the index.
// Assumption: (start, (start + count -1)) are not aleady allocated.
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::reserve_id(uint32_t start, uint32_t count)
{
    sdk_ret_t ret = SDK_RET_OK;
    indexer::status irs;

    for (uint32_t i = start; i < (start + count); i++) {
        irs = indexer_->alloc_withid(i);
        if (irs != indexer::SUCCESS) {
            ret = (irs == indexer::DUPLICATE_ALLOC) ? SDK_RET_ENTRY_EXISTS : SDK_RET_NO_RESOURCE;
            goto end;
        }
    }
    SDK_TRACE_DEBUG("lif_mgr: Reserved lif_id: %u with count: %u",
                    start, count);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Free the index
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::free_id(uint32_t start, uint32_t count)
{
    sdk_ret_t ret = SDK_RET_OK;
    indexer::status irs;
    for (uint32_t i = start; i < (start + count); i++) {
        irs = indexer_->free(i);
        if (irs != indexer::SUCCESS) {
            ret = (irs == indexer::DUPLICATE_FREE) ? SDK_RET_DUPLICATE_FREE : SDK_RET_OOB;
            goto end;
        }
    }
    SDK_TRACE_DEBUG("lif_mgr: Freed lif_id: %u with count: %u",
                    start, count);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Lockless API to get qstate.
// - Supposed to be called only from other APIs and not public.
//-----------------------------------------------------------------------------
lif_qstate_t *
lif_mgr::get_lif_qstate_(uint32_t lif_id)
{
    lif_qstate_t *qstate = NULL;

    if (lif_id >= num_lifs_) {
        return qstate;
    }

    auto it = lifs_.find(lif_id);
    if (it == lifs_.end()) {
        goto end;
    }
    qstate = it->second;

end:
    return qstate;
}

//-----------------------------------------------------------------------------
// Get lif_qstate_t
//-----------------------------------------------------------------------------
lif_qstate_t *
lif_mgr::get_lif_qstate(uint32_t lif_id)
{
    lif_qstate_t *qstate = NULL;

    LIF_MGR_API_START_LOCK();

    qstate = get_lif_qstate_(lif_id);

    LIF_MGR_API_END_UNLOCK();
    return qstate;
}

//-----------------------------------------------------------------------------
// Gets Lif queue's Qstate address
//-----------------------------------------------------------------------------
int64_t
lif_mgr::get_lif_qstate_addr(uint32_t lif_id, uint32_t type, uint32_t qid)
{
    lif_qstate_t *qstate = NULL;
    int64_t qstate_addr = -1;

    if ((lif_id >= num_lifs_) || (type >= kNumQTypes)) {
        return -1;
    }
    LIF_MGR_API_START_LOCK();

    qstate = get_lif_qstate_(lif_id);
    if (!qstate) {
        goto end;
    }

    if (qid >= qstate->type[type].num_queues) {
        goto end;
    }

    qstate_addr = (int64_t)(qstate->hbm_address + qstate->type[type].hbm_offset +
                            (qid * qstate->type[type].qsize));

end:
    LIF_MGR_API_END_UNLOCK();
    return qstate_addr;
}

//-----------------------------------------------------------------------------
// Gets Lif queue's base Qstate address
//-----------------------------------------------------------------------------
int64_t
lif_mgr::get_lif_qstate_base_addr(uint32_t lif_id, uint32_t type)
{
    lif_qstate_t *qstate = NULL;
    int64_t qstate_addr = -1;

    if ((lif_id >= num_lifs_) || (type >= kNumQTypes)) {
        return -1;
    }

    LIF_MGR_API_START_LOCK();

    qstate = get_lif_qstate_(lif_id);
    if (!qstate) {
        goto end;
    }

    qstate_addr = (int64_t)(qstate->hbm_address + qstate->type[type].hbm_offset);

end:
    LIF_MGR_API_END_UNLOCK();
    return qstate_addr;
}

//-----------------------------------------------------------------------------
// Lif Initialization
// - Prepare software state for lif
// - Calculate sizes for qstate programming
// - Allocate hbm memory
// - Assumption: Lif should already been allocated or reserved in indexer.
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::init(lif_qstate_t *qstate)
{
    sdk_ret_t ret = SDK_RET_OK;
    indexer::status irs;
    uint32_t running_offset = 0;
    uint32_t cur_size;
    uint32_t num_entries;
    uint32_t i;
    uint32_t max_size = 0;
    uint32_t alloc_units;
    uint32_t alloc_offset;
    uint32_t lif_id = qstate->lif_id;
    std::pair<lif_map_t::iterator, bool> elem;
    lif_qstate_t *state = (lif_qstate_t *)SDK_CALLOC(SDK_MEM_ALLOC_LIF_QSTATE,
                                                     sizeof(lif_qstate_t));

    LIF_MGR_API_START_LOCK();

    memcpy(state, qstate, sizeof(lif_qstate_t));

    /*
     *  Find the max_size of any Qstate, and make sure size of every qstate
     * region is aligned to this max size. It means we might allocate more Qs
     * (entries) for a qtype than what is spec'ed in qtype info.
     */
    max_size = state->type[0].qtype_info.size;
    for (i = 1; i < kNumQTypes; i++) {
        if (state->type[i].qtype_info.size > max_size) {
            max_size = state->type[i].qtype_info.size;
        }
    }
    max_size = 1 << (max_size + 5);

    for (i = 0; i < kNumQTypes ; i++) {
        cur_size = state->type[i].qtype_info.size;
        cur_size = 1 << (cur_size + 5);
        state->type[i].hbm_offset = running_offset;
        state->type[i].qsize = cur_size;

        num_entries = state->type[i].qtype_info.entries;
        num_entries = 1 << num_entries;

        // Total memory for any Qstate region should be
        // minimum of max qsize among all qtypes.
        if (num_entries * cur_size < max_size) {
            num_entries = max_size / cur_size;
        }
        running_offset += (num_entries * cur_size);
        // Make sure num entries is a power of 2 and less than 16Mil
        assert((num_entries & (num_entries - 1)) == 0);
        assert(num_entries <= (16 * 1024 * 1024));

        // Update what was passed in.
        state->type[i].qtype_info.entries = __builtin_ffs(num_entries) - 1;
        state->type[i].num_queues = num_entries;
        state->type[i].coses = ((state->type[i].qtype_info.cosA & 0x0f) |
                                 ((state->type[i].qtype_info.cosB << 4) & 0xf0));
    }
    state->allocation_size = running_offset;

    // Insert into map
    elem = lifs_.insert(std::pair<uint32_t, lif_qstate_t*>(lif_id, state));
    if (elem.second == false) {
        SDK_TRACE_ERR("Duplicate init for lif: %d", state->lif_id);
        ret = SDK_RET_ENTRY_EXISTS;
    }

    // Compute hbm base
    alloc_units = (state->allocation_size + kAllocUnit - 1) & ~(kAllocUnit - 1);
    alloc_units /= kAllocUnit;
    irs = hbm_indexer_->alloc_block(&alloc_offset, alloc_units);
    if (irs != indexer::SUCCESS) {
        ret = SDK_RET_NO_RESOURCE;
        goto end;
    }
    allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kAllocUnit;
    state->hbm_address = hbm_base_ + alloc_offset;

end:
    LIF_MGR_API_END_UNLOCK();
    return ret;
}

//-----------------------------------------------------------------------------
// Write qstate
// - Program qstate
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::write_qstate(uint32_t lif_id, uint32_t type, uint32_t qid,
                      uint8_t *buf, uint32_t bufsize)
{
    sdk_ret_t ret                = SDK_RET_OK;
    lif_qstate_t *qstate         = NULL;
    uint64_t q_addr              = 0;
    mpartition_region_t *reg     = NULL;

    if ((lif_id >= num_lifs_) || (type >= kNumQTypes) ||
        (bufsize == 0) || (buf == nullptr)) {
        return SDK_RET_INVALID_ARG;
    }

    LIF_MGR_API_START_LOCK();

    auto it = lifs_.find(lif_id);
    if (it == lifs_.end()) {
        ret = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
    }
    qstate = it->second;
    if (qid > qstate->type[type].num_queues ||
        bufsize > qstate->type[type].qsize) {
        ret = SDK_RET_INVALID_ARG;
        goto end;
    }

    // Fill in the appropriate cos values for that qtype in buf at offset 2.
    if(bufsize > 2)  {
        buf[2] = qstate->type[type].coses;
    }

    q_addr = qstate->hbm_address + qstate->type[type].hbm_offset +
        (qid * qstate->type[type].qsize);


    sdk::asic::pd::asic_pd_qstate_write(q_addr, buf, bufsize);

    if (mp_) {
        reg = mp_->region_by_address(q_addr);
        SDK_ASSERT(reg != NULL);
    }

    ret = sdk::asic::pd::asic_pd_p4plus_invalidate_cache(reg, q_addr, bufsize);

end:
    LIF_MGR_API_END_UNLOCK();
    return ret;
}

//-----------------------------------------------------------------------------
// - Read qstate
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::read_qstate(uint32_t lif_id, uint32_t type, uint32_t qid,
                     uint8_t *buf, uint32_t bufsize)
{
    sdk_ret_t ret         = SDK_RET_OK;
    lif_qstate_t *qstate  = NULL;
    uint64_t q_addr       = 0;

    if ((lif_id >= num_lifs_) || (type >= kNumQTypes) ||
        (bufsize == 0) || (buf == nullptr)) {
        return SDK_RET_INVALID_ARG;
    }

    LIF_MGR_API_START_LOCK();

    auto it = lifs_.find(lif_id);
    if (it == lifs_.end()) {
        ret = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
    }
    qstate = it->second;
    if (qid > qstate->type[type].num_queues) {
        ret = SDK_RET_INVALID_ARG;
        goto end;
    }
    if (bufsize > qstate->type[type].qsize) {
        bufsize = qstate->type[type].qsize;
    }
    q_addr = qstate->hbm_address + qstate->type[type].hbm_offset +
        (qid * qstate->type[type].qsize);

    ret = sdk::asic::pd::asic_pd_qstate_read(q_addr, buf, bufsize);

end:
    LIF_MGR_API_END_UNLOCK();
    return ret;
}

#define QSTATE_INFO(QID) \
    qstate->type[QID].qtype_info.entries, qstate->type[QID].qtype_info.size

sdk_ret_t
lif_mgr::read_qstate_map(uint32_t lif_id, lif_qstate_t *qstate)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!qstate) {
        return SDK_RET_INVALID_ARG;
    }

    qstate->lif_id = lif_id;
    LIF_MGR_API_START_LOCK();
    ret = asic_pd_qstate_map_read(qstate);
    LIF_MGR_API_END_UNLOCK();

    return ret;
}

//-----------------------------------------------------------------------------
// - Clear qstate
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::clear_qstate(uint32_t lif_id)
{
    sdk_ret_t ret = SDK_RET_OK;
    lif_qstate_t *qstate = NULL;

    LIF_MGR_API_START_LOCK();

    qstate = get_lif_qstate_(lif_id);
    if (!qstate) {
        ret = SDK_RET_INVALID_ARG;
        goto end;
    }
    ret = sdk::asic::pd::asic_pd_qstate_clear(qstate);

end:
    LIF_MGR_API_END_UNLOCK();
    return ret;
}

//-----------------------------------------------------------------------------
// Lif Enable
// - Program qstate map
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::enable(uint32_t lif_id)
{
    sdk_ret_t ret = SDK_RET_OK;
    lif_qstate_t *qstate = NULL;

    LIF_MGR_API_START_LOCK();

    qstate = get_lif_qstate_(lif_id);
    if (!qstate) {
        ret = SDK_RET_INVALID_ARG;
        goto end;
    }
    ret = sdk::asic::pd::asic_pd_qstate_map_write(qstate, true);
end:
    LIF_MGR_API_END_UNLOCK();
    return ret;
}

//-----------------------------------------------------------------------------
// Lif Disable
// - DeProgram qstate map
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::disable(uint32_t lif_id)
{
    return sdk::asic::pd::asic_pd_qstate_map_clear(lif_id);
}

//-----------------------------------------------------------------------------
// Remove Lif.
// - Clear qstate map and qstate
// - Free up hbm memory
// - Clean up local state & free up lif id
//-----------------------------------------------------------------------------
sdk_ret_t
lif_mgr::remove(uint32_t lif_id)
{
    sdk_ret_t ret = SDK_RET_OK;
    indexer::status irs;
    uint32_t alloc_offset = 0;
    uint32_t alloc_units = 0;
    lif_qstate_t *qstate = NULL;

    LIF_MGR_API_START_LOCK();

    qstate = get_lif_qstate_(lif_id);
    if (!qstate) {
        ret = SDK_RET_INVALID_ARG;
        goto end;
    }

    // zero out qstate map
    ret = sdk::asic::pd::asic_pd_qstate_map_clear(lif_id);

    // zero out qstate
    ret = sdk::asic::pd::asic_pd_qstate_clear(qstate);

    // Free up hbm memory
    alloc_offset = qstate->hbm_address - hbm_base_;
    if (allocation_sizes_.find(alloc_offset) != allocation_sizes_.end()) {
        alloc_units = allocation_sizes_[alloc_offset];
        for (uint32_t i = alloc_offset; i < (alloc_offset + alloc_units); i++) {
            irs = hbm_indexer_->free(i);
            if (irs != indexer::SUCCESS) {
                ret = (irs == indexer::DUPLICATE_FREE) ? SDK_RET_DUPLICATE_FREE : SDK_RET_OOB;
                goto end;
            }
        }
        allocation_sizes_.erase(alloc_offset);
    }

    // Remove from map
    lifs_.erase(lif_id);

    // Free up lif id
    ret = free_id(lif_id, 1);

end:
    LIF_MGR_API_END_UNLOCK();
    return ret;
}

// TODO : Handle HAL/nicmgr process restarts
sdk_ret_t
lif_mgr::lifs_reset(uint32_t start_lif, uint32_t end_lif)
{
    for (uint32_t lif_id = start_lif; lif_id <= end_lif; lif_id++) {
        sdk::platform::capri::capri_reset_qstate_map(lif_id);
    }

    return SDK_RET_OK;
}


} // namespace utils
} // namespace platform
} // namespace sdk
