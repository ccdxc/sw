#ifndef __HOSTMEM_HPP__
#define __HOSTMEM_HPP__

// TODO Need to move bm_allocator to utils.
#include "nic/utils/bm_allocator/bm_allocator.hpp"

#include "nic/utils/host_mem/params.hpp"

namespace utils {

class HostMem {
 public:
  static HostMem *New(bool bhalf = false);
  ~HostMem();
  void *Alloc(size_t size, uint32_t align=1);
  void Free(void *ptr);
  uint64_t VirtToPhys(void *ptr);
  void *PhysToVirt(uint64_t);

 private:
  HostMem() {}

  int shmid_ = -1;
  void *shmaddr_ = nullptr;
  void *base_addr_;
  std::unique_ptr<hal::BMAllocator> allocator_;
 
  // allocations_ track the sizes of different allocations which
  // are then used to free the allocations.
  std::map<int, uint32_t> allocations_;
  int offset_ = 0;
};

}  // namespace utils

#endif
