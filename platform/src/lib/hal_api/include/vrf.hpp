
#ifndef __VRF_HPP__
#define __VRF_HPP__

#include <vector>

#include "lib/indexer/indexer.hpp"

#include "hal.hpp"
#include "gen/proto/types.pb.h"

class Uplink;

class HalVrf : public HalObject
{
public:
  static HalVrf *Factory(types::VrfType type = types::VRF_TYPE_CUSTOMER, Uplink *uplink = NULL);
  static void Destroy(HalVrf *vrf);


  uint64_t GetId();
  uint64_t GetHandle();

  static void Probe();
  Uplink *GetUplink() { return uplink; }

private:
  HalVrf(types::VrfType type = types::VRF_TYPE_CUSTOMER, Uplink *uplink = NULL);
  ~HalVrf();

  uint32_t id;
  uint64_t handle;
  types::VrfType type;
  Uplink *uplink;

  // For vrf id
  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_vrfs = 8;
};

#endif
