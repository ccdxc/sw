//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Manages eth lif device
//-----------------------------------------------------------------------------
#include <iostream>
#include <cmath>

#include "nic/include/storage_seq_common.h"

#include "lif.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "platform/src/lib/nicmgr/include/nicmgr_utils.hpp"

using namespace std;


Lif *Lif::internal_mgmt_ethlif = NULL;
LifMap Lif::ethlif_db;

Lif *
Lif::Factory(hal_lif_info_t *info)
{
    HalL2Segment *native_l2seg;
    HalVrf       *vrf;

    if (info->hw_lif_id != 0) {
        if (ethlif_db.find(info->hw_lif_id) != ethlif_db.end()) {
            NIC_LOG_WARN("Duplicate Create of Lif with id: {}",
                           info->hw_lif_id);
            return NULL;
        }
    }

    Lif *lif = new Lif(info);

    if (lif->IsInternalManagementMnic()) {
        NIC_LOG_DEBUG("lif-{}:Setting Internal management lif",
                     info->hw_lif_id);
        Lif::SetInternalMgmtEthLif(lif);
    }

    if (lif->GetUplink() == NULL) {
        // HW: Admin Lif.
        // DOL: HAL_LIF_ID_NICMGR_MIN lifs created in DevMgr constructor
        if (!lif->IsInternalManagement()) {
            // All LIFs with uplink NULL except internal mgmt mnic and host mgmt.
            NIC_LOG_DEBUG("lif-{}: Uplink is NULL", lif->GetHwLifId());
            goto end;
        }
    }

    // Skipping for Internal mgmt mnic
    if (lif->GetUplink()) {
        lif->GetUplink()->IncNumLifs();
    }

    // Store in DB for disruptive upgrade
    ethlif_db[lif->GetHwLifId()] = lif;

    if (lif->IsInternalManagement()) {
        // For Internal Mgmt mnic, create vrf and native l2seg.
        if (lif->IsInternalManagementMnic()) {
            // Create Internal Mgmt Vrf
            vrf = HalVrf::Factory(types::VRF_TYPE_INTERNAL_MANAGEMENT);
            lif->SetVrf(vrf);
            // Create native l2seg to hal
            native_l2seg = HalL2Segment::Factory(lif->GetVrf(),
                                                 NATIVE_VLAN_ID);
            lif->SetNativeL2Seg(native_l2seg);
        }
    } else {
        if (lif->GetUplink()->GetNumLifs() == 1) {

            NIC_LOG_INFO("First lif id: {}, hw_id: {} on uplink {}",
                         lif->GetId(),
                         lif->GetHwLifId(),
                         lif->GetUplink()->GetId());

            // Create native l2seg to hal
            native_l2seg = HalL2Segment::Factory(lif->GetUplink()->GetVrf(),
                                                 NATIVE_VLAN_ID);

            // Update uplink structure with native l2seg
            lif->GetUplink()->SetNativeL2Seg(native_l2seg);

            // Update native_l2seg on uplink to hal
            lif->GetUplink()->UpdateHalWithNativeL2seg(native_l2seg->GetId());

            // Update l2seg's mbrifs on uplink
            // native_l2seg->AddUplink(lif->GetUplink());
        }
    }

    // Create Enic
    lif->SetEnic(Enic::Factory(lif));

    // Add Vlan filter on Lif
    lif->AddVlan(NATIVE_VLAN_ID);

    if (!lif->IsClassicForwarding()) {
        // If its promiscuos. send (Lif, *, *) filter to HAL
        if (info->receive_promiscuous) {
            lif->CreateMacVlanFilter(0, 0);
        }
    }

end:
    return lif;
}

void
Lif::Reset()
{
    api_trace("Resetting Lif");

    // Remove Filters
    RemoveMacFilters();
    RemoveVlanFilters(true /* skip_native_vlan */);
    RemoveMacVlanFilters();

    // Reset Vlan Offload
    UpdateVlanOffload(
        false /* Vlan Strip */,
        false /* Vlan Insert */
    );

    // Reset Rx Modes
    UpdateReceiveMode(
        false /* Broadcast */,
        false /* All Multicast */,
        false /* Promiscuous */
    );
}

