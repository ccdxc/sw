
#ifndef __NIC_HPP__
#define __NIC_HPP__

#include "hal_types.hpp"

// #include "lif.hpp"
#include "hal.hpp"
#include "enic.hpp"
#include "uplink.hpp"
#include "vrf.hpp"
#include "filter.hpp"

typedef std::map<uint32_t, EthLif*> EthLifMap;

class EthLif : public HalObject
{
public:

    static EthLif *Factory(hal_lif_info_t *info);

    static void Destroy(EthLif *eth_lif);

    hal_irisc_ret_t AddMac(mac_t mac);
    hal_irisc_ret_t DelMac(mac_t mac);

    hal_irisc_ret_t AddVlan(vlan_t vlan);
    hal_irisc_ret_t DelVlan(vlan_t vlan);

    hal_irisc_ret_t AddMacVlan(mac_t mac, vlan_t vlan);
    hal_irisc_ret_t DelMacVlan(mac_t mac, vlan_t vlan);

    hal_irisc_ret_t UpdateVlanStripEn(bool vlan_stip_en);
    hal_irisc_ret_t UpdateVlanInsertEn(bool vlan_insert_en);
    hal_irisc_ret_t UpdateReceiveBroadcast(bool receive_broadcast);
    hal_irisc_ret_t UpdateReceiveAllMulticast(bool receive_all_multicast);
    hal_irisc_ret_t UpdateReceivePromiscuous(bool receive_promiscuous);

    hal_irisc_ret_t remove_mac_filters();
    hal_irisc_ret_t remove_vlan_filters();
    hal_irisc_ret_t remove_mac_vlan_filters();

    Lif *GetLif();
    Uplink *GetUplink();
    Enic *GetEnic();
    void SetEnic(Enic *enic);
    uint32_t GetHwLifId();
    bool GetIsPromiscuous();
    hal_lif_info_t *GetLifInfo();
    bool IsOOBMnic();

private:
    EthLif(hal_lif_info_t *info);
    ~EthLif();

    static sdk::lib::indexer *filter_allocator;
    static constexpr uint64_t max_macaddrs_per_lif = 64;
    static constexpr uint64_t max_vlans_per_lif = 8;
    static constexpr uint64_t max_filters_per_lif = 4096;

    // Config State (For Disruptive Upgrade):
#if 0
    bool is_mgmt_lif;
    bool is_promiscuous;
    uint32_t hw_lif_id;
#endif
    hal_lif_info_t info_;
    Enic *enic_;
    std::set<mac_t> mac_table_;
    std::set<vlan_t> vlan_table_;
    std::set<mac_vlan_t> mac_vlan_table_;

    // Oper State
    Lif *lif_;
    // Oper State (For Expanding to EPs in Classic):
#if 0
    std::map<mac_t, uint32_t> mac_table;
    std::map<vlan_t, uint32_t> vlan_table;
    std::map<mac_vlan_t, uint32_t> mac_vlan_table;
#endif
    std::map<mac_vlan_filter_t, MacVlanFilter*> mac_vlan_filter_table;

    void CreateMacVlanFilter(mac_t mac, vlan_t vlan);
    void DeleteMacVlanFilter(mac_t mac, vlan_t vlan);
    void CreateMacFilter(mac_t mac);
    void DeleteMacFilter(mac_t mac);
    void CreateVlanFilter(vlan_t vlan);
    void DeleteVlanFilter(vlan_t vlan);


    // For upgrade. hw_lif_id -> EthLif
    static EthLifMap ethlif_db;
};

#endif
