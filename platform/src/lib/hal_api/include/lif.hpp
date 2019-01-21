// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __ETH_LIF_HPP__
#define __ETH_LIF_HPP__

#include "hal_types.hpp"

#include "hal.hpp"
#include "enic.hpp"
#include "uplink.hpp"
#include "vrf.hpp"
#include "filter.hpp"

typedef std::map<uint32_t, Lif*> LifMap;

class Lif : public HalObject
{
public:

    static Lif *Factory(hal_lif_info_t *info);

    static void Destroy(Lif *lif);

    hal_irisc_ret_t AddMac(mac_t mac, bool re_add = false);
    hal_irisc_ret_t DelMac(mac_t mac, bool update_db = true);

    hal_irisc_ret_t AddVlan(vlan_t vlan);
    hal_irisc_ret_t DelVlan(vlan_t vlan, bool update_db = true);

    hal_irisc_ret_t AddMacVlan(mac_t mac, vlan_t vlan);
    hal_irisc_ret_t DelMacVlan(mac_t mac, vlan_t vlan, bool update_db = true);

    hal_irisc_ret_t UpdateVlanOffload(bool vlan_strip, bool vlan_insert);
    hal_irisc_ret_t UpdateReceiveMode(bool broadcast, bool all_multicast,
        bool promiscuous);

    hal_irisc_ret_t UpdateName(std::string name);

    void Reset();

    void RemoveMacFilters();
    void RemoveVlanFilters(bool skip_native_vlan = false);
    void RemoveMacVlanFilters();

    // Get APIs
    Lif *GetLif();
    Uplink *GetUplink();
    Enic *GetEnic();
    HalVrf *GetVrf();
    HalL2Segment *GetNativeL2Seg();
    uint32_t GetId();
    uint32_t GetHwLifId();
    bool GetIsPromiscuous();
    hal_lif_info_t *GetLifInfo();
    static Lif *GetInternalMgmtEthLif() { return Lif::internal_mgmt_ethlif; }

    // Set APIs
    void SetEnic(Enic *enic);
    void SetVrf(HalVrf *vrf) { vrf_ = vrf; }
    void SetNativeL2Seg(HalL2Segment *l2seg) { native_l2seg_ = l2seg; }
    static void SetInternalMgmtEthLif(Lif *lif) { Lif::internal_mgmt_ethlif = lif; }

    bool IsMnic();
    bool IsOOBMnic();
    bool IsInternalManagementMnic();
    bool IsInbandManagementMnic();
    bool IsHostManagement();
    bool IsInternalManagement();
    bool IsClassicForwarding();
    bool IsReceiveAllMulticast();
    void ProgramMCFilters();
    void DeProgramMCFilters();

private:
    Lif(hal_lif_info_t *info);
    ~Lif();

    static Lif *internal_mgmt_ethlif;

    // Config State (For Disruptive Upgrade):
    hal_lif_info_t info_;
    Enic *enic_;
    std::set<mac_t> mac_table_;
    std::set<vlan_t> vlan_table_;
    std::set<mac_vlan_t> mac_vlan_table_;

    // Oper State
    uint32_t id_;
    intf::LifSpec spec;

    // Valid only for internal mgmt mnic
    HalVrf *vrf_;
    HalL2Segment *native_l2seg_;

    // Oper State (For Expanding to EPs in Classic):
    std::map<mac_vlan_filter_t, MacVlanFilter*> mac_vlan_filter_table;

    void TriggerHalCreate();
    void TriggerHalDelete();
    void TriggerHalUpdate();
    void PopulateRequest(intf::LifRequestMsg &req_msg,
                         intf::LifSpec **req_ptr);
    void CreateMacVlanFilter(mac_t mac, vlan_t vlan);
    void DeleteMacVlanFilter(mac_t mac, vlan_t vlan);
    void CreateMacFilter(mac_t mac);
    void DeleteMacFilter(mac_t mac);
    void CreateVlanFilter(vlan_t vlan);
    void DeleteVlanFilter(vlan_t vlan);

    hal_irisc_ret_t UpdateVlanStripEn(bool vlan_strip_en);
    hal_irisc_ret_t UpdateVlanInsertEn(bool vlan_insert_en);
    hal_irisc_ret_t UpdateReceiveBroadcast(bool receive_broadcast);
    hal_irisc_ret_t UpdateReceiveAllMulticast(bool receive_all_multicast);
    hal_irisc_ret_t UpdateReceivePromiscuous(bool receive_promiscuous);

    // For upgrade. hw_lif_id -> Lif
    static LifMap ethlif_db;

    static constexpr uint32_t max_lifs = 1024;
};

#endif // __ETH_LIF_HPP__
