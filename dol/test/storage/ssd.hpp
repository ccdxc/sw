// Implementation of ssd using ssd_core.
#ifndef DOL_TEST_STORAGE_SSD_HPP
#define DOL_TEST_STORAGE_SSD_HPP

#include <byteswap.h>
#include "dol/test/storage/ssd_core.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

namespace storage_test {

class NvmeSsd : public NvmeSsdCore {
 public:
  NvmeSsd() { Ctor(); }
  ~NvmeSsd() { Dtor(); }
  virtual void *DMAMemAlloc(uint32_t size) { return alloc_host_mem(size); }
  virtual void DMAMemFree(void *ptr) { free_host_mem(ptr); }
  virtual uint64_t DMAMemV2P(void *ptr) { return host_mem_v2p(ptr); }
  virtual void *DMAMemP2V(uint64_t addr) { return host_mem_p2v(addr); }
  virtual void RaiseInterrupt(uint16_t index) { 
    if (intr_enabled_) {
      uint64_t db_data_ndx =  (db_data_ & 0xFFFFFFFFFFFF0000ULL) | bswap_16(index);
      step_doorbell(db_addr_, db_data_ndx);
    }
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
