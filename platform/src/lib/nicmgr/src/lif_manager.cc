//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <errno.h>
#include <assert.h>
#include "capri_hbm.hpp"
#include "capri_tbl_rw.hpp"
#include "capri_qstate.hpp"
#include "lif_manager.hpp"
#include "logger.hpp"
#include "nic/sdk/include/sdk/platform/utils/mpartition.hpp"

const static char *kHBMLabel = "nicmgrqstate_map";
const static uint32_t kAllocUnit = 4096;

static p4plus_cache_action_t
capri_hbm_cache_pipe_to_action (sdk::platform::utils::mpartition_cache_pipe_t cache_pipe)
{
    if (cache_pipe == sdk::platform::utils::MPARTITION_CACHE_PIPE_P4PLUS_RXDMA) {
        return (P4PLUS_CACHE_INVALIDATE_RXDMA);
    } else if (cache_pipe == sdk::platform::utils::MPARTITION_CACHE_PIPE_P4PLUS_TXDMA) {
        return (P4PLUS_CACHE_INVALIDATE_TXDMA);
    } else if (cache_pipe == sdk::platform::utils::MPARTITION_CACHE_PIPE_P4PLUS_ALL) {
        return (P4PLUS_CACHE_INVALIDATE_BOTH);
    }
    return (P4PLUS_CACHE_ACTION_NONE);
}

LIFManagerBase::LIFManagerBase() :
        lif_allocator_(kNumMaxLIFs) {}

int32_t LIFManagerBase::LIFRangeAlloc(int32_t start, uint32_t count) {
    if (count == 0)
        return -EINVAL;

    std::lock_guard<std::mutex> lk(lk_);
    int32_t base;
    if (start < 0) {
        base = lif_allocator_.Alloc(count);
        if (base < 0)
            return -ENOSPC;
    } else {
        if ((start + count) >= kNumMaxLIFs)
            return -EINVAL;
        base = lif_allocator_.CheckAndReserve(start, count);
        if (base < 0)
            return -ENOSPC;
    }


    for (uint32_t i = 0; i < count; i++) {
        // Accessing an entry will allocate it.
        NIC_LOG_INFO("Populating allocated_lifs for hw_lif_id: {}", base+i);
        alloced_lifs_[base + i].lif_id = base + i;
    }

    return base;
}

void LIFManagerBase::DeleteLIF(int32_t hw_lif_id) {
    if ((uint32_t) hw_lif_id >= kNumMaxLIFs)
        return;
    std::lock_guard<std::mutex> lk(lk_);
    LIFQState *qstate = GetLIFQState(hw_lif_id);
    if (qstate != nullptr) {
        DeleteLIFQStateImpl(qstate);
        alloced_lifs_.erase(hw_lif_id);
    }
    lif_allocator_.Free(hw_lif_id, 1);
}

LIFQState *LIFManagerBase::GetLIFQState(uint32_t lif_id) {
    if (lif_id >= kNumMaxLIFs)
        return nullptr;

    auto it = alloced_lifs_.find(lif_id);
    if (it == alloced_lifs_.end())
        return nullptr;

    assert(it->second.lif_id == lif_id);
    return &it->second;
}

