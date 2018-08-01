
#ifndef __ENIC_HPP__
#define __ENIC_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "types.hpp"
#include "hal.hpp"
#include "lif.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"


// lif
typedef uint64_t enic_classic_key_t;

// (lif, mac, vlan)
typedef std::tuple<uint64_t, mac_t, vlan_t> enic_hostpin_key_t;


class Enic : public HalObject
{
public:
    // Classic ENIC constructor
    Enic(std::shared_ptr<Lif> lif,
         std::shared_ptr<Vrf> vrf);

    // Classic ENIC instance getter
    static std::shared_ptr<Enic> GetInstance(
        std::shared_ptr<Lif> lif,
        std::shared_ptr<Vrf> vrf);

    // Classic ENIC APIs only
    int Update();
    int AddVlan(vlan_t vlan);
    int DelVlan(vlan_t vlan);

    // Hostpin ENIC constructor
    Enic(std::shared_ptr<Lif> lif,
         std::shared_ptr<Vrf> vrf,
         mac_t mac, vlan_t vlan);

    // Hostpin ENIC instance getter
    static std::shared_ptr<Enic> GetInstance(
        std::shared_ptr<Lif> lif,
        std::shared_ptr<Vrf> vrf,
        mac_t mac, vlan_t vlan);

    ~Enic();

    static void Probe();

    uint64_t GetId();
    uint64_t GetHandle();

    // void SetNativeL2Segment(std::shared_ptr<L2Segment> l2seg);

private:
    uint32_t id;
    uint64_t handle;

    mac_t _mac;
    vlan_t _vlan;

    std::shared_ptr<Lif> lif_ref;
    std::shared_ptr<Vrf> vrf_ref;
    std::map<vlan_t, std::shared_ptr<L2Segment>> l2seg_refs;

    static sdk::lib::indexer *allocator;
    static constexpr uint32_t max_enics = 4096;

    static std::map<enic_classic_key_t, std::weak_ptr<Enic>> classic_registry;
    static std::map<enic_hostpin_key_t, std::weak_ptr<Enic>> hostpin_registry;
};

#endif /* __ENIC_HPP__ */
