
#ifndef __VRF_HPP__
#define __VRF_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal.hpp"


class Vrf : public HalObject
{
public:
  Vrf();
  ~Vrf();

  uint64_t GetId();
  uint64_t GetHandle();

  static void Probe();

private:
  uint32_t id;
  uint64_t handle;

  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_vrfs = 8;
};

#endif
