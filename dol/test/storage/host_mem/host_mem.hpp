#ifndef __HOSTMEM_HPP__
#define __HOSTMEM_HPP__

#include "dol/test/storage/host_mem/bm_allocator.hpp"
#include "dol/test/storage/host_mem/params.hpp"

namespace utils {

class HostMem {
 public:
  static HostMem *New();
  ~HostMem();
  void *Alloc(size_t size);
  void Free(void *ptr);
  uint64_t VirtToPhys(void *ptr);
  void *PhysToVirt(uint64_t);

 private:
  HostMem() {}

  int shmid_ = -1;
  void *shmaddr_ = nullptr;
  void *base_addr_;
  std::unique_ptr<BMAllocator> allocator_;
 
  // allocations_ track the sizes of different allocations which
  // are then used to free the allocations.
  std::map<int, uint32_t> allocations_;
};

}  // namespace utils

#endif
