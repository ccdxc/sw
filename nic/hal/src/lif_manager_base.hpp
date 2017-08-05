#ifndef _LIF_MANAGER_BASE_HPP_
#define _LIF_MANAGER_BASE_HPP_

#include "bm_allocator.hpp"

#include <set>
#include <map>
#include <mutex>
#include <strings.h>

namespace hal {

class LIFToQstateParams {
 public:
  uint32_t lif_id;
  struct PerTypeEntry {
    uint8_t entries:8,
            size:3;
  } per_type_ent[8];

  // In future we can also add a user specific handle here.

  LIFToQstateParams() {
    bzero(this, sizeof(*this));
  }

 private:
  uint64_t impl_handle_;  // Implementation specific handle.
  friend class LIFManagerBase;
};

class LIFManagerBase {
 public:
  LIFManagerBase();

  // Allocates a range of LIFs.
  // If 'start' == -1, pick an appropriate starting point, else
  // allocate from 'start'
  // Returns:
  //    - A positive LIF number if the allocation suceeded.
  //    - -errno in case of failure.
  int32_t LIFRangeAlloc(int32_t start, uint32_t count);

  // Deletes a range of LIFs. Also deletes any associated
  // qstates with each LIF.
  // void LIFRangeDelete(uint32_t start, uint32_t count);

  // Callers are not suppose to allocate LIFToQstateParams.
  // They should call this function to retrieve the pointer,
  // initialize the params and then call InitLIFToQstate().
  // Returns a pointer to params upon success and nullptr in case
  // of failure (invalid LIF ID, LIF not alloced).
  LIFToQstateParams *GetLIFToQstateParams(uint32_t lif_id);

  // Initialize the LIF to Qstate Map in hw and allocate any
  // metadata.
  // Returns
  //   0 -      In case of success.
  //   -errno - In case of failure.
  int32_t InitLIFToQstate(LIFToQstateParams *params);

  // Implement later
  // void DestroyLIFToQstate(uint32_t lif_id);

 protected:
  // Implementation specific
  // Initialize the LIF to Qstate Map in hw and allocate any
  // metadata. The implementation can return a uint64_t to
  // be stored along with the map. This will be passed to future
  // calls to the implementation.
  // Returns
  //   0 -      In case of success.
  //   -errno - In case of failure.
  virtual int32_t InitLIFToQstateImpl(LIFToQstateParams *params,
                                      uint64_t *ret_handle) = 0;

 private:
  const uint32_t kNumMaxLIFs = 2048;

  std::mutex lk_;
  BMAllocator lif_allocator_;
  std::map<uint32_t, LIFToQstateParams> alloced_lifs_;
};

}  // namespace hal

#endif  // _LIF_MANAGER_BASE_HPP_
