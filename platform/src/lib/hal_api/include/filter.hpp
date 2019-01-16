
#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include <vector>

#include "hal.hpp"
#include "enic.hpp"
#include "endpoint.hpp"
#include "multicast.hpp"

#include "gen/proto/kh.grpc.pb.h"

#if 0
// Have to be in sync with kh::FilterType
#define FILTER_TYPE(ENTRY)                                          \
    ENTRY(FILTER_NONE,      0, "FILTER_NONE")                       \
    ENTRY(FILTER_LIF,       1, "FILTER_LIF")                        \
    ENTRY(FILTER_LIF_MAC,   2, "FILTER_LIF_MAC")                    \
    ENTRY(FILTER_LIF_VLAN,  3, "FILTER_LIF_VLAN")                   \
    ENTRY(FILTER_LIF_MAC_VLAN,    4, "FILTER_LIF_MAC_VLAN")
DEFINE_ENUM(filter_type_t, FILTER_TYPE)
#undef FILTER_TYPE
#endif

typedef kh::FilterType filter_type_t;

class MacVlanFilter : public HalObject
{
public:
  static MacVlanFilter *Factory(EthLif *eth_lif,
                                mac_t mac, vlan_t vlan,
                                filter_type_t type = kh::FILTER_LIF_MAC_VLAN);
  static void Destroy(MacVlanFilter *filter);

private:
    MacVlanFilter(
        EthLif *eth_lif,
        mac_t mac, vlan_t vlan,
        filter_type_t type);
    ~MacVlanFilter();

    filter_type_t _type;
    mac_t _mac;
    vlan_t _vlan;

    EthLif *eth_lif;
    HalEndpoint *ep;
    HalMulticast *mcast;

    uint64_t handle;
    endpoint::FilterSpec spec;


};

#endif  /* __FILTER_HPP__ */
