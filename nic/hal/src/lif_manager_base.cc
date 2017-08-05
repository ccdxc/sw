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

LIFToQstateParams *LIFManagerBase::GetLIFToQstateParams(uint32_t lif_id) {
  if (lif_id >= kNumMaxLIFs)
    return nullptr;

  auto it = alloced_lifs_.find(lif_id);
  if (it == alloced_lifs_.end())
    return nullptr;

  return &it->second;
}

int32_t LIFManagerBase::InitLIFToQstate(LIFToQstateParams *params) {
  assert(params->lif_id < kNumMaxLIFs);
  assert(GetLIFToQstateParams(params->lif_id) == params);
  uint64_t handle;
  int32_t ret = InitLIFToQstateImpl(params, &handle);
  if (ret < 0)
    return ret;
  params->impl_handle_ = handle;

  return ret;
}

}  // namespace hal
