
#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include <vector>

#include "hal.hpp"
#include "lif.hpp"
#include "enic.hpp"
#include "endpoint.hpp"
#include "multicast.hpp"
#include "vrf.hpp"


class MacVlanFilter : public HalObject
{
public:
    MacVlanFilter(
        std::shared_ptr<Lif> lif,
        std::shared_ptr<Vrf> vrf,
        mac_t mac, vlan_t vlan);
    ~MacVlanFilter();

private:
    mac_t _mac;
    vlan_t _vlan;

    std::shared_ptr<Lif> lif_ref;
    std::shared_ptr<Vrf> vrf_ref;
    std::shared_ptr<Enic> enic_ref;
    std::shared_ptr<Endpoint> ep_ref;
    std::shared_ptr<Multicast> mcast_ref;
};

#endif  /* __FILTER_HPP__ */
