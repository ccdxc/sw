#ifndef _LIF_MANAGER_BASE_HPP_
#define _LIF_MANAGER_BASE_HPP_

#include "bm_allocator.hpp"

#include <set>
#include <map>
#include <mutex>
#include <strings.h>

namespace hal {

const static uint32_t kNumQTypes = 8;
const static uint32_t kMaxQStateSize = 4096;

// Parameters for the InitLIFToQstate call.
struct LIFQStateParams {
  struct {
    uint8_t entries:5,
            size:3;
  } type[kNumQTypes];
  bool dont_zero_memory;
};

// Per LIF queue state managed by the LIF Manager.
struct LIFQState {
  uint32_t lif_id;
  uint32_t allocation_size;   // Amount of HBM allocated.
  uint64_t hbm_address;       // Use uint64_t to support tests.
  LIFQStateParams params_in;  // A copy of user input.
  struct {                    // Per type data.
    uint32_t hbm_offset;
    uint32_t qsize;
    uint32_t rsvd;
    uint32_t num_queues;
  } type[kNumQTypes];
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


  // Initialize the LIF to Qstate Map in hw and allocate any
  // metadata.
  // Returns
  //   0 -      In case of success.
  //   -errno - In case of failure.
  int32_t InitLIFQState(uint32_t lif_id, LIFQStateParams *params);

  //
  int32_t GetLIFQStateBaseAddr(uint32_t, uint32_t);

  // Get the qstate address for a LIF, type and qid.
  // Returns:
  //   Positive address in case of success.
  //   -errno in case of failure.
  int64_t GetLIFQStateAddr(uint32_t lif_id, uint32_t type, uint32_t qid);

  // Return QState for a LIF/Type/QID. The user must pass in the buffer
  // to read the state in. Its ok to pass a buffer of smaller size (> 0).
  // If the passed in buffer is larger than the queue size, only queue size
  // worth of data will be read.
  // Returns:
  //   0     - success.
  //   -errno- failure.
  int32_t ReadQState(uint32_t lif_id, uint32_t type, uint32_t qid,
                     uint8_t *buf, uint32_t bufsize);
 
  // Set QState for a LIF/Type/QID. The user must pass in the buffer
  // to write the state. Its ok to pass a buffer of smaller size (> 0).
  // If the passed in buffer is larger than the queue size, the call will
  // fail. 
  // Returns:
  //   0     - success.
  //   -errno- failure.
  int32_t WriteQState(uint32_t lif_id, uint32_t type, uint32_t qid,
                      const uint8_t *buf, uint32_t bufsize);

  // GetPCOffset for a specific P4+ program.
  virtual int32_t GetPCOffset(
      const char *handle, const char *prog_name,
      const char *label, uint8_t *offset) = 0;

  LIFQState *GetLIFQState(uint32_t lif_id);

  // Implement later
  // void DestroyLIFToQstate(uint32_t lif_id);

 protected:
  // Interactions with Model/HW.
  // Initialize the LIF Qstate in hw and allocate any memory.
  // Returns
  //   0 -      In case of success.
  //   -errno - In case of failure.
  virtual int32_t InitLIFQStateImpl(LIFQState *qstate) = 0;

  virtual int32_t ReadQStateImpl(
      uint64_t q_addr, uint8_t *buf, uint32_t q_size) = 0;
  virtual int32_t WriteQStateImpl(
      uint64_t q_addr, const uint8_t *buf, uint32_t q_size) = 0;

 private:
  const uint32_t kNumMaxLIFs = 2048;


  std::mutex lk_;
  BMAllocator lif_allocator_;
  std::map<uint32_t, LIFQState> alloced_lifs_;

  // Test only
  friend class MockLIFManager;
};

}  // namespace hal

#endif  // _LIF_MANAGER_BASE_HPP_
