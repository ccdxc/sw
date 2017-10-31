// Implementation of CAPRI LIF Manager.
#ifndef _LIF_MANAGER_HPP_
#define _LIF_MANAGER_HPP_

#include "nic/hal/src/lif_manager_base.hpp"
#include <memory>
#include <map>

namespace hal {

class LIFManager : public LIFManagerBase {
 public:
  LIFManager();
  virtual int32_t GetPCOffset(const char *handle, const char *prog_name,
                              const char *label, uint8_t *offset);

 protected:
  virtual int32_t InitLIFQStateImpl(LIFQState *qstate);
  virtual int32_t ReadQStateImpl(
      uint64_t q_addr, uint8_t *buf, uint32_t q_size);
  virtual int32_t WriteQStateImpl(
      uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
  virtual void DeleteLIFQStateImpl(LIFQState *qstate);

 private:
  uint64_t hbm_base_;
  std::unique_ptr<BMAllocator> hbm_allocator_;

  // Track allocation size which are needed when we
  // free memory.
  std::map<uint32_t, uint32_t> allocation_sizes_;
};

}  // namespace hal

#endif