int32_t
LIFManagerBase::InitLIFQState(uint32_t lif_id,
                              LIFQStateParams *params,
                              uint8_t hint_cos) {
    if (lif_id >= kNumMaxLIFs)
        return -EINVAL;
    std::lock_guard<std::mutex> lk(lk_);
    LIFQState *qstate = GetLIFQState(lif_id);
    if (qstate == nullptr) {
        NIC_LOG_ERR("Unable to get lifqstate for hw_lif_id: {}", lif_id);
        return -ENOENT;
    }

    uint32_t running_offset = 0;
    uint32_t cur_size;
    uint32_t num_entries;
    uint32_t i;
    uint32_t max_size = params->type[0].size;
    // Find the max_size of any Qstate, and make sure
    // beg of every qstate region is aligned to this
    // max size. It means we might allocate more Qs
    // (entries) for a qtype than what is spec'ed in
    // the params
    for (i = 1; i < kNumQTypes; i++) {
        if (params->type[i].size > max_size) {
            max_size = params->type[i].size;
        }
    }
    max_size = 1 << (max_size + 5);

    for (i = 0; i < kNumQTypes; i++) {
        cur_size = params->type[i].size;
        cur_size = 1 << (cur_size + 5);
        qstate->type[i].hbm_offset = running_offset;
        qstate->type[i].qsize = cur_size;

        num_entries = params->type[i].entries;
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
        params->type[i].entries = __builtin_ffs(num_entries) - 1;
        qstate->type[i].num_queues = num_entries;
        qstate->type[i].coses = ((params->type[i].cosA & 0x0f) |
                                 ((params->type[i].cosB << 4) & 0xf0));
    }
    qstate->allocation_size = running_offset;
    // Cache the params.
    qstate->params_in = *params;

    int32_t ret = InitLIFQStateImpl(qstate, hint_cos);
    if (ret < 0) {
        NIC_LOG_ERR("InitLIFQStateImpl failure. ret: {}", ret);
        return ret;
    }

    return ret;
}

int64_t LIFManagerBase::GetLIFQStateAddr(
        uint32_t lif_id, uint32_t type, uint32_t qid) {
    if ((lif_id >= kNumMaxLIFs) || (type >= kNumQTypes))
        return -EINVAL;
    std::lock_guard<std::mutex> lk(lk_);
    LIFQState *qstate = GetLIFQState(lif_id);
    if (qstate == nullptr)
        return -ENOENT;
    if (qid >= qstate->type[type].num_queues)
        return -EINVAL;
    return (int64_t) (qstate->hbm_address + qstate->type[type].hbm_offset +
                      (qid * qstate->type[type].qsize));
}

int64_t LIFManagerBase::GetLIFQStateBaseAddr(
        uint32_t lif_id, uint32_t type) {
    if ((lif_id >= kNumMaxLIFs) || (type >= kNumQTypes))
        return -EINVAL;
    std::lock_guard<std::mutex> lk(lk_);
    LIFQState *qstate = GetLIFQState(lif_id);
    if (qstate == nullptr)
        return -ENOENT;
    return qstate->hbm_address + qstate->type[type].hbm_offset;
}

int32_t LIFManagerBase::ReadQState(
        uint32_t lif_id, uint32_t type, uint32_t qid, uint8_t *buf,
        uint32_t bufsize) {
    if ((lif_id >= kNumMaxLIFs) || (type >= kNumQTypes) || (bufsize == 0) ||
        (buf == nullptr)) {
        return -EINVAL;
    }
    std::lock_guard<std::mutex> lk(lk_);
    LIFQState *qstate = GetLIFQState(lif_id);
    if (qstate == nullptr)
        return -ENOENT;
    if (qid >= qstate->type[type].num_queues)
        return -EINVAL;
    if (bufsize > qstate->type[type].qsize)
        bufsize = qstate->type[type].qsize;
    uint64_t q_addr = qstate->hbm_address + qstate->type[type].hbm_offset +
                      (qid * qstate->type[type].qsize);
    return ReadQStateImpl(q_addr, buf, bufsize);
}

int32_t LIFManagerBase::WriteQState(
        uint32_t lif_id, uint32_t type, uint32_t qid, uint8_t *buf,
        uint32_t bufsize) {
    if ((lif_id >= kNumMaxLIFs) || (type >= kNumQTypes) || (bufsize == 0) ||
        (buf == nullptr)) {
        return -EINVAL;
    }
    std::lock_guard<std::mutex> lk(lk_);
    LIFQState *qstate = GetLIFQState(lif_id);
    if (qstate == nullptr)
        return -ENOENT;
    if (qid >= qstate->type[type].num_queues)
        return -EINVAL;
    if (bufsize > qstate->type[type].qsize)
        return -EINVAL;

    // Fill in the appropriate cos values for that qtype in buf at offset 2.
    if (bufsize > 2) {
        buf[2] = qstate->type[type].coses;
    }

    uint64_t q_addr = qstate->hbm_address + qstate->type[type].hbm_offset +
                      (qid * qstate->type[type].qsize);
    return WriteQStateImpl(q_addr, buf, bufsize);
}

