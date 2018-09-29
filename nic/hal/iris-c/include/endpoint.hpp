
#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include "nic/sdk/include/sdk/indexer.hpp"

#include "hal.hpp"
#include "enic.hpp"

typedef std::tuple<L2Segment *, mac_t> ep_key_t;

class Endpoint : public HalObject
{
public:
  static Endpoint *Factory(L2Segment *l2seg, mac_t mac, Enic *enic);
  static void Destroy(Endpoint *ep);
  static Endpoint *Lookup(L2Segment *l2seg, mac_t mac);

  L2Segment *GetL2Seg();
  mac_t GetMac();

private:
  Endpoint(L2Segment *l2seg, mac_t mac, Enic *enic);
  ~Endpoint();

  mac_t mac;
  L2Segment *l2seg;
  Enic *enic;
  uint64_t handle;

  static std::map<ep_key_t, Endpoint*> ep_db;
};

#endif /* __ENDPOINT_HPP__ */
