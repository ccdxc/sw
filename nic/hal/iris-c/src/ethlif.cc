
#include <iostream>
#include <cmath>

#include "ethlif.hpp"
#include "print.hpp"

using namespace std;


const char qstate_64[64] = { 0 };
const char qstate_1024[1024] = { 0 };


sdk::lib::indexer *EthLif::filter_allocator = sdk::lib::indexer::factory(EthLif::max_filters_per_lif, false, true);

EthLifMap EthLif::ethlif_db;


EthLif *
EthLif::Factory(uint32_t hw_lif_id, Uplink *pinned_uplink,
                bool is_mgmt_lif)
{
    api_trace("EthLif Create");

    if (ethlif_db.find(hw_lif_id) != ethlif_db.end()) {
        HAL_TRACE_WARN("Duplicate Create of EthLif with id: {}",
                       hw_lif_id);
        return NULL;
    }

    EthLif *eth_lif = new EthLif(hw_lif_id,
                                 pinned_uplink,
                                 is_mgmt_lif);

    // Store in DB for disruptive upgrade
    ethlif_db[hw_lif_id] = eth_lif;

    // Create Enic for every Lif in Classic Mode
    if (hal->GetMode() == FWD_MODE_CLASSIC) {
        eth_lif->enic = Enic::Factory(eth_lif);
    }

    return eth_lif;
}

void
EthLif::Destroy(EthLif *eth_lif)
{
    api_trace("EthLif Delete");

    // Remove from DB
    ethlif_db.erase(eth_lif->GetHwLifId());

    if (eth_lif) {
        eth_lif->~EthLif();
    }
}

EthLif::EthLif(uint32_t hw_lif_id, Uplink *pinned_uplink,
               bool is_mgmt_lif)
{
    this->is_mgmt_lif = is_mgmt_lif;
    this->uplink = pinned_uplink;
    this->hw_lif_id = hw_lif_id;

    lif = Lif::Factory(hw_lif_id, pinned_uplink);
}

EthLif::~EthLif()
{
    mac_table.clear();
    vlan_table.clear();
    mac_vlan_table.clear();
    mac_vlan_filter_table.clear();
}

void
EthLif::AddMac(mac_t mac)
{
    mac_vlan_t mac_vlan;

    api_trace("Adding Mac Filter");
    HAL_TRACE_DEBUG("Adding Mac filter: {}", macaddr2str(mac));

    if (mac_table.find(mac) == mac_table.end()) {
        /*
         * Classic:
         *      - Walk through Vlans and create (Mac,Vlan) filters
         * Smart:
         *      - Create Mac filter
         */
        if (hal->GetMode() == FWD_MODE_CLASSIC) {
            // Register new mac across all existing vlans
            for (auto vlan_it = vlan_table.cbegin(); vlan_it != vlan_table.cend(); vlan_it++) {
                // Check if (MacVlan) filter is already present
                mac_vlan = make_tuple(mac, *vlan_it);
                if (mac_vlan_table.find(mac_vlan) == mac_vlan_table.end()) {
                    // No (MacVlan) filter. Creating (Mac, Vlan)
                    CreateMacVlanFilter(mac, *vlan_it);
                } else {
                    HAL_TRACE_DEBUG("(Mac,Vlan) filter present. No-op");
                }
            }
        } else {
            CreateMacFilter(mac);
        }

        // Store mac filter
        mac_table.insert(mac);
    } else {
        HAL_TRACE_WARN("Mac already registered: {}", mac);
    }
}