void NicLIFManager::destroy(NicLIFManager *lm) {
    assert(0);
    delete lm;
}

NicLIFManager* NicLIFManager::factory(sdk::platform::utils::mpartition *mp,
                                      sdk::platform::program_info *pinfo) {
    assert(mp);
    assert(pinfo);

    NicLIFManager *lm = new NicLIFManager();
    assert(lm);

    lm->mp_ = mp;
    lm->pinfo_ = pinfo;

    uint64_t hbm_addr = lm->mp_->start_addr(kHBMLabel);
    assert(hbm_addr > 0);

    uint32_t size = lm->mp_->size_kb(kHBMLabel);
    uint32_t num_units = (size * 1024) / kAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFFull) & ~0xFFFull;
        num_units--;
    }

    lm->hbm_base_ = hbm_addr;
    lm->hbm_allocator_.reset(new hal::BMAllocator(num_units));

    // Allocate 0th hw_lif_id
    uint32_t hw_lif_id = lm->LIFRangeAlloc(-1, 1);
    NIC_LOG_INFO("Reserving hw_lif_id: {}", hw_lif_id);

    return lm;
}

int32_t NicLIFManager::InitLIFQStateImpl(LIFQState *qstate, int cos) {
    uint32_t alloc_units;

    alloc_units = (qstate->allocation_size + kAllocUnit - 1) & ~(kAllocUnit - 1);
    alloc_units /= kAllocUnit;
    int alloc_offset = hbm_allocator_->Alloc(alloc_units);
    if (alloc_offset < 0)
        return -ENOMEM;
    allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kAllocUnit;
    qstate->hbm_address = hbm_base_ + alloc_offset;

    push_qstate_to_capri(qstate, cos);

    return 0;
}

void NicLIFManager::DeleteLIFQStateImpl(LIFQState *qstate) {

    clear_qstate(qstate);

    int alloc_offset = qstate->hbm_address - hbm_base_;
    if (allocation_sizes_.find(alloc_offset) != allocation_sizes_.end()) {
        hbm_allocator_->Free(alloc_offset, allocation_sizes_[alloc_offset]);
        allocation_sizes_.erase(alloc_offset);
    }
}

int32_t NicLIFManager::ReadQStateImpl(
        uint64_t q_addr, uint8_t *buf, uint32_t q_size) {

    read_qstate(q_addr, buf, q_size);

    return 0;
}

int32_t NicLIFManager::WriteQStateImpl(
        uint64_t q_addr, const uint8_t *buf, uint32_t q_size) {
    p4plus_cache_action_t action = P4PLUS_CACHE_ACTION_NONE;
    sdk::platform::utils::mpartition_region_t *reg = NULL;

    capri_hbm_write_mem(q_addr, buf, q_size);

    reg = mp_->region_by_address(q_addr);
    HAL_ASSERT(reg != NULL);

    action = capri_hbm_cache_pipe_to_action(reg->cache_pipe);
    if (action != P4PLUS_CACHE_ACTION_NONE) {
        p4plus_invalidate_cache(q_addr, q_size, action);
    }

    return 0;
}

int32_t NicLIFManager::GetPCOffset(const char *handle,
                                   const char *prog_name,
                                   const char *label,
                                   uint8_t *ret_offset) {
    assert(handle);
    return get_pc_offset(pinfo_, prog_name, label, ret_offset);
}

