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
    sdk_ret_t reset_channel(uint32_t channel);
    sdk_ret_t upd_vlan_mode(bool enable, uint32_t vlan_mode, uint32_t channel);
    sdk_ret_t remove_mac_filters(channel_info_t *cinfo);
    sdk_ret_t remove_vlan_filters(channel_info_t *cinfo);

    static devapi_swm *swm() { return swm_; }

    void set_oob_up(devapi_uplink * oob_up) { oob_up_ = oob_up; }
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
