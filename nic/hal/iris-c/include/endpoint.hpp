
#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include "sdk/indexer.hpp"

#include "hal.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"
#include "enic.hpp"


class Endpoint : public HalObject
{
public:
  Endpoint(
      std::shared_ptr<Enic> enic,
      std::shared_ptr<Vrf> vrf,
      mac_t mac, vlan_t vlan);
  ~Endpoint();

  void Probe();

private:
  mac_t _mac;
  uint64_t handle;

  std::shared_ptr<Vrf> vrf_ref;
  std::shared_ptr<L2Segment> l2seg_ref;
};

#endif /* __ENDPOINT_HPP__ */
