
#ifndef __VRF_HPP__
#define __VRF_HPP__

#include <vector>

#include "nic/sdk/include/sdk/indexer.hpp"

#include "hal.hpp"
#include "gen/proto/types.pb.h"

#define VRF_ID_BASE 17


class HalVrf : public HalObject
{
public:
  static HalVrf *Factory(types::VrfType type = types::VRF_TYPE_CUSTOMER);
  static void Destroy(HalVrf *vrf);


  uint64_t GetId();
  uint64_t GetHandle();

  static void Probe();

private:
  HalVrf(types::VrfType type = types::VRF_TYPE_CUSTOMER);
  ~HalVrf();

  uint32_t id;
  uint64_t handle;
  types::VrfType type;

  // For vrf id
  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_vrfs = 8;
};

#endif
