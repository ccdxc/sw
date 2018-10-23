
#ifndef __L2SEG_HPP__
#define __L2SEG_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal_types.hpp"
#include "hal.hpp"
#include "vrf.hpp"


// (vrf, vlan)
typedef std::tuple<uint32_t, vlan_t> l2seg_key_t;

// uplink_id -> Uplink
typedef std::map<uplink_id_t, Uplink*> uplink_map_t;


class HalL2Segment : public HalObject
{
public:
    static HalL2Segment *Factory(HalVrf *vrf, uint16_t vlan);
    static void Destroy(HalL2Segment *l2seg);

    static HalL2Segment *Lookup(HalVrf *vrf, uint16_t vlan);


    void AddUplink(Uplink *uplink);
    void DelUplink(Uplink *uplink);

    uint64_t GetId();
    uint64_t GetHandle();
    HalVrf *GetVrf();

    static void Probe();

private:
  HalL2Segment(HalVrf *vrf, uint16_t vlan_id);
  ~HalL2Segment();
  void TriggerHalUpdate();

  uint32_t id;
  uint64_t handle;

  vlan_t _vlan;

  HalVrf *vrf;

  uplink_map_t uplink_refs;

  // L2seg id
  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_l2segs = 4096;

  static std::map<l2seg_key_t, HalL2Segment*> l2seg_db;

};

#endif /* __L2SEG_HPP__ */