void
Lif::Destroy(Lif *lif)
{
    api_trace("Lif Delete");

    // Remove from DB
    ethlif_db.erase(lif->GetHwLifId());

    if (lif) {

        lif->RemoveMacFilters();
        lif->RemoveVlanFilters();
        lif->RemoveMacVlanFilters();

        if (lif->GetUplink()) {
            lif->GetUplink()->DecNumLifs();
        }

        if (lif->IsClassicForwarding()) {
            // Delete Vlan filter
            lif->DelVlan(NATIVE_VLAN_ID);

            // Delete enic
            Enic::Destroy(lif->GetEnic());

            if (lif->IsInternalManagement()) {
                if (lif->IsInternalManagementMnic()) {
                    // Delete L2seg
                    HalL2Segment::Destroy(lif->GetNativeL2Seg());
                    lif->SetNativeL2Seg(NULL);
                    // Delete Vrf
                    HalVrf::Destroy(lif->GetVrf());
                    lif->SetVrf(NULL);
                }
            } else {

                if (lif->GetUplink()->GetNumLifs() == 0) {
                    NIC_LOG_DEBUG("Last lif id: {}, hw_id: {} on uplink {}",
                                 lif->GetId(),
                                 lif->GetHwLifId(),
                                 lif->GetUplink()->GetId());

                    lif->GetUplink()->GetNativeL2Seg()->DelUplink(lif->GetUplink());

                    // Update native_l2seg on uplink to hal
                    lif->GetUplink()->UpdateHalWithNativeL2seg(0);

                    // Delete L2seg
                    HalL2Segment::Destroy(lif->GetUplink()->GetNativeL2Seg());

                    lif->GetUplink()->SetNativeL2Seg(NULL);
                }
            }
        } else {
            if (lif->GetIsPromiscuous()) {
                lif->DeleteMacVlanFilter(0, 0);
            }
        }

        lif->~Lif();
    }
}

Lif::Lif(hal_lif_info_t *info)
{
    memcpy(&info_, info, sizeof(hal_lif_info_t));

    vrf_ = NULL;

    // lif_ = Lif::Factory(this);
    TriggerHalCreate();
}

Lif::~Lif()
{
    mac_table_.clear();
    vlan_table_.clear();
    mac_vlan_table_.clear();
    mac_vlan_filter_table.clear();

    // Delete Lif
    // Lif::Destroy(lif_);
    TriggerHalDelete();
}

/**
 * @re_add: Mac is already added without any registration. So re-adding.
 *          Happens for ALL_MC filter.
 */
