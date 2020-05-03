//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __LIF_HPP__
#define __LIF_HPP__

#include "devapi_object.hpp"
#include "devapi_iris_types.hpp"

namespace iris {

#define MAX_FILTERS 64
#if 0
#define MAX_FILTERS_CLASSIC 32
#define MAX_FILTERS_SMART   4096
#endif

class devapi_enic;
class devapi_l2seg;
class devapi_vrf;
class devapi_filter;
class devapi_uplink;
class devapi_iris;
class devapi_lif : public devapi_object {
private:
    static devapi_lif *internal_mgmt_ethlif;

    // Config State (For Disruptive Upgrade):
    lif_info_t info_;
    devapi_enic *enic_;
    std::set<mac_t> mac_table_;
    std::set<vlan_t> vlan_table_;
    std::set<mac_vlan_t> mac_vlan_table_;

    // Oper State
    intf::LifSpec spec_;
    intf::LifState state_;

    // Valid only for internal mgmt mnic
    devapi_vrf *vrf_;
    devapi_l2seg *native_l2seg_;

    // Oper State (For Expanding to EPs in Classic):
    std::map<mac_vlan_filter_t, devapi_filter*> mac_vlan_filter_table_;
    // For upgrade. hw_lif_id -> Lif
    static std::map<uint32_t, devapi_lif*> lif_db_;
    static constexpr uint32_t max_lifs = 1024;

private:
    sdk_ret_t init_(lif_info_t *info);
    devapi_lif() {}
    ~devapi_lif();
    sdk_ret_t lif_halcreate(void);
    sdk_ret_t lif_haldelete(void);
    sdk_ret_t lif_halupdate(void);
    void populate_req(intf::LifRequestMsg &req_msg,
                      intf::LifSpec **req_ptr);
    sdk_ret_t create_macvlan_filter(mac_t mac, vlan_t vlan);
    sdk_ret_t delete_macvlan_filter(mac_t mac, vlan_t vlan);
    sdk_ret_t create_mac_filter(mac_t mac);
    sdk_ret_t delete_mac_filter(mac_t mac);
    sdk_ret_t create_vlan_filter(vlan_t vlan);
    sdk_ret_t delete_vlan_filter(vlan_t vlan);

    sdk_ret_t update_vlanstrip(bool vlan_strip_en);
    sdk_ret_t update_vlanins(bool vlan_insert_en);

public:
    static devapi_lif *factory(lif_info_t *info, devapi_iris *dapi);
    static void destroy(devapi_lif *lif, devapi_iris *dapi);

    static sdk_ret_t set_micro_seg_en(bool en);

    sdk_ret_t add_mac(mac_t mac, bool re_add = false);
    sdk_ret_t del_mac(mac_t mac, bool update_db = true, bool add_failure = false);

    sdk_ret_t add_vlan(vlan_t vlan, bool replay_from_db);
    sdk_ret_t del_vlan(vlan_t vlan, bool update_db = true, bool add_failure = false);

    sdk_ret_t add_macvlan(mac_t mac, vlan_t vlan);
    sdk_ret_t del_macvlan(mac_t mac, vlan_t vlan, bool update_db = true,
                          bool add_failure = false);

    sdk_ret_t upd_vlanoff(bool vlan_strip, bool vlan_insert);
    sdk_ret_t upd_rxmode(bool broadcast, bool all_multicast,
                         bool promiscuous);
    sdk_ret_t update_recbcast(bool receive_broadcast);
    sdk_ret_t update_recallmc(bool receive_all_multicast);
    sdk_ret_t update_recprom(bool receive_promiscuous);
    sdk_ret_t update_bcast_filters(lif_bcast_filter_t bcast_filter);
    sdk_ret_t update_mcast_filters(lif_mcast_filter_t mcast_filter);
    sdk_ret_t upd_rx_en(bool rx_en);
    sdk_ret_t upd_state(intf::LifState state);

    sdk_ret_t upd_name(std::string name);
    sdk_ret_t reset(void);
    static sdk_ret_t get_max_filters(uint32_t *ucast_filters, uint32_t *mcast_filters);
    sdk_ret_t upd_state(lif_state_t state);
    sdk_ret_t upd_rdma_sniff(bool rdma_sniff);

    void remove_macfilters(bool skip_native_mac, bool update_d);
    void remove_vlanfilters(bool skip_native_vlan,
                            bool update_db);
    void remove_macvlanfilters(void);
    void add_vlanfilters(bool skip_native_vlan = false);
    void add_macfilters(bool skip_native_mac = false);

    // Get APIs
    devapi_lif *get_lif(void);
    devapi_uplink *get_uplink(void);
    devapi_enic *get_enic(void);
    devapi_vrf *get_vrf(void);
    devapi_l2seg *get_nativel2seg(void);
    uint32_t get_id(void);
    bool get_isprom(void);
    lif_info_t *get_lifinfo(void);
    static devapi_lif *lookup(uint32_t lif_id);
    static devapi_lif *get_intmgmt_lif(void) { return devapi_lif::internal_mgmt_ethlif; }
    bool is_swm(void);

    // Set APIs
    void set_enic(devapi_enic *enic);
    void set_vrf(devapi_vrf *vrf) { vrf_ = vrf; }
    void set_nativel2seg(devapi_l2seg *l2seg) { native_l2seg_ = l2seg; }
    static void set_intmgmt_lif(devapi_lif *lif) { devapi_lif::internal_mgmt_ethlif = lif; }

    bool is_mnic(void);
    bool is_oobmnic(void);
    bool is_intmgmtmnic(void);
    bool is_inbmgmtmnic(void);
    bool is_host(void);
    bool is_hostmgmt(void);
    bool is_intmgmt(void);
    bool is_classicfwd(vlan_t vlan);
    bool is_recallmc(void);
    void program_mcfilters(void);
    void deprogram_mcfilters(void);
};

}    // namespace iris

using iris::devapi_lif;

#endif // __LIF_HPP__