void
EthLif::DelMac(mac_t mac)
{
    mac_vlan_t mac_vlan_key, mac_key, vlan_key;

    api_trace("Deleting Mac Filter");
    HAL_TRACE_DEBUG("Deleting Mac filter: {}", macaddr2str(mac));

    mac_key = make_tuple(mac, 0);
    if (mac_table.find(mac) != mac_table.end()) {
        if (hal->GetMode() == FWD_MODE_CLASSIC) {
            for (auto vlan_it = vlan_table.cbegin(); vlan_it != vlan_table.cend(); vlan_it++) {
                vlan_key = make_tuple(0, *vlan_it);
                mac_vlan_key = make_tuple(mac, *vlan_it);
                if (vlan_table.find(*vlan_it) != vlan_table.end() &&
                    mac_vlan_table.find(mac_vlan_key) == mac_vlan_table.end()) {
                    HAL_TRACE_DEBUG("Mac Delete: Mac, Vlan are present but (Mac,Vlan) is not. Remove (Mac,Vlan) entity");
                    // Mac, Vlan are present and (Mac,Vlan) is not
                    DeleteMacVlanFilter(mac, *vlan_it);
                } else {
                    // Case:
                    //  Case 1: Vlan filter not present but (Mac,Vlan) is either present or not.
                    //  Case 2: Vlan filter is present along with (Mac,Vlan)
                    HAL_TRACE_DEBUG("Mac Delete: No-op");
                }
            }
        } else {
            DeleteMacFilter(mac);
        }

        // Erase mac filter
        mac_table.erase(mac);
    } else {
        HAL_TRACE_ERR("Mac not registered: {}", mac);
    }
}

void
EthLif::AddVlan(vlan_t vlan)
{
    mac_vlan_t mac_vlan;

    api_trace("Adding Vlan Filter");
    HAL_TRACE_DEBUG("Adding Vlan filter: {}", vlan);

    if (vlan_table.find(vlan) == vlan_table.end()) {
        /*
         * Classic:
         *      - Walk through Vlans and create (Mac,Vlan) filters
         * Smart:
         *      - Create Vlan filter
         */
        if (hal->GetMode() == FWD_MODE_CLASSIC) {
            // Register new mac across all existing vlans
            for (auto it = mac_table.cbegin(); it != mac_table.cend(); it++) {
                // Check if (MacVlan) filter is already present
                mac_vlan = make_tuple(*it, vlan);
                if (mac_vlan_table.find(mac_vlan) == mac_vlan_table.end()) {
                    // No (MacVlan) filter. Creating (Mac, Vlan)
                    CreateMacVlanFilter(*it, vlan);
                } else {
                    HAL_TRACE_DEBUG("(Mac,Vlan) filter present. No-op");
                }
            }
        } else {
            CreateVlanFilter(vlan);
        }

        // Store vlan filter
        vlan_table.insert(vlan);
    } else {
        HAL_TRACE_WARN("Vlan already registered: {}", vlan);
    }
}

void
EthLif::DelVlan(vlan_t vlan)
{
    mac_vlan_t mac_vlan_key;

    api_trace("Deleting Vlan Filter");
    HAL_TRACE_DEBUG("Deleting Vlan filter: {}", vlan);

    if (vlan_table.find(vlan) != vlan_table.end()) {
        if (hal->GetMode() == FWD_MODE_CLASSIC) {
            for (auto it = mac_table.cbegin(); it != mac_table.cend(); it++) {
                mac_vlan_key = make_tuple(*it, vlan);
                if (mac_table.find(*it) != mac_table.end() &&
                    mac_vlan_table.find(mac_vlan_key) == mac_vlan_table.end()) {
                    HAL_TRACE_DEBUG("Vlan Delete: Mac, Vlan are present but (Mac,Vlan) is not. Remove (Mac,Vlan) entity");
                    // Mac, Vlan are present and (Mac,Vlan) is not
                    DeleteMacVlanFilter(*it, vlan);
                } else {
                    // Case:
                    //  Case 1: Mac filter not present but (Mac,Vlan) is either present or not.
                    //  Case 2: Mac filter is present along with (Mac,Vlan)
                    HAL_TRACE_DEBUG("Vlan Delete: No-op");
                }
            }
        } else {
            DeleteVlanFilter(vlan);
        }

        // Erase mac filter
        vlan_table.erase(vlan);
    } else {
        HAL_TRACE_ERR("Vlan not registered: {}", vlan);
    }
}

void
EthLif::AddMacVlan(mac_t mac, vlan_t vlan)
{
    mac_vlan_t key(mac, vlan);

    api_trace("Adding (Mac,Vlan) Filter");
    HAL_TRACE_DEBUG("Adding (Mac,Vlan) mac: {}, filter: {}", macaddr2str(mac), vlan);

    if (mac_vlan_table.find(key) == mac_vlan_table.end()) {
        if (hal->GetMode() == FWD_MODE_CLASSIC) {
            // Check if mac filter and vlan filter is present
            if (mac_table.find(mac) == mac_table.end() ||
                vlan_table.find(vlan) == vlan_table.end()) {
                CreateMacVlanFilter(mac, vlan);
            } else {
                HAL_TRACE_DEBUG("Mac filter and Vlan filter preset. "
                                "No-op for (Mac,Vlan) filter");
            }
        } else {
            CreateMacVlanFilter(mac, vlan);
        }

        // Store mac-vlan filter
        mac_vlan_table.insert(key);
    } else {
        HAL_TRACE_WARN("Mac-Vlan already registered: {}", mac);
    }
}

