
#ifndef __NIC_HPP__
#define __NIC_HPP__

#include "types.hpp"

#include "lif.hpp"
#include "hal.hpp"
#include "enic.hpp"
#include "uplink.hpp"
#include "vrf.hpp"
#include "filter.hpp"


class EthLif : public HalObject
{
public:
    EthLif(std::shared_ptr<Uplink> uplink, std::shared_ptr<Vrf> vrf,
        struct eth_spec &eth_spec, bool is_mgmt_lif);
    ~EthLif();

    int AddMac(mac_t mac);
    int DelMac(mac_t mac);

    int AddVlan(vlan_t vlan);
    int DelVlan(vlan_t vlan);

    int AddMacVlan(mac_t mac, vlan_t vlan);
    int DelMacVlan(mac_t mac, vlan_t vlan);

private:
    bool is_mgmt_lif;
    static struct queue_spec qinfo[NUM_QUEUE_TYPES];

    int CreateMacVlanFilter(mac_t mac, vlan_t vlan);
    int DeleteMacVlanFilter(mac_t mac, vlan_t vlan);

    std::shared_ptr<Lif> lif_ref;

    static sdk::lib::indexer *allocator;
    static constexpr uint64_t max_macaddrs_per_lif = 64;
    static constexpr uint64_t max_vlans_per_lif = 8;
    static constexpr uint64_t max_filters_per_lif = 4096;

    std::map<mac_t, uint32_t> mac_table;
    std::map<vlan_t, uint32_t> vlan_table;
    std::map<mac_vlan_t, uint32_t> mac_vlan_table;

    // TODO: Move these into a filter table network object, that will
    // decouple the lif host object from keeping vrf/uplink object
    // references.
    // In smart nic mode we do not have knowledge of these objects, so
    // we can redefine the filter table object to use HAL filter APIs.
    std::shared_ptr<Vrf> vrf_ref;
    std::shared_ptr<Uplink> uplink_ref;
    std::map<mac_vlan_t, std::shared_ptr<MacVlanFilter>> mac_vlan_filter_table;
};

#endif