hal_irisc_ret_t
Lif::AddMac(mac_t mac, bool re_add)
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
        return HAL_IRISC_DUP_ADDR_ADD;
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::DelMac(mac_t mac, bool update_db)
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
Lif::AddVlan(vlan_t vlan)
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
Lif::DelVlan(vlan_t vlan, bool update_db)
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

        if (update_db) {
            // Erase vlan filter
            vlan_table_.erase(vlan);
        }
    } else {
        NIC_LOG_ERR("Vlan not registered: {}", vlan);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::AddMacVlan(mac_t mac, vlan_t vlan)
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
Lif::DelMacVlan(mac_t mac, vlan_t vlan, bool update_db)
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
        if (update_db) {
            // Erase mac-vlan filter
            mac_vlan_table_.erase(mac_vlan_key);
        }
    } else {
        NIC_LOG_ERR("(Mac,Vlan) already not registered: mac: {}, vlan: {}",
                      mac, vlan);
    }
    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateReceiveMode(bool broadcast, bool all_multicast, bool promiscuous)
{
    hal_irisc_ret_t status;

    if (broadcast != info_.receive_broadcast ||
        all_multicast != info_.receive_all_multicast ||
        promiscuous != info_.receive_promiscuous) {

        status = UpdateReceivePromiscuous(promiscuous);
        if (status != HAL_IRISC_RET_SUCCESS) {
            return status;
        }

        status = UpdateReceiveAllMulticast(all_multicast);
        if (status != HAL_IRISC_RET_SUCCESS) {
            return status;
        }

        status = UpdateReceiveBroadcast(broadcast);
        if (status != HAL_IRISC_RET_SUCCESS) {
            return status;
        }

        TriggerHalUpdate();

    } else {
        NIC_LOG_DEBUG("Lif: {}. No rx mode change.", GetId());
    }

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateReceivePromiscuous(bool receive_promiscuous)
{
    if (receive_promiscuous == info_.receive_promiscuous) {
        NIC_LOG_WARN("Prom flag: {}. No change in promiscuous flag. Nop",
                       receive_promiscuous);
        return HAL_IRISC_RET_SUCCESS;
    }

    NIC_LOG_DEBUG("Lif: {}. Prom. flag change {} -> {}",
                  GetId(), info_.receive_promiscuous,
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

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateReceiveBroadcast(bool receive_broadcast)
{
    if (receive_broadcast == info_.receive_broadcast) {
        NIC_LOG_WARN("Bcast flag: {}. No change in broadcast flag. Nop",
                       receive_broadcast);
        return HAL_IRISC_RET_SUCCESS;
    }

    NIC_LOG_DEBUG("Lif: {}. Prom. flag change {} -> {}",
                    GetId(), info_.receive_broadcast,
                    receive_broadcast);

    info_.receive_broadcast = receive_broadcast;

    // Update Lif to Hal
    TriggerHalUpdate();

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateReceiveAllMulticast(bool receive_all_multicast)
{
    if (receive_all_multicast == info_.receive_all_multicast) {
        NIC_LOG_WARN("ALL_MC flag: {}. No change in all_multicast flag. Nop",
                       receive_all_multicast);
        return HAL_IRISC_RET_SUCCESS;
    }

    NIC_LOG_DEBUG("Lif: {}. ALL_MC flag change {} -> {}",
                  GetId(), info_.receive_all_multicast,
                  receive_all_multicast);

    /*
     * False -> True
     * - False: Call DeProgramMCFilters
     * True -> False
     * - False: Call ProgramMCFilters
     */

    if (receive_all_multicast) {
        // Enable ALL_MC: Remove all MC filters on this lif
        DeProgramMCFilters();
        info_.receive_all_multicast = receive_all_multicast;
    } else {
        // Disable ALL_MC: Add all MC filters on this lif
        info_.receive_all_multicast = receive_all_multicast;
        ProgramMCFilters();
    }

    // Update Lif to Hal
    TriggerHalUpdate();

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateVlanOffload(bool vlan_strip, bool vlan_insert)
{
    hal_irisc_ret_t status;

    if (vlan_strip != info_.vlan_strip_en ||
        vlan_insert != info_.vlan_insert_en) {

        status = UpdateVlanStripEn(vlan_strip);
        if (status != HAL_IRISC_RET_SUCCESS) {
            return status;
        }

        status = UpdateVlanInsertEn(vlan_insert);
        if (status != HAL_IRISC_RET_SUCCESS) {
            return status;
        }

        TriggerHalUpdate();

    } else {
        NIC_LOG_DEBUG("Lif: {}. No vlan offload change.", GetId());
    }

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateVlanStripEn(bool vlan_strip_en)
{
    api_trace("Vlan Strip change");
    if (vlan_strip_en == info_.vlan_strip_en) {
        NIC_LOG_WARN("No change in Vlan strip. Nop",
                       vlan_strip_en);
        return HAL_IRISC_RET_SUCCESS;
    }

    NIC_LOG_DEBUG("Lif: {}. Vlan strip change {} -> {}",
                    GetId(), info_.vlan_strip_en,
                    vlan_strip_en);

    info_.vlan_strip_en = vlan_strip_en;

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateVlanInsertEn(bool vlan_insert_en)
{
    api_trace("Vlan Insert change");
    if (vlan_insert_en == info_.vlan_insert_en) {
        NIC_LOG_WARN("Prom flag: {}. No change in broadcast flag. Nop",
                       vlan_insert_en);
        return HAL_IRISC_RET_SUCCESS;
    }

    NIC_LOG_DEBUG("Lif: {}. Prom. flag change {} -> {}",
                  GetId(), info_.vlan_insert_en,
                  vlan_insert_en);

    info_.vlan_insert_en = vlan_insert_en;

    return HAL_IRISC_RET_SUCCESS;
}

hal_irisc_ret_t
Lif::UpdateName(std::string name)
{
    info_.name = name;
    TriggerHalUpdate();
    return HAL_IRISC_RET_SUCCESS;
}

void
Lif::CreateMacVlanFilter(mac_t mac, vlan_t vlan)
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
Lif::DeleteMacVlanFilter(mac_t mac, vlan_t vlan)
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
Lif::CreateMacFilter(mac_t mac)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC, mac, 0);

    mac_vlan_filter_table[key] = MacVlanFilter::Factory(this, mac, 0,
                                                        kh::FILTER_LIF_MAC);
}

void
Lif::DeleteMacFilter(mac_t mac)
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
Lif::CreateVlanFilter(vlan_t vlan)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_VLAN, 0, vlan);

    mac_vlan_filter_table[key] = MacVlanFilter::Factory(this, 0, vlan,
                                                        kh::FILTER_LIF_VLAN);
}

void
Lif::DeleteVlanFilter(vlan_t vlan)
{
    std::map<mac_vlan_filter_t, MacVlanFilter*>::iterator it;
    mac_vlan_filter_t key(kh::FILTER_LIF_VLAN, 0, vlan);

    it = mac_vlan_filter_table.find(key);
    MacVlanFilter *filter = it->second;

    mac_vlan_filter_table.erase(it);
    MacVlanFilter::Destroy(filter);
}

void
Lif::ProgramMCFilters()
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
Lif::DeProgramMCFilters()
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

void
Lif::RemoveMacFilters()
{
    mac_t mac;

    NIC_LOG_DEBUG("lif-{}: Removing Mac Filters", GetHwLifId());
    for (auto it = mac_table_.begin(); it != mac_table_.end();) {
        mac = *it;
        DelMac(mac, false);
        it = mac_table_.erase(it);
    }
    NIC_LOG_DEBUG("# of Mac Filters: {}", mac_table_.size());
}

void
Lif::RemoveVlanFilters(bool skip_native_vlan)
{
    vlan_t vlan;

    NIC_LOG_DEBUG("lif-{}: Removing Vlan Filters", GetHwLifId());

    for (auto it = vlan_table_.begin(); it != vlan_table_.end();) {
        vlan = *it;
        if (skip_native_vlan && vlan == NATIVE_VLAN_ID) {
            it++;
            continue;
        }
        DelVlan(vlan, false);
        it = vlan_table_.erase(it);
    }
    NIC_LOG_DEBUG("# of Vlan Filters: {}", vlan_table_.size());
}

void
Lif::RemoveMacVlanFilters()
{
    mac_t mac;
    vlan_t vlan;

    NIC_LOG_DEBUG("lif-{}: Removing Mac-Vlan Filters", GetHwLifId());
    for (auto it = mac_vlan_table_.begin(); it != mac_vlan_table_.end();) {
        mac = std::get<0>(*it);
        vlan = std::get<1>(*it);
        DelMacVlan(mac, vlan, false);
        it = mac_vlan_table_.erase(it);
    }
    NIC_LOG_DEBUG("# of Mac-Vlan Filters: {}", mac_vlan_table_.size());
}

void
Lif::TriggerHalCreate()
{
    grpc::ClientContext        context;
    grpc::Status               status;

    intf::LifSpec              *req;
    intf::LifResponse          rsp;
    intf::LifRequestMsg        req_msg;
    intf::LifResponseMsg       rsp_msg;
    intf::LifQStateMapEntry    *lif_qstate_map_ent;
    hal_lif_info_t             *lif_info = GetLifInfo();

    id_ = lif_info->hw_lif_id;
    assert(id_ != 0);

    // Set default number of max filters if nothing is passed
    if (!lif_info->max_vlan_filters) {
        lif_info->max_vlan_filters = LIF_DEFAULT_MAX_VLAN_FILTERS;
    }

    if (!lif_info->max_mac_filters) {
        lif_info->max_mac_filters = LIF_DEFAULT_MAX_MAC_FILTERS;
    }

    if (!lif_info->max_mac_vlan_filters) {
        lif_info->max_mac_vlan_filters = LIF_DEFAULT_MAX_MAC_VLAN_FILTERS;
    }

    NIC_LOG_DEBUG("Creating Lif: {}, prom: {}, oob: {}, int_mgmt_mnic: {}, host_mgmt_mnic: {}, rdma_en: {}",
                 lif_info->name,
                 lif_info->receive_promiscuous,
                 IsOOBMnic(),
                 IsInternalManagementMnic(),
                 IsHostManagement(),
                 lif_info->enable_rdma);

    PopulateRequest(req_msg, &req);

    // Populate qstate map
    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = lif_info->queue_info[i];
        NIC_LOG_DEBUG("Processing queue type: {}, size: {}", i, qinfo.size);
        if (qinfo.size < 1) continue;

        NIC_LOG_DEBUG("Queue type_num: {}, entries: {}, purpose: {}",
                      qinfo.type_num,
                      qinfo.entries, qinfo.purpose);

        lif_qstate_map_ent = req->add_lif_qstate_map();
        lif_qstate_map_ent->set_type_num(qinfo.type_num);
        lif_qstate_map_ent->set_size(qinfo.size);
        lif_qstate_map_ent->set_entries(qinfo.entries);
        lif_qstate_map_ent->set_purpose(qinfo.purpose);
    }

    if (!hal) {
        NIC_LOG_ERR("FATAL: HAL is not UP yet.");
        NIC_ASSERT(0);
    }

    status = hal->lif_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Created Lif id: {} hw_lif_id: {}",
                          id_, lif_info->hw_lif_id);
        } else {
            NIC_LOG_ERR("Failed to create Lif for hw_lif_id: {}. err: {}",
                        lif_info->hw_lif_id, rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to create Lif for hw_lif_id: {}. err: {}:{}",
                     lif_info->hw_lif_id, status.error_code(), status.error_message());
    }

    // Store spec
    spec.CopyFrom(*req);
}

