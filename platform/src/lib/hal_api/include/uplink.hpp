
#ifndef __UPLINK_HPP__
#define __UPLINK_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal.hpp"
#include "hal_types.hpp"
#include "vrf.hpp"

typedef std::list<std::shared_ptr<Uplink>> UplinkList;

class Uplink : public HalObject
{
public:

  static Uplink *Factory(uplink_id_t id);
  static void Destroy(Uplink *uplink);
  int UpdateHalWithNativeL2seg(uint32_t native_l2seg_id);

  // Get APIs
  uint32_t GetId();
  uint64_t GetHandle();
  uint32_t GetPortNum();
  uint32_t GetNumLifs();
  void IncNumLifs();
  void DecNumLifs();
  HalVrf *GetVrf();
  HalL2Segment *GetNativeL2Seg();
  void SetNativeL2Seg(HalL2Segment *l2seg);

  // Set APIs
  void SetPortNum();



private:
  // APIs
  Uplink(uplink_id_t id);
  ~Uplink();

  uplink_id_t id;
  uint64_t handle;
  uint32_t port_num;
  uint32_t num_lifs;
  intf::InterfaceSpec if_spec;
  HalVrf *vrf;
  HalL2Segment *native_l2seg;

  // id -> Uplink
  static std::map<uint64_t, Uplink*> uplink_db;
};

#endif /* __UPLINK_HPP__ */
