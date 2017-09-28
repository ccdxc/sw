// Implementation of ssd using ssd_core.
#ifndef DOL_TEST_STORAGE_SSD_HPP
#define DOL_TEST_STORAGE_SSD_HPP

#include "dol/test/storage/ssd_core.hpp"
#include "nic/utils/host_mem/host_mem.hpp"
#include "nic/model_sim/include/lib_model_client.h"

extern std::unique_ptr<utils::HostMem> g_hostmem;

namespace storage_test {

class NvmeSsd : public NvmeSsdCore {
 public:
  NvmeSsd() { Ctor(); }
  ~NvmeSsd() { Dtor(); }
  virtual void *DMAMemAlloc(uint32_t size) { return g_hostmem->Alloc(size); }
  virtual void DMAMemFree(void *ptr) { g_hostmem->Free(ptr); }
  virtual uint64_t DMAMemV2P(void *ptr) { return g_hostmem->VirtToPhys(ptr); }
  virtual void *DMAMemP2V(uint64_t addr) { return g_hostmem->PhysToVirt(addr); }
  virtual void RaiseInterrupt() { 
    step_doorbell(db_addr_, db_data_);
  }

  void EnableInterrupt(uint64_t addr, uint64_t data) {
    intr_enabled_ = true;
    db_addr_ = addr; db_data_ = data;
  }
  void DisableInterrupt() { intr_enabled_ = false; }

 private:
  bool intr_enabled_ = false;
  uint64_t db_addr_, db_data_;
};

}  // namespace storage_test

#endif  // DOL_TEST_STORAGE_SSD_HPP
