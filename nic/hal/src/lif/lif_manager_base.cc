#include "nic/hal/src/lif/lif_manager_base.hpp"
#include <errno.h>
#include <assert.h>

namespace hal {

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
    alloced_lifs_[base + i].lif_id = base + i;
  }

  return base;
}

void LIFManagerBase::DeleteLIF(int32_t hw_lif_id) {
  if ((uint32_t)hw_lif_id >= kNumMaxLIFs)
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

int32_t LIFManagerBase::InitLIFQState(uint32_t lif_id,
                                      LIFQStateParams *params, uint8_t hint_cos) {
  if (lif_id >= kNumMaxLIFs)
    return -EINVAL;
  std::lock_guard<std::mutex> lk(lk_);
  LIFQState *qstate = GetLIFQState(lif_id);
  if (qstate == nullptr)
    return -ENOENT;

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

  for (i = 0; i < kNumQTypes ; i++) {
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
  if (ret < 0)
    return ret;

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
  return (int64_t)(qstate->hbm_address + qstate->type[type].hbm_offset +
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
  if(bufsize > 2)  {
    buf[2] = qstate->type[type].coses;
  }

  uint64_t q_addr = qstate->hbm_address + qstate->type[type].hbm_offset +
    (qid * qstate->type[type].qsize);
  return WriteQStateImpl(q_addr, buf, bufsize);
}
}  // namespace hal
