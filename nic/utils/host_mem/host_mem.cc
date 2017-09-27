#include <assert.h>
#include "nic/utils/host_mem/host_mem.hpp"

namespace utils {

static uint32_t NumUnits(size_t size) {
  return (size + kAllocUnit - 1) >> kAllocUnitShift;
}

HostMem *HostMem::New() {
  std::unique_ptr<HostMem> mem(new HostMem());
  mem->shmid_ = shmget(HostMemHandle(), kShmSize, 0666);
  if (mem->shmid_ < 0)
    return nullptr;
  mem->shmaddr_ = shmat(mem->shmid_, nullptr, 0);
  if (mem->shmaddr_ == (void*)-1)
    return nullptr;

  uint32_t num_units = kShmSize/kAllocUnit;
  uint64_t mask = kAllocUnit - 1;
  if ((uint64_t)mem->shmaddr_ & mask) {
    mem->base_addr_ = (void *)
        (((uint64_t)mem->shmaddr_ + mask) & ~mask);
    num_units--;
  } else {
    mem->base_addr_ = mem->shmaddr_;
  }
  mem->allocator_.reset(new hal::BMAllocator(num_units));

  return mem.release();
}

void *HostMem::Alloc(size_t size, uint32_t align) {
  uint32_t num_units = NumUnits(size);
  uint32_t units_per_align = (align / kAllocUnit) +
                             ((align % kAllocUnit) ? 1 : 0);
  int offset = allocator_->Alloc(num_units, units_per_align);
  if (offset < 0)
    return nullptr;
  allocations_[offset] = num_units;
  return (void *)(((uint8_t *)base_addr_) +
      (((uint64_t)offset) << kAllocUnitShift));
}

void HostMem::Free(void *addr) {
  uint64_t offset = VirtToPhys(addr);
  assert ((offset & (kAllocUnit - 1)) == 0);
  offset >>= kAllocUnitShift;
  auto it = allocations_.find((int)offset);
  assert(it != allocations_.end());
  allocations_.erase(it);
  allocator_->Free((uint32_t)offset, it->second);
}

uint64_t HostMem::VirtToPhys(void *addr) {
  assert(addr >= shmaddr_);
  return (1ull << 63) | ((uint64_t)addr - (uint64_t)shmaddr_);
}

void *HostMem::PhysToVirt(uint64_t phys) {
  return (void *)(~(1ull << 63) & ((uint64_t)phys + (uint64_t)shmaddr_));
}

HostMem::~HostMem() {
  if ((shmaddr_) && (shmaddr_ != (void *)-1))
    shmdt(shmaddr_);
  shmaddr_ = nullptr;
}

}  // namespace utils
