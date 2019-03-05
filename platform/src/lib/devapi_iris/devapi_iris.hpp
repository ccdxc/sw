//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_IRIS_HPP__
#define __DEVAPI_IRIS_HPP__

#include "include/sdk/lock.hpp"
#include "include/sdk/base.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "devapi_iris_types.hpp"

namespace iris {

class devapi_iris : public devapi {
private:
    fwd_mode_t fwd_mode_;
    sdk_spinlock_t slock_;

private:
    sdk_ret_t init_();

public:
    static devapi_iris *factory();
    static void destroy(devapi *dapi);
    static void destroy(devapi_iris *dapi);

    devapi_iris() {
        SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }
    ~devapi_iris(){
        SDK_SPINLOCK_DESTROY(&slock_);
    }

    // Lif APIs
    sdk_ret_t lif_create(lif_info_t *info);
    sdk_ret_t lif_destroy(uint32_t lif_id);
    sdk_ret_t lif_reset(uint32_t lif_id);
    sdk_ret_t lif_add_mac(uint32_t lif_id, mac_t mac);
    sdk_ret_t lif_del_mac(uint32_t lif_id, mac_t mac);
    sdk_ret_t lif_add_vlan(uint32_t lif_id, vlan_t vlan);
    sdk_ret_t lif_del_vlan(uint32_t lif_id, vlan_t vlan);
    sdk_ret_t lif_add_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan);
    sdk_ret_t lif_del_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan);
    sdk_ret_t lif_upd_vlan_offload(uint32_t lif_id, bool vlan_strip, bool vlan_insert);
    sdk_ret_t lif_upd_rx_mode(uint32_t lif_id, bool broadcast,
                              bool all_multicast, bool promiscuous);
    sdk_ret_t lif_upd_name(uint32_t lif_id, std::string name);
    sdk_ret_t qos_get_txtc_cos(const std::string &group, uint32_t uplink_port,
                               uint8_t *cos);
    sdk_ret_t set_fwd_mode(fwd_mode_t fwd_mode);
    sdk_ret_t uplink_create(uint32_t id, uint32_t port, bool is_oob);
    sdk_ret_t uplink_destroy(uint32_t port);
};

} // namespace iris

using iris::devapi_iris;

#endif  // __DEVAPI_IRIS_HPP__
