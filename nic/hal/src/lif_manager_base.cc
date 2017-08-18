#include "lif_manager_base.hpp"
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
                                      LIFQStateParams *params) {
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
  for (i = 0; i < (kNumQTypes - 1); i++) {
    cur_size = params->type[i].size;
    cur_size = 1 << (cur_size + 5);
    qstate->type[i].hbm_offset = running_offset;
    qstate->type[i].qsize = cur_size;

    uint32_t next_size = params->type[i+1].size;
    next_size = 1 << (next_size + 5);
    uint32_t align = next_size - 1;
    num_entries = params->type[i].entries;
    num_entries = 1 << num_entries;
    running_offset += (num_entries * cur_size);
    while ((running_offset & align) != 0) {
      num_entries++;
      running_offset += cur_size;
      assert(num_entries <= (16 * 1024 * 1024));
    }
    // Make sure num entries is a power of 2.
    assert((num_entries & (num_entries - 1)) == 0);
    // Update what was passed in.
    params->type[i].entries = __builtin_ffs(num_entries) - 1;
    qstate->type[i].num_queues = num_entries;
  }
  // Now put in the last entry. There is no align requirement.
  cur_size = params->type[i].size;
  cur_size = 1 << (cur_size + 5);
  qstate->type[i].hbm_offset = running_offset;
  qstate->type[i].qsize = cur_size;
  num_entries = params->type[i].entries;
  num_entries = 1 << num_entries;
  qstate->type[i].num_queues = num_entries;
  running_offset += (num_entries * cur_size);
  qstate->allocation_size = running_offset;
  // Cache the params.
  qstate->params_in = *params;
  
  int32_t ret = InitLIFQStateImpl(qstate);
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
    uint32_t lif_id, uint32_t type, uint32_t qid, const uint8_t *buf,
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
  uint64_t q_addr = qstate->hbm_address + qstate->type[type].hbm_offset +
    (qid * qstate->type[type].qsize);
  return WriteQStateImpl(q_addr, buf, bufsize);
}

}  // namespace hal
