// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __UPLINK_HPP__
#define __UPLINK_HPP__

#include "hal.hpp"
#include "vrf.hpp"
#include "hal_types.hpp"

typedef std::list<std::shared_ptr<Uplink>> UplinkList;

class Uplink : public HalObject
{
public:
  static Uplink *Factory(uplink_id_t id, bool is_oob = false);
  static void Destroy(Uplink *uplink);
  int UpdateHalWithNativeL2seg(uint32_t native_l2seg_id);
  void CreateVrf();

  // Get APIs
  uint32_t GetId();
  uint32_t GetPortNum();
  uint32_t GetNumLifs();
  HalVrf *GetVrf();
  HalL2Segment *GetNativeL2Seg();

  // Set APIs
  void SetPortNum(uint32_t port_num);
  void SetNativeL2Seg(HalL2Segment *l2seg);

  void IncNumLifs();
  void DecNumLifs();

  bool IsOOB();

private:
  // APIs
  Uplink(uplink_id_t id, bool is_oob = false);
  ~Uplink() {}

  uplink_id_t id_;              // Uplink PI Id
  uint32_t port_num_;           // Uplinks port number
  uint32_t num_lifs_;           // # of lifs pinned to this Uplink
  bool is_oob_;                 // Out of Band mgmt. uplink
  InterfaceSpec if_spec_;       // Interface spec for HAL
  HalVrf *vrf_;                 // Vrf on the uplink. Appl. in classic.
  HalL2Segment *native_l2seg_;  // Native L2seg

  // id -> Uplink
  static std::map<uint64_t, Uplink*> uplink_db;
};

#endif /* __UPLINK_HPP__ */
