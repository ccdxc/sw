
#ifndef __L2SEG_HPP__
#define __L2SEG_HPP__

#include <vector>

#include "nic/sdk/include/sdk/indexer.hpp"

#include "types.hpp"
#include "hal.hpp"
#include "vrf.hpp"


// (vrf, vlan)
typedef std::tuple<uint32_t, vlan_t> l2seg_key_t;

// uplink_id -> Uplink
typedef std::map<uplink_id_t, Uplink*> uplink_map_t;


class L2Segment : public HalObject
{
public:
    static L2Segment *Factory(Vrf *vrf, uint16_t vlan);
    static void Destroy(L2Segment *l2seg);

    static L2Segment *Lookup(Vrf *vrf, uint16_t vlan);


    void AddUplink(Uplink *uplink);
    void DelUplink(Uplink *uplink);

    uint64_t GetId();
    uint64_t GetHandle();
    Vrf *GetVrf();

    static void Probe();

private:
  L2Segment(Vrf *vrf, uint16_t vlan_id);
  ~L2Segment();
  void TriggerHalUpdate();

  uint32_t id;
  uint64_t handle;

  vlan_t _vlan;

  Vrf *vrf;

  uplink_map_t uplink_refs;

  // L2seg id
  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_l2segs = 4096;

  static std::map<l2seg_key_t, L2Segment*> l2seg_db;

};

#endif /* __L2SEG_HPP__ */
