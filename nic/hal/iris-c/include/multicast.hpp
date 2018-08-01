
#ifndef __MULTICAST_HPP__
#define __MULTICAST_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "types.hpp"
#include "hal.hpp"
#include "lif.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"
#include "endpoint.hpp"


// (vrf, vlan, mac)
typedef std::tuple<uint64_t, vlan_t, mac_t> mcast_key_t;


class Multicast : public HalObject
{
public:
    Multicast(std::shared_ptr<Vrf> vrf,
              mac_t mac, vlan_t vlan);

    ~Multicast();

    static std::shared_ptr<Multicast> GetInstance(
        std::shared_ptr<Vrf> vrf,
        mac_t mac, vlan_t vlan);

    int AddEnic(std::shared_ptr<Enic> enic);

    int DelEnic(std::shared_ptr<Enic> enic);

    int Update();

    static void Probe();

private:
    mac_t _mac;
    vlan_t _vlan;

    std::shared_ptr<Vrf> vrf_ref;
    std::shared_ptr<L2Segment> l2seg_ref;
    std::map<uint64_t, std::shared_ptr<Enic>> enic_refs;

    static constexpr uint32_t max_mcast_entries = 4096;

    static std::map<mcast_key_t, std::weak_ptr<Multicast>> registry;
};

#endif  /* __MULTICAST_HPP__ */