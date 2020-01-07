//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SWM_HPP__
#define __SWM_HPP__

#include "nic/sdk/platform/devapi/devapi.hpp"
#include "devapi_types.hpp"
#include "devapi_object.hpp"

namespace iris {

#define INVALID_PORT_NUM 0xFFFFFFFF

class devapi_ep;
class devapi_uplink;
class devapi_swm : public devapi_object {
private:
    // config.
    uint32_t port_num_;

    // state
    devapi_uplink *oob_up_, *swm_up_;
    devapi_ep *bmc_ep_;
    uint32_t swm_lif_id_;
    std::set<mac_t> mac_table_;
    std::set<vlan_t> vlan_table_;
    bool receive_broadcast_;
    bool receive_all_multicast_;
    bool receive_promiscuous_;

    // Single ton class
    static devapi_swm *swm_;
    static sdk::platform::devapi *dapi_;

private:
    sdk_ret_t init_();
    devapi_swm() {}
    ~devapi_swm();

    sdk_ret_t config_oob_uplink_vlan_(vlan_t vlan);
    sdk_ret_t unconfig_oob_uplink_vlan_(vlan_t vlan);

    sdk_ret_t initialize_lif_(void);
    sdk_ret_t uninitialize_lif_(void);
    sdk_ret_t add_mac_filters_(void);
    sdk_ret_t add_vlan_filters_(void);
    sdk_ret_t config_oob_l2seg_swm_uplink_change_(devapi_uplink *old_swm_up,
                                                  devapi_uplink *new_swm_up);

public:
    static devapi_swm *factory();
    static void destroy(devapi_swm *swm);

    static sdk_ret_t swm_initialize(devapi *dapi);
    static sdk_ret_t swm_uninitialize(void);

    sdk_ret_t add_mac(mac_t mac);
    sdk_ret_t del_mac(mac_t mac);
    sdk_ret_t add_vlan(vlan_t vlan);
    sdk_ret_t del_vlan(vlan_t vlan);
    sdk_ret_t upd_rx_bmode(bool broadcast);
    sdk_ret_t upd_rx_mmode(bool all_multicast);
    sdk_ret_t upd_rx_pmode(bool promiscuous);
    sdk_ret_t upd_bcast_filter(lif_bcast_filter_t bcast_filter);
    sdk_ret_t upd_mcast_filter(lif_mcast_filter_t mcast_filter);
    sdk_ret_t upd_uplink(uint32_t port_num);

    uint32_t port_num() { return port_num_; }
    devapi_ep *bmc_ep() { return bmc_ep_; }
    uint32_t swm_lif() { return swm_lif_id_; }
    devapi_uplink *swm_up() { return swm_up_; }
    static devapi_swm *swm() { return swm_; }

    void set_port_num(uint32_t port_num) { port_num_ = port_num; }
    void set_oob_up(devapi_uplink * oob_up) { oob_up_ = oob_up; }
    void set_swm_up(devapi_uplink * swm_up) { swm_up_ = swm_up; }
    void set_bmc_ep(devapi_ep *ep) { bmc_ep_ = ep; }
    void set_mac(mac_t mac) { mac_table_.insert(mac); }
    void set_vlan(vlan_t vlan) { vlan_table_.insert(vlan); }
    void unset_mac(mac_t mac) { mac_table_.erase(mac); }
    void unset_vlan(vlan_t vlan) { vlan_table_.erase(vlan); }
    void set_rx_bmode(bool broadcast) { receive_broadcast_ = broadcast; }
    void set_rx_mmode(bool multicast) { receive_all_multicast_ = multicast; }
    void set_rx_pmode(bool promiscuous) { receive_promiscuous_ = promiscuous; }
};

}     // namespace iris

#endif
