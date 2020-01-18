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
    // uint32_t port_num_;

    // state
    devapi_uplink *oob_up_;
    std::map<uint32_t, channel_info_t*> channel_state_; // channel -> info
    uint32_t lif_base_;
    int tx_channel_;
#if 0
    devapi_ep *bmc_ep_;
    uint32_t swm_lif_id_;
    std::set<mac_t> mac_table_;
    std::set<vlan_t> vlan_table_;
    bool receive_broadcast_;
    bool receive_all_multicast_;
    bool receive_promiscuous_;
#endif

    // Single ton class
    static devapi_swm *swm_;
    static sdk::platform::devapi *dapi_;

private:
    sdk_ret_t init_();
    devapi_swm() {}
    ~devapi_swm();

    sdk_ret_t config_oob_uplink_vlan_(vlan_t vlan, channel_info_t *cinfo);
    sdk_ret_t unconfig_oob_uplink_vlan_(vlan_t vlan, channel_info_t *cinfo);

    sdk_ret_t initialize_lif_(channel_info_t *cinfo);
    sdk_ret_t uninitialize_lif_(channel_info_t *cinfo);
    sdk_ret_t add_mac_filters_(uint32_t channel);
    sdk_ret_t add_vlan_filters_(uint32_t channel);
#if 0
    sdk_ret_t config_oob_l2seg_swm_uplink_change_(devapi_uplink *old_swm_up,
                                                  devapi_uplink *new_swm_up);
#endif

public:
    static devapi_swm *factory();
    static void destroy(devapi_swm *swm);

    static sdk_ret_t swm_initialize(devapi *dapi);
    static sdk_ret_t swm_uninitialize(void);

    channel_info_t *lookup_channel_info_(uint32_t channel);
    sdk_ret_t swm_uninitialize_lif(void);
    void free_channel_info_();


    sdk_ret_t create_channel(uint32_t channel, uint32_t port_num);
    sdk_ret_t get_channels_info(std::set<channel_info_t *>* channels_info);
    sdk_ret_t add_mac(mac_t mac, uint32_t channel);
    sdk_ret_t del_mac(mac_t mac, uint32_t channel);
    sdk_ret_t add_vlan(vlan_t vlan, uint32_t channel);
    sdk_ret_t del_vlan(vlan_t vlan, uint32_t channel);
    sdk_ret_t upd_rx_bmode(bool broadcast, uint32_t channel);
    sdk_ret_t upd_rx_mmode(bool all_multicast, uint32_t channel);
    sdk_ret_t upd_rx_pmode(bool promiscuous, uint32_t channel);
    sdk_ret_t upd_bcast_filter(lif_bcast_filter_t bcast_filter, uint32_t channel);
    sdk_ret_t upd_mcast_filter(lif_mcast_filter_t mcast_filter, uint32_t channel);
    sdk_ret_t enable_tx(uint32_t channel);
    sdk_ret_t disable_tx(uint32_t channel);
    sdk_ret_t enable_rx(uint32_t channel);
    sdk_ret_t disable_rx(uint32_t channel);
    // sdk_ret_t upd_uplink(uint32_t port_num);

    // uint32_t port_num() { return port_num_; }
    // devapi_ep *bmc_ep() { return bmc_ep_; }
    // uint32_t swm_lif() { return swm_lif_id_; }
    // devapi_uplink *swm_up() { return swm_up_; }
    static devapi_swm *swm() { return swm_; }

    // void set_port_num(uint32_t port_num) { port_num_ = port_num; }
    void set_oob_up(devapi_uplink * oob_up) { oob_up_ = oob_up; }
    // void set_bmc_ep(devapi_ep *ep) { bmc_ep_ = ep; }
    void set_mac(mac_t mac, channel_info_t *cinfo);
    void set_vlan(vlan_t vlan, channel_info_t *cinfo);
    void unset_mac(mac_t mac, channel_info_t *cinfo);
    void unset_vlan(vlan_t vlan, channel_info_t *cinfo);
    void set_rx_bmode(bool broadcast, channel_info_t *cinfo);
    void set_rx_mmode(bool multicast, channel_info_t *cinfo);
    void set_rx_pmode(bool promiscuous, channel_info_t *cinfo);
};

}     // namespace iris

#endif
