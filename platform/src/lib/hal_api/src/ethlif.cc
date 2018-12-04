//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Manages eth lif device
//-----------------------------------------------------------------------------
#include <iostream>
#include <cmath>

#include "nic/include/storage_seq_common.h"

#include "ethlif.hpp"
#include "print.hpp"
#include "utils.hpp"

using namespace std;

const char qstate_64[64] = { 0 };
const char qstate_1024[1024] = { 0 };

sdk::lib::indexer *EthLif::filter_allocator = sdk::lib::indexer::factory(EthLif::max_filters_per_lif, false, true);
EthLif *EthLif::internal_mgmt_ethlif = NULL;

EthLifMap EthLif::ethlif_db;

EthLif *
EthLif::Factory(hal_lif_info_t *info)
{
    HalL2Segment *native_l2seg;
    HalVrf       *vrf;

    api_trace("EthLif Create");

    if (info->hw_lif_id != 0) {
        if (ethlif_db.find(info->hw_lif_id) != ethlif_db.end()) {
            NIC_LOG_WARN("Duplicate Create of EthLif with id: {}",
                           info->hw_lif_id);
            return NULL;
        }
    }

    EthLif *eth_lif = new EthLif(info);

    if (eth_lif->IsInternalManagementMnic()) {
        NIC_LOG_DEBUG("lif-{}:Setting Internal management lif",
                     info->hw_lif_id);
        EthLif::SetInternalMgmtEthLif(eth_lif);
    }

    if (eth_lif->GetUplink() == NULL) {
        // HW: Admin Lif.
        // DOL: HAL_LIF_ID_NICMGR_MIN lifs created in DevMgr constructor
        if (!eth_lif->IsInternalManagement()) {
            // All LIFs with uplink NULL except internal mgmt mnic and host mgmt.
            NIC_LOG_DEBUG("lif-{}: Uplink is NULL", eth_lif->GetHwLifId());
            goto end;
        }
    }

#if 0
    // Only for storage lif STORAGE_SEQ_SW_LIF_ID uplink is NULL
    // if (eth_lif->GetUplink() == NULL) {
    if (info->id == STORAGE_SEQ_SW_LIF_ID) {
        NIC_LOG_DEBUG("Storage lif id: {}. Skip fwd entities creation.",
                     info->id);
        // NIC_LOG_DEBUG("lif-{}: Uplink is NULL", eth_lif->GetHwLifId());
        goto end;
    }
#endif


    // Skipping for Internal mgmt mnic
    if (eth_lif->GetUplink()) {
        eth_lif->GetUplink()->IncNumLifs();
    }

    // Store in DB for disruptive upgrade
    ethlif_db[eth_lif->GetHwLifId()] = eth_lif;

    if (eth_lif->IsInternalManagement()) {
        // For Internal Mgmt mnic, create vrf and native l2seg.
        if (eth_lif->IsInternalManagementMnic()) {
            // Create Internal Mgmt Vrf
            vrf = HalVrf::Factory(types::VRF_TYPE_INTERNAL_MANAGEMENT);
            eth_lif->SetVrf(vrf);
            // Create native l2seg to hal
            native_l2seg = HalL2Segment::Factory(eth_lif->GetVrf(),
                                                 NATIVE_VLAN_ID);
            eth_lif->SetNativeL2Seg(native_l2seg);
        }
    } else {
        if (eth_lif->GetUplink()->GetNumLifs() == 1) {

            NIC_LOG_INFO("First lif id: {}, hw_id: {} on uplink {}",
                         eth_lif->GetLif()->GetId(),
                         eth_lif->GetHwLifId(),
                         eth_lif->GetUplink()->GetId());

            // Create native l2seg to hal
            native_l2seg = HalL2Segment::Factory(eth_lif->GetUplink()->GetVrf(),
                                                 NATIVE_VLAN_ID);

            // Update uplink structure with native l2seg
            eth_lif->GetUplink()->SetNativeL2Seg(native_l2seg);

            // Update native_l2seg on uplink to hal
            eth_lif->GetUplink()->UpdateHalWithNativeL2seg(native_l2seg->GetId());

            // Update l2seg's mbrifs on uplink
            native_l2seg->AddUplink(eth_lif->GetUplink());
        }
    }

    // Create Enic
    eth_lif->SetEnic(Enic::Factory(eth_lif));

    // Add Vlan filter on ethlif
    eth_lif->AddVlan(NATIVE_VLAN_ID);

    if (hal->GetMode() == FWD_MODE_SMART) {
        // If its promiscuos. send (Lif, *, *) filter to HAL
        if (info->receive_promiscuous) {
            eth_lif->CreateMacVlanFilter(0, 0);
        }
    }

#if 0
    // Create Enic for every Lif in Classic Mode
    if (eth_lif->IsClassicForwarding()) {
        if (eth_lif->IsInternalManagement()) {
            // For Internal Mgmt mnic, create vrf and native l2seg.
            if (eth_lif->IsInternalManagementMnic()) {
                // Create Internal Mgmt Vrf
                vrf = HalVrf::Factory(types::VRF_TYPE_MANAGEMENT);
                eth_lif->SetVrf(vrf);
                // Create native l2seg to hal
                native_l2seg = HalL2Segment::Factory(eth_lif->GetVrf(),
                                                     NATIVE_VLAN_ID);
                eth_lif->SetNativeL2Seg(native_l2seg);
            }
        } else {
            if (eth_lif->GetUplink()->GetNumLifs() == 1) {

                NIC_LOG_DEBUG("First lif id: {}, hw_id: {} on uplink {}",
                             eth_lif->GetLif()->GetId(),
                             eth_lif->GetHwLifId(),
                             eth_lif->GetUplink()->GetId());

                // Create native l2seg to hal
                native_l2seg = HalL2Segment::Factory(eth_lif->GetUplink()->GetVrf(),
                                                     NATIVE_VLAN_ID);

                // Update uplink structure with native l2seg
                eth_lif->GetUplink()->SetNativeL2Seg(native_l2seg);

                // Update native_l2seg on uplink to hal
                eth_lif->GetUplink()->UpdateHalWithNativeL2seg(native_l2seg->GetId());

                // Update l2seg's mbrifs on uplink
                native_l2seg->AddUplink(eth_lif->GetUplink());
            }
        }

        // Create Enic
        eth_lif->SetEnic(Enic::Factory(eth_lif));

        // Add Vlan filter on ethlif
        eth_lif->AddVlan(NATIVE_VLAN_ID);

    } else {
        // If its promiscuos. send (Lif, *, *) filter to HAL
        if (info->receive_promiscuous) {
            eth_lif->CreateMacVlanFilter(0, 0);
        }
    }
#endif

end:
    return eth_lif;
}