void
Lif::TriggerHalUpdate()
{
    grpc::ClientContext        context;
    grpc::Status               status;

    intf::LifSpec              *req;
    intf::LifResponse          rsp;
    intf::LifRequestMsg        req_msg;
    intf::LifResponseMsg       rsp_msg;
    hal_lif_info_t             *lif_info = GetLifInfo();

    PopulateRequest(req_msg, &req);

    status = hal->lif_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Updated Lif id: {} hw_lif_id: {}",
                            id_, lif_info->hw_lif_id);
        } else {
            NIC_LOG_ERR("Failed to update Lif for id: {}. err: {}",
                          id_, rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update Lif for id: {}. err: {}:{}",
                      id_, status.error_code(), status.error_message());
    }

    // Store spec
    spec.CopyFrom(*req);
}

void
Lif::TriggerHalDelete()
{
    grpc::ClientContext           context;
    grpc::Status                  status;

    intf::LifDeleteRequest        *req;
    intf::LifDeleteResponse       rsp;
    intf::LifDeleteRequestMsg     req_msg;
    intf::LifDeleteResponseMsg    rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    status = hal->lif_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Deleted Lif id: {} hw_lif_id: {}",
                            id_, GetLifInfo()->hw_lif_id);
        } else {
            NIC_LOG_ERR("Failed to delete Lif for id: {}. err: {}",
                          id_, rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to delete Lif for id: {}. err: {}:{}",
                      id_, status.error_code(), status.error_message());
    }
}

