
#ifndef __L2SEG_HPP__
#define __L2SEG_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "types.hpp"
#include "hal.hpp"
#include "vrf.hpp"


// (vrf, vlan)
typedef std::tuple<uint32_t, vlan_t> l2seg_key_t;


class L2Segment : public HalObject
{
public:
  L2Segment(std::shared_ptr<Vrf> vrf, uint16_t vlan_id);
  ~L2Segment();

  static std::shared_ptr<L2Segment> GetInstance(
    std::shared_ptr<Vrf> vrf, uint16_t vlan_id);

  uint64_t GetId();
  uint64_t GetHandle();

  static void Probe();

private:
  uint32_t id;
  uint64_t handle;

  vlan_t _vlan;

  std::shared_ptr<Vrf> vrf_ref;

  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_l2segs = 4096;

  static std::map<l2seg_key_t, std::shared_ptr<L2Segment>> registry;  
};

#endif /* __L2SEG_HPP__ */