void
EthLif::DelMacVlan(mac_t mac, vlan_t vlan)
{
    mac_vlan_t mac_vlan_key;

    api_trace("Deleting (Mac,Vlan) Filter");
    HAL_TRACE_DEBUG("Deleting (Mac,Vlan) mac: {}, filter: {}", macaddr2str(mac), vlan);

    mac_vlan_key = make_tuple(mac, vlan);
    if (mac_vlan_table.find(mac_vlan_key) != mac_vlan_table.end()) {
        if (hal->GetMode() == FWD_MODE_CLASSIC) {
            if (mac_table.find(mac) == mac_table.end() ||
                 vlan_table.find(vlan) == vlan_table.end()) {
                // One of Mac or Vlan is not present.
                // (Mac,Vlan) entity was created only by (Mac,Vlan) filter
                DeleteMacVlanFilter(mac, vlan);
            } else {
                // Mac filter and Vlan filter both exist
                HAL_TRACE_DEBUG("Mac filter and Vlan filter present. "
                                "No-op for (Mac,Vlan) filter");
            }
        } else {
            DeleteMacVlanFilter(mac, vlan);
        }
        // Erase mac-vlan filter
        mac_vlan_table.erase(mac_vlan_key);
    } else {
        HAL_TRACE_ERR("(Mac,Vlan) already not registered: mac: {}, vlan: {}",
                      mac, vlan);
    }
}

void
EthLif::CreateMacVlanFilter(mac_t mac, vlan_t vlan)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC_VLAN, mac, vlan);

    mac_vlan_filter_table[key] = MacVlanFilter::Factory(this, mac, vlan);
}

void
EthLif::DeleteMacVlanFilter(mac_t mac, vlan_t vlan)
{
    std::map<mac_vlan_filter_t, MacVlanFilter*>::iterator it;
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC_VLAN, mac, vlan);

    it = mac_vlan_filter_table.find(key);
    MacVlanFilter *filter = it->second;

    mac_vlan_filter_table.erase(it);
    MacVlanFilter::Destroy(filter);
}

// Should be called only in case of smart.
void
EthLif::CreateMacFilter(mac_t mac)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC, mac, 0);

    mac_vlan_filter_table[key] = MacVlanFilter::Factory(this, mac, 0,
                                                        kh::FILTER_LIF_MAC);
}

void
EthLif::DeleteMacFilter(mac_t mac)
{
    std::map<mac_vlan_filter_t, MacVlanFilter*>::iterator it;
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC, mac, 0);

    it = mac_vlan_filter_table.find(key);
    MacVlanFilter *filter = it->second;

    mac_vlan_filter_table.erase(it);
    MacVlanFilter::Destroy(filter);
}

// Should be called only in case of smart.
void
EthLif::CreateVlanFilter(vlan_t vlan)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_VLAN, 0, vlan);

    mac_vlan_filter_table[key] = MacVlanFilter::Factory(this, 0, vlan,
                                                        kh::FILTER_LIF_VLAN);
}

void
EthLif::DeleteVlanFilter(vlan_t vlan)
{
    std::map<mac_vlan_filter_t, MacVlanFilter*>::iterator it;
    mac_vlan_filter_t key(kh::FILTER_LIF_VLAN, 0, vlan);

    it = mac_vlan_filter_table.find(key);
    MacVlanFilter *filter = it->second;

    mac_vlan_filter_table.erase(it);
    MacVlanFilter::Destroy(filter);
}


Lif *
EthLif::GetLif()
{
    return lif;
}

Uplink *
EthLif::GetUplink()
{
    return uplink;
}

Enic *
EthLif::GetEnic()
{
    return enic;
}

uint32_t
EthLif::GetHwLifId()
{
    return hw_lif_id;
}
