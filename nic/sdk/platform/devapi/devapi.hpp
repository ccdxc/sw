//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_HPP__
#define __DEVAPI_HPP__
#include "include/sdk/base.hpp"
#include "devapi_types.hpp"

namespace sdk {
namespace platform {

class devapi {
private:

public:
    static devapi *factory();
    static void destroy(devapi *dapi);

    devapi() {}
    virtual ~devapi(){}

    // Lif APIs
    virtual sdk_ret_t lif_create(lif_info_t *info) = 0;
    virtual sdk_ret_t lif_destroy(uint32_t lif_id) = 0;
    virtual sdk_ret_t lif_reset(uint32_t lif_id) = 0;
    virtual sdk_ret_t lif_add_mac(uint32_t lif_id, mac_t mac) = 0;
    virtual sdk_ret_t lif_del_mac(uint32_t lif_id, mac_t mac) = 0;
    virtual sdk_ret_t lif_add_vlan(uint32_t lif_id, vlan_t vlan) = 0;
    virtual sdk_ret_t lif_del_vlan(uint32_t lif_id, vlan_t vlan) = 0;
    virtual sdk_ret_t lif_add_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan) = 0;
    virtual sdk_ret_t lif_del_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan) = 0;
    virtual sdk_ret_t lif_upd_vlan_offload(uint32_t lif_id, bool vlan_strip,
                                           bool vlan_insert) = 0;
    virtual sdk_ret_t lif_upd_rx_mode(uint32_t lif_id, bool broadcast,
                                      bool all_multicast, bool promiscuous) = 0;
    virtual sdk_ret_t lif_upd_name(uint32_t lif_id, std::string name) = 0 ;
    virtual sdk_ret_t qos_get_txtc_cos(const std::string &group, uint32_t uplink_port,
                                       uint8_t *cos) = 0;
    virtual sdk_ret_t set_fwd_mode(fwd_mode_t fwd_mode) = 0;
    virtual sdk_ret_t uplink_create(uint32_t id, uint32_t port, bool is_oob) = 0;
    virtual sdk_ret_t uplink_destroy(uint32_t port) = 0;
};

} // namespace platform
} // namespace sdk

using sdk::platform::devapi;

#endif  // __DEVAPI_HPP__