void
EthLif::Destroy(EthLif *eth_lif)
{
    api_trace("EthLif Delete");

    // Remove from DB
    ethlif_db.erase(eth_lif->GetHwLifId());

    if (eth_lif) {

        eth_lif->remove_mac_filters();
        eth_lif->remove_vlan_filters();
        eth_lif->remove_mac_vlan_filters();

        if (eth_lif->GetUplink()) {
            eth_lif->GetUplink()->DecNumLifs();
        }

        if (eth_lif->IsClassicForwarding()) {
            // Delete Vlan filter
            eth_lif->DelVlan(NATIVE_VLAN_ID);

            // Delete enic
            Enic::Destroy(eth_lif->GetEnic());

            if (eth_lif->IsInternalManagement()) {
                if (eth_lif->IsInternalManagementMnic()) {
                    // Delete L2seg
                    HalL2Segment::Destroy(eth_lif->GetNativeL2Seg());
                    eth_lif->SetNativeL2Seg(NULL);
                    // Delete Vrf
                    HalVrf::Destroy(eth_lif->GetVrf());
                    eth_lif->SetVrf(NULL);
                }
            } else {

                if (eth_lif->GetUplink()->GetNumLifs() == 0) {
                    NIC_LOG_DEBUG("Last lif id: {}, hw_id: {} on uplink {}",
                                 eth_lif->GetLif()->GetId(),
                                 eth_lif->GetHwLifId(),
                                 eth_lif->GetUplink()->GetId());

                    eth_lif->GetUplink()->GetNativeL2Seg()->DelUplink(eth_lif->GetUplink());

                    // Update native_l2seg on uplink to hal
                    eth_lif->GetUplink()->UpdateHalWithNativeL2seg(0);

                    // Delete L2seg
                    HalL2Segment::Destroy(eth_lif->GetUplink()->GetNativeL2Seg());

                    eth_lif->GetUplink()->SetNativeL2Seg(NULL);
                }
            }
        } else {
            if (eth_lif->GetIsPromiscuous()) {
                eth_lif->DeleteMacVlanFilter(0, 0);
            }
        }

        eth_lif->~EthLif();
    }
}

