//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_HPP__
#define __DEVAPI_HPP__

#include <string>
#include "include/sdk/base.hpp"
#include "devapi_types.hpp"

namespace sdk {
namespace platform {

using std::string;

class devapi {
private:

public:
    static devapi *factory(void);
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
    // Qos APIs
    virtual sdk_ret_t qos_get_txtc_cos(const std::string &group, uint32_t uplink_port,
                                       uint8_t *cos) = 0;
    // Uplink APIs
    virtual sdk_ret_t uplink_create(uint32_t id, uint32_t port, bool is_oob) = 0;
    virtual sdk_ret_t uplink_destroy(uint32_t port) = 0;
    // Port APIs
    virtual sdk_ret_t port_get_status(uint32_t port_num,
                                      port_status_t *status /* o/p */) = 0;
    virtual sdk_ret_t port_get_config(uint32_t port_num,
                                      port_config_t *config /* o/p */) = 0;
    virtual sdk_ret_t port_set_config(uint32_t port_num,
                                      port_config_t *config) = 0;
    // Generic APIs
    virtual sdk_ret_t set_fwd_mode(fwd_mode_t fwd_mode) = 0;
    // Accel APIs
    virtual sdk_ret_t accel_rgroup_add(string name) = 0;
    virtual sdk_ret_t accel_rgroup_del(string name) = 0;
    virtual sdk_ret_t accel_rgroup_ring_add(string name,
                                            std::vector<std::pair<const std::string,uint32_t>>& ring_vec) = 0;
    virtual sdk_ret_t accel_rgroup_ring_del(string name,
                                            std::vector<std::pair<const std::string,uint32_t>>& ring_vec) = 0;
    virtual sdk_ret_t accel_rgroup_reset_set(string name, uint32_t sub_ring,
                                             bool reset_sense) = 0;
    virtual sdk_ret_t accel_rgroup_enable_set(string name, uint32_t sub_ring,
                                              bool enable_sense) = 0;
    virtual sdk_ret_t accel_rgroup_pndx_set(string name, uint32_t sub_ring,
                                            uint32_t val, bool conditional) = 0;
    virtual sdk_ret_t accel_rgroup_info_get(string name, uint32_t sub_ring,
                                            accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                                            void *user_ctx,
                                            uint32_t *ret_num_entries) = 0;
    virtual sdk_ret_t accel_rgroup_indices_get(string name, uint32_t sub_ring,
                                               accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                                               void *user_ctx,
                                               uint32_t *ret_num_entries) = 0;
    virtual sdk_ret_t accel_rgroup_metrics_get(string name, uint32_t sub_ring,
                                               accel_rgroup_rmetrics_rsp_cb_t rsp_cb_func,
                                               void *user_ctx,
                                               uint32_t *ret_num_entries) = 0;
    virtual sdk_ret_t crypto_key_index_upd(uint32_t key_index,
                                           crypto_key_type_t type,
                                           void *key, uint32_t key_size) = 0;
};

}    // namespace platform
}    // namespace sdk

using sdk::platform::devapi;

#endif    // __DEVAPI_HPP__
