
#ifndef __UPLINK_HPP__
#define __UPLINK_HPP__

#include <vector>

#include "nic/sdk/include/sdk/indexer.hpp"

#include "hal.hpp"
#include "types.hpp"

typedef std::list<std::shared_ptr<Uplink>> UplinkList;

class Uplink : public HalObject
{
public:

  static Uplink *Factory(uplink_id_t id);
  static void Destroy(Uplink *uplink);

  // Get APIs
  uint32_t GetId();
  uint64_t GetHandle();
  uint32_t GetPortNum();
  Vrf *GetVrf();

  // Set APIs
  void SetPortNum();



private:
  // APIs
  Uplink(uplink_id_t id);
  ~Uplink();

  uplink_id_t id;
  uint64_t handle;
  uint32_t port_num;
  intf::InterfaceSpec if_spec;
  Vrf *vrf;

  // id -> Uplink
  static std::map<uint64_t, Uplink*> uplink_db;
};

#endif /* __UPLINK_HPP__ */
