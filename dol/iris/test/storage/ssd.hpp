// Implementation of ssd using ssd_core.
#ifndef DOL_TEST_STORAGE_SSD_HPP
#define DOL_TEST_STORAGE_SSD_HPP

#include <byteswap.h>
#include "dol/iris/test/storage/ssd_core.hpp"
#include "pal_compat.hpp"

namespace storage_test {

class NvmeSsd : public NvmeSsdCore {
 public:
  NvmeSsd() { Ctor(); }
  ~NvmeSsd() { Dtor(); }
  virtual void *DMAMemAlloc(uint32_t size) { return ALLOC_HOST_MEM(size); }
  virtual void DMAMemFree(void *ptr) { FREE_HOST_MEM(ptr); }
  virtual uint64_t DMAMemV2P(void *ptr) { return HOST_MEM_V2P(ptr); }
  virtual void *DMAMemP2V(uint64_t addr) { return HOST_MEM_P2V(addr); }
  virtual void DMAMemCopyP2V(uint64_t src, void *dst, uint32_t size) {
    if (src & (1ull << 63)) {
      memcpy(dst, DMAMemP2V(src), size);
    } else {
      READ_MEM(src, (uint8_t *)dst, size, 0);
    }
  }
  virtual void DMAMemCopyV2P(void *src, uint64_t dst, uint32_t size) {
    if (dst & (1ull << 63)) {
      memcpy(DMAMemP2V(dst), src, size);
    } else {
      WRITE_MEM(dst, (uint8_t *)src, size, 0);
    }
  }
  virtual void RaiseInterrupt(uint16_t index) { 
    if (intr_enabled_) {
      uint64_t db_data_ndx =  (db_data_ & 0xFFFFFFFFFFFF0000ULL) | index;
      //printf("Calling DB %lx %lx\n", db_addr_, db_data_ndx);
      WRITE_DB64(db_addr_, db_data_ndx);
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