void
Lif::PopulateRequest(intf::LifRequestMsg &req_msg, intf::LifSpec **req_ptr)
{
    hal_lif_info_t             *lif_info = GetLifInfo();
    intf::LifSpec              *req;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    req->set_name(lif_info->name);
    req->set_type(lif_info->type);
    req->set_hw_lif_id(lif_info->hw_lif_id);
    req->mutable_pinned_uplink_if_key_handle()->
        set_interface_id(GetUplink() ? GetUplink()->GetId() : 0);
        // set_interface_id(lif_info->pinned_uplink ? lif_info->pinned_uplink->GetId() : 0);
    req->mutable_packet_filter()->set_receive_broadcast(lif_info->receive_broadcast);
    req->mutable_packet_filter()->set_receive_all_multicast(lif_info->receive_all_multicast);
    req->mutable_packet_filter()->set_receive_promiscuous(lif_info->receive_promiscuous);
    req->set_vlan_strip_en(lif_info->vlan_strip_en);
    req->set_vlan_insert_en(lif_info->vlan_insert_en);
    req->set_is_management(IsOOBMnic() ||
                           IsInternalManagement());
    req->set_admin_status(::intf::IF_STATUS_UP);
    req->set_enable_rdma(lif_info->enable_rdma);

    *req_ptr = req;
}

Uplink *
Lif::GetUplink()
{
    // return info_.pinned_uplink;
    return Uplink::GetUplink(info_.pinned_uplink_port_num);
}

Enic *
Lif::GetEnic()
{
    return enic_;
}

void
Lif::SetEnic(Enic *enic)
{
    enic_ = enic;
}

uint32_t
Lif::GetId()
{
    return id_;
}

uint32_t
Lif::GetHwLifId()
{
    return info_.hw_lif_id;
}

bool
Lif::GetIsPromiscuous()
{
    return info_.receive_promiscuous;
}

hal_lif_info_t *
Lif::GetLifInfo()
{
    return &info_;
}

HalVrf *
Lif::GetVrf()
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
Lif::GetNativeL2Seg()
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
Lif::IsOOBMnic()
{
    return (info_.type ==
            types::LIF_TYPE_MNIC_OOB_MANAGEMENT);
}

bool
Lif::IsInternalManagementMnic()
{
    return (info_.type ==
            types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT);
}

bool
Lif::IsInbandManagementMnic()
{
    return (info_.type ==
            types::LIF_TYPE_MNIC_INBAND_MANAGEMENT);
}

bool
Lif::IsMnic()
{
    return (IsOOBMnic() ||
            IsInternalManagementMnic() ||
            IsInbandManagementMnic());
}

bool
Lif::IsHostManagement()
{
    return (info_.type == types::LIF_TYPE_HOST_MANAGEMENT);
}

bool
Lif::IsInternalManagement()
{
    return (IsInternalManagementMnic() ||
            IsHostManagement());
}

bool
Lif::IsReceiveAllMulticast()
{
    return info_.receive_all_multicast;
}

bool
Lif::IsClassicForwarding()
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