EthLif::EthLif(hal_lif_info_t *info)
{
    memcpy(&info_, info, sizeof(hal_lif_info_t));

    vrf_ = NULL;

    lif_ = Lif::Factory(this);
}

EthLif::~EthLif()
{
    mac_table_.clear();
    vlan_table_.clear();
    mac_vlan_table_.clear();
    mac_vlan_filter_table.clear();

    // Delete Lif
    Lif::Destroy(lif_);
}

hal_irisc_ret_t
EthLif::remove_mac_filters()
{
    mac_t mac;
    // Remove mac filters
    for (auto it = mac_table_.begin(); it != mac_table_.end();) {
        mac = *it;
        it++;
        DelMac(mac);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::remove_vlan_filters()
{
    vlan_t vlan;
    // Remove vlan filters
    for (auto it = vlan_table_.begin(); it != vlan_table_.end();) {
        vlan = *it;
        it++;
        DelVlan(vlan);
    }

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::remove_mac_vlan_filters()
{
    mac_t mac;
    vlan_t vlan;
    // Remove (mac,vlan) filters
    for (auto it = mac_vlan_table_.begin(); it != mac_vlan_table_.end(); it++) {
        mac = std::get<0>(*it);
        vlan = std::get<1>(*it);
        it++;
        DelMacVlan(mac,vlan);
    }
    return HAL_IRISC_RET_SUCCESS;
}

/**
 * @re_add: Mac is already added without any registration. So re-adding.
 *          Happens for ALL_MC filter.
 */
hal_irisc_ret_t
EthLif::AddMac(mac_t mac, bool re_add)
{
    mac_vlan_t mac_vlan;
    bool       skip_registration = false;

    api_trace("Adding Mac Filter");
    NIC_LOG_DEBUG("Adding Mac filter: {}", macaddr2str(mac));

    // re_add
    // - True: Has to exist and should not be counted against max limit
    // - False: Should not exists and should be counted against max limit
    if ((re_add && mac_table_.find(mac) != mac_table_.end()) ||
        mac_table_.find(mac) == mac_table_.end()) {

        // Check if max limit reached
        if (!re_add && mac_table_.size() == info_.max_mac_filters) {
            NIC_LOG_ERR("Reached Max Mac filter limit of {} for lif: {}",
                          info_.max_mac_filters,
                          GetHwLifId());
            return HAL_IRISC_RET_LIMIT_REACHED;
        }

        /*
         * Classic:
         *      - Walk through Vlans and create (Mac,Vlan) filters
         * Smart:
         *      - Create Mac filter
         */
        if (IsClassicForwarding()) {

            if (is_multicast(mac) && IsReceiveAllMulticast()) {
                skip_registration = true;
            }

            if (!skip_registration) {
                // Register new mac across all existing vlans
                for (auto vlan_it = vlan_table_.cbegin(); vlan_it != vlan_table_.cend(); vlan_it++) {
                    // Check if (MacVlan) filter is already present
                    mac_vlan = make_tuple(mac, *vlan_it);
                    if (mac_vlan_table_.find(mac_vlan) == mac_vlan_table_.end()) {
                        // No (MacVlan) filter. Creating (Mac, Vlan)
                        CreateMacVlanFilter(mac, *vlan_it);
                    } else {
                        NIC_LOG_DEBUG("(Mac,Vlan) filter present. No-op");
                    }
                }
            }
        } else {
            CreateMacFilter(mac);
        }

        if (!re_add) {
            // Store mac filter
            mac_table_.insert(mac);
        }
    } else {
        NIC_LOG_WARN("Mac already registered: {}", macaddr2str(mac));
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::DelMac(mac_t mac, bool update_db)
{
    mac_vlan_t mac_vlan_key, mac_key, vlan_key;
    bool skipped_registration = false;

    api_trace("Deleting Mac Filter");
    NIC_LOG_DEBUG("Deleting Mac filter: {}", macaddr2str(mac));

    mac_key = make_tuple(mac, 0);
    if (mac_table_.find(mac) != mac_table_.end()) {
        if (IsClassicForwarding()) {

            if (is_multicast(mac)) {
                if (IsReceiveAllMulticast()) {
                    skipped_registration = true;
                }
            }

            if (!skipped_registration) {
                for (auto vlan_it = vlan_table_.cbegin(); vlan_it != vlan_table_.cend(); vlan_it++) {
                    vlan_key = make_tuple(0, *vlan_it);
                    mac_vlan_key = make_tuple(mac, *vlan_it);
                    if (vlan_table_.find(*vlan_it) != vlan_table_.end() &&
                        mac_vlan_table_.find(mac_vlan_key) == mac_vlan_table_.end()) {
                        NIC_LOG_DEBUG("Mac Delete: Mac, Vlan are present but (Mac,Vlan) is not. Remove (Mac,Vlan) entity");
                        // Mac, Vlan are present and (Mac,Vlan) is not
                        DeleteMacVlanFilter(mac, *vlan_it);
                    } else {
                        // Case:
                        //  Case 1: Vlan filter not present but (Mac,Vlan) is either present or not.
                        //  Case 2: Vlan filter is present along with (Mac,Vlan)
                        NIC_LOG_DEBUG("Mac Delete: No-op");
                    }
                }
            }
        } else {
            DeleteMacFilter(mac);
        }

        if (update_db) {
            // Erase mac filter
            mac_table_.erase(mac);
        }
    } else {
        NIC_LOG_ERR("Mac not registered: {}", mac);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::AddVlan(vlan_t vlan)
{
    mac_vlan_t mac_vlan;

    api_trace("Adding Vlan Filter");
    NIC_LOG_DEBUG("Adding Vlan filter: {}", vlan);
    if (!vlan) {
        vlan = 8192;
    }

    if (vlan_table_.find(vlan) == vlan_table_.end()) {
        // Check if max limit reached
        if (vlan_table_.size() == info_.max_vlan_filters) {
            NIC_LOG_ERR("Reached Max Vlan filter limit of {} for lif: {}",
                          info_.max_vlan_filters,
                          GetHwLifId());
            return HAL_IRISC_RET_LIMIT_REACHED;
        }

        /*
         * Classic:
         *      - Walk through Vlans and create (Mac,Vlan) filters
         * Smart:
         *      - Create Vlan filter
         */
        if (IsClassicForwarding()) {
            // Register new mac across all existing vlans
            for (auto it = mac_table_.cbegin(); it != mac_table_.cend(); it++) {
                if (is_multicast(*it) && IsReceiveAllMulticast()) {
                    // skip Mcast macs if ALL_MC is set
                    continue;
                }
                // Check if (MacVlan) filter is already present
                mac_vlan = make_tuple(*it, vlan);
                if (mac_vlan_table_.find(mac_vlan) == mac_vlan_table_.end()) {
                    // No (MacVlan) filter. Creating (Mac, Vlan)
                    CreateMacVlanFilter(*it, vlan);
                } else {
                    NIC_LOG_DEBUG("(Mac,Vlan) filter present. No-op");
                }
            }
        } else {
            CreateVlanFilter(vlan);
        }

        // Store vlan filter
        vlan_table_.insert(vlan);
    } else {
        NIC_LOG_WARN("Vlan already registered: {}", vlan);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::DelVlan(vlan_t vlan)
{
    mac_vlan_t mac_vlan_key;

    api_trace("Deleting Vlan Filter");
    NIC_LOG_DEBUG("Deleting Vlan filter: {}", vlan);
    if (!vlan) {
        NIC_LOG_DEBUG("Ignoring Delete of Vlan filter 0");
        return HAL_IRISC_RET_SUCCESS;
    }

    if (vlan_table_.find(vlan) != vlan_table_.end()) {
        if (IsClassicForwarding()) {
            for (auto it = mac_table_.cbegin(); it != mac_table_.cend(); it++) {
                if (is_multicast(*it) && IsReceiveAllMulticast()) {
                    // skip Mcast macs if ALL_MC is set
                    continue;
                }
                mac_vlan_key = make_tuple(*it, vlan);
                if (mac_table_.find(*it) != mac_table_.end() &&
                    mac_vlan_table_.find(mac_vlan_key) == mac_vlan_table_.end()) {
                    NIC_LOG_DEBUG("Vlan Delete: Mac, Vlan are present but (Mac,Vlan) is not. Remove (Mac,Vlan) entity");
                    // Mac, Vlan are present and (Mac,Vlan) is not
                    DeleteMacVlanFilter(*it, vlan);
                } else {
                    // Case:
                    //  Case 1: Mac filter not present but (Mac,Vlan) is either present or not.
                    //  Case 2: Mac filter is present along with (Mac,Vlan)
                    NIC_LOG_DEBUG("Vlan Delete: No-op");
                }
            }
        } else {
            DeleteVlanFilter(vlan);
        }

        // Erase mac filter
        vlan_table_.erase(vlan);
    } else {
        NIC_LOG_ERR("Vlan not registered: {}", vlan);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::AddMacVlan(mac_t mac, vlan_t vlan)
{
    mac_vlan_t key(mac, vlan);

    api_trace("Adding (Mac,Vlan) Filter");
    NIC_LOG_DEBUG("Adding (Mac,Vlan) mac: {}, filter: {}", macaddr2str(mac), vlan);

    if (mac_vlan_table_.find(key) == mac_vlan_table_.end()) {
        // Check if max limit reached
        if (mac_vlan_table_.size() == info_.max_mac_vlan_filters) {
            NIC_LOG_ERR("Reached Max Mac-Vlan filter limit of {} for lif: {}",
                          info_.max_mac_vlan_filters,
                          GetHwLifId());
            return HAL_IRISC_RET_LIMIT_REACHED;
        }
        if (IsClassicForwarding()) {
            // Check if mac filter and vlan filter is present
            if (mac_table_.find(mac) == mac_table_.end() ||
                vlan_table_.find(vlan) == vlan_table_.end()) {
                CreateMacVlanFilter(mac, vlan);
            } else {
                NIC_LOG_DEBUG("Mac filter and Vlan filter preset. "
                                "No-op for (Mac,Vlan) filter");
            }
        } else {
            CreateMacVlanFilter(mac, vlan);
        }

        // Store mac-vlan filter
        mac_vlan_table_.insert(key);
    } else {
        NIC_LOG_WARN("Mac-Vlan already registered: {}", mac);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::DelMacVlan(mac_t mac, vlan_t vlan)
{
    mac_vlan_t mac_vlan_key;

    api_trace("Deleting (Mac,Vlan) Filter");
    NIC_LOG_DEBUG("Deleting (Mac,Vlan) mac: {}, filter: {}", macaddr2str(mac), vlan);

    mac_vlan_key = make_tuple(mac, vlan);
    if (mac_vlan_table_.find(mac_vlan_key) != mac_vlan_table_.end()) {
        if (IsClassicForwarding()) {
            if (mac_table_.find(mac) == mac_table_.end() ||
                 vlan_table_.find(vlan) == vlan_table_.end()) {
                // One of Mac or Vlan is not present.
                // (Mac,Vlan) entity was created only by (Mac,Vlan) filter
                DeleteMacVlanFilter(mac, vlan);
            } else {
                // Mac filter and Vlan filter both exist
                NIC_LOG_DEBUG("Mac filter and Vlan filter present. "
                                "No-op for (Mac,Vlan) filter");
            }
        } else {
            DeleteMacVlanFilter(mac, vlan);
        }
        // Erase mac-vlan filter
        mac_vlan_table_.erase(mac_vlan_key);
    } else {
        NIC_LOG_ERR("(Mac,Vlan) already not registered: mac: {}, vlan: {}",
                      mac, vlan);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::UpdateReceivePromiscuous(bool receive_promiscuous)
{
    api_trace("Promiscuous Flag change");
    if (receive_promiscuous == info_.receive_promiscuous) {
        NIC_LOG_WARN("Prom flag: {}. No change in promiscuous flag. Nop",
                       receive_promiscuous);
        goto end;
    }

    NIC_LOG_DEBUG("Lif: {}. Prom. flag change {} -> {}",
                    lif_->GetId(), info_.receive_promiscuous,
                    receive_promiscuous);

    info_.receive_promiscuous = receive_promiscuous;

    if (IsClassicForwarding()) {
    } else {
        if (receive_promiscuous) {
            CreateMacVlanFilter(0, 0);
        } else {
            DeleteMacVlanFilter(0, 0);
        }
    }

    // Update Lif to Hal
    lif_->TriggerHalUpdate();
end:
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::UpdateReceiveBroadcast(bool receive_broadcast)
{
    api_trace("Broadcast change");
    if (receive_broadcast == info_.receive_broadcast) {
        NIC_LOG_WARN("Prom flag: {}. No change in broadcast flag. Nop",
                       receive_broadcast);
        goto end;
    }

    NIC_LOG_DEBUG("Lif: {}. Prom. flag change {} -> {}",
                    lif_->GetId(), info_.receive_broadcast,
                    receive_broadcast);

    info_.receive_broadcast = receive_broadcast;

    // Update Lif to Hal
    lif_->TriggerHalUpdate();
end:
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::UpdateReceiveAllMulticast(bool receive_all_multicast)
{
    api_trace("AllHalMulticast change");
    if (receive_all_multicast == info_.receive_all_multicast) {
        NIC_LOG_WARN("ALL_MC flag: {}. No change in all_multicast flag. Nop",
                       receive_all_multicast);
        goto end;
    }

    NIC_LOG_DEBUG("Lif: {}. ALL_MC flag change {} -> {}",
                    lif_->GetId(), info_.receive_all_multicast,
                    receive_all_multicast);

    /*
     * False -> True
     * - False: Call RemoveMCFilters
     * True -> False
     * - False: Call AddMCFilters
     */

    if (receive_all_multicast) {
        // Enable ALL_MC: Remove all MC filters on this lif
        RemoveMCFilters();
        info_.receive_all_multicast = receive_all_multicast;
    } else {
        // Disable ALL_MC: Add all MC filters on this lif
        info_.receive_all_multicast = receive_all_multicast;
        AddMCFilters();
    }


    // Update Lif to Hal
    lif_->TriggerHalUpdate();
end:
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::UpdateVlanStripEn(bool vlan_strip_en)
{
    api_trace("Vlan Strip change");
    if (vlan_strip_en == info_.vlan_strip_en) {
        NIC_LOG_WARN("No change in Vlan strip. Nop",
                       vlan_strip_en);
        goto end;
    }

    NIC_LOG_DEBUG("Lif: {}. Vlan strip change {} -> {}",
                    lif_->GetId(), info_.vlan_strip_en,
                    vlan_strip_en);

    info_.vlan_strip_en = vlan_strip_en;

    // Update Lif to Hal
    lif_->TriggerHalUpdate();
end:
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
EthLif::UpdateVlanInsertEn(bool vlan_insert_en)
{
    api_trace("Vlan Insert change");
    if (vlan_insert_en == info_.vlan_insert_en) {
        NIC_LOG_WARN("Prom flag: {}. No change in broadcast flag. Nop",
                       vlan_insert_en);
        goto end;
    }

    NIC_LOG_DEBUG("Lif: {}. Prom. flag change {} -> {}",
                    lif_->GetId(), info_.vlan_insert_en,
                    vlan_insert_en);

    info_.vlan_insert_en = vlan_insert_en;

    // Update Lif to Hal
    lif_->TriggerHalUpdate();
end:
    return HAL_IRISC_RET_SUCCESS;
}

void
EthLif::CreateMacVlanFilter(mac_t mac, vlan_t vlan)
{
    mac_vlan_filter_t key;
    filter_type_t type;

    if (!mac && !vlan) {
        type = kh::FILTER_LIF;
    } else {
        type = kh::FILTER_LIF_MAC_VLAN;
    }

    key = make_tuple(type, mac, vlan);
    mac_vlan_filter_table[key] = MacVlanFilter::Factory(this, mac, vlan, type);
}

void
EthLif::DeleteMacVlanFilter(mac_t mac, vlan_t vlan)
{
    std::map<mac_vlan_filter_t, MacVlanFilter*>::iterator it;
    mac_vlan_filter_t key;
    filter_type_t type;

    if (!mac && !vlan) {
        type = kh::FILTER_LIF;
    } else {
        type = kh::FILTER_LIF_MAC_VLAN;
    }

    key = make_tuple(type, mac, vlan);
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

void
EthLif::AddMCFilters()
{
    mac_t mac;
    for (auto it = mac_table_.begin(); it != mac_table_.end();it++) {
        mac = *it;
        if (is_multicast(mac)) {
            NIC_LOG_DEBUG("Trigger multicast MAC:{} filter add", macaddr2str(mac));
            AddMac(mac, true);
        }
    }
}

void
EthLif::RemoveMCFilters()
{
    mac_t mac;
    for (auto it = mac_table_.begin(); it != mac_table_.end();it++) {
        mac = *it;
        if (is_multicast(mac)) {
            NIC_LOG_DEBUG("Trigger multicast MAC:{} filter del", macaddr2str(mac));
            DelMac(mac, false);
        }
    }
}


Lif *
EthLif::GetLif()
{
    return lif_;
}

Uplink *
EthLif::GetUplink()
{
    return info_.pinned_uplink;
}

Enic *
EthLif::GetEnic()
{
    return enic_;
}

void
EthLif::SetEnic(Enic *enic)
{
    enic_ = enic;
}

uint32_t
EthLif::GetHwLifId()
{
    return GetLif()->GetHwLifId();
}

bool
EthLif::GetIsPromiscuous()
{
    return info_.receive_promiscuous;
}

hal_lif_info_t *
EthLif::GetLifInfo()
{
    return &info_;
}

HalVrf *
EthLif::GetVrf()
{
    if (IsInternalManagementMnic()) {
        return vrf_;
    } else if (IsHostManagement()) {
        return internal_mgmt_ethlif->GetVrf();
    } else {
        return GetUplink()->GetVrf();
    }
}

HalL2Segment *
EthLif::GetNativeL2Seg()
{
    if (IsInternalManagementMnic()) {
        return native_l2seg_;
    } else if (IsHostManagement()) {
        return internal_mgmt_ethlif->GetNativeL2Seg();
    } else {
        return GetUplink()->GetNativeL2Seg();
    }

}

bool
EthLif::IsOOBMnic()
{
    return (info_.type ==
            types::LIF_TYPE_MNIC_OOB_MANAGEMENT);
#if 0
    Uplink *up = GetUplink();
    if (up) {
        return up->IsOOB();
    }
    return false;
#endif
}

bool
EthLif::IsInternalManagementMnic()
{
    return (info_.type ==
            types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT);
}

bool
EthLif::IsInbandManagementMnic()
{
    return (info_.type ==
            types::LIF_TYPE_MNIC_INBAND_MANAGEMENT);
}

bool
EthLif::IsMnic()
{
    return (IsOOBMnic() ||
            IsInternalManagementMnic() ||
            IsInbandManagementMnic());
}

bool
EthLif::IsHostManagement()
{
    return (info_.type == types::LIF_TYPE_HOST_MANAGEMENT);
}

bool
EthLif::IsInternalManagement()
{
    return (IsInternalManagementMnic() ||
            IsHostManagement());
}

bool
EthLif::IsReceiveAllMulticast()
{
    return info_.receive_all_multicast;
}

bool
EthLif::IsClassicForwarding()
{
    // Classic:
    // - All LIFs
    // Smart (Hostpin):
    // - All LIFs
    //   - Host Mgmt - All traffic
    //   - Host Data - Untag traffic
    //   - ARM Internal Mgmt - All traffic
    //   - ARM Internal Data - Untag traffic
    //   - ARM OOB - All traffic
    return true;
#if 0
    return (hal->GetMode() == FWD_MODE_CLASSIC ||
            IsOOBMnic() ||
            IsInternalManagementMnic() ||
            IsHostManagement());
#endif
}

