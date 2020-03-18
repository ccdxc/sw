//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __LIF_MANAGER_BASE_HPP__
#define __LIF_MANAGER_BASE_HPP__

namespace sdk {
namespace platform {
namespace utils {

const static uint32_t kNumQTypes = 8;
const static uint32_t kMaxQStateSize = 4096;

// Parameters for the InitLIFToQstate call.
struct LIFQStateParams {
  struct {
    uint8_t entries:5,
            size:3;
    uint8_t cosA:4,
            cosB:4;
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
    uint8_t  coses;            // stores cosA in bits 0-3, cosB in bits 4-7
  } type[kNumQTypes];
};

}   // namespace utils
}   // namespace platform
}   // namespace sdk

using namespace sdk::platform::utils;

#endif  // _LIF_MANAGER_BASE_HPP_
