//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_HPP__
#define __DEVAPI_HPP__

#include <string>
#include <vector>
#include <set>
#include "include/sdk/base.hpp"
#include "platform/devapi/devapi_types.hpp"

namespace sdk {
namespace platform {

using std::string;
using std::vector;
using std::set;

class devapi {
public:
    static devapi *factory(void);
    static void destroy(devapi *dapi);

    // lif APIs
    virtual sdk_ret_t lif_create(lif_info_t *info) = 0;
    virtual sdk_ret_t lif_destroy(uint32_t lif_id) = 0;
    virtual sdk_ret_t lif_init(lif_info_t *info) = 0;
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
    virtual sdk_ret_t lif_upd_rx_bmode(uint32_t lif_id, bool broadcast) = 0;
    virtual sdk_ret_t lif_upd_rx_mmode(uint32_t lif_id, bool all_multicast) = 0;
    virtual sdk_ret_t lif_upd_rx_pmode(uint32_t lif_id, bool promiscuous) = 0;
    virtual sdk_ret_t lif_upd_name(uint32_t lif_id, string name) = 0;
    virtual sdk_ret_t lif_get_max_filters(uint32_t *ucast_filters, uint32_t *mcast_filters) = 0;
    virtual sdk_ret_t lif_upd_state(uint32_t lif_id, lif_state_t state) = 0;
    virtual sdk_ret_t lif_upd_rdma_sniff(uint32_t lif_id, bool rdma_sniff) = 0;
    virtual sdk_ret_t lif_upd_bcast_filter(uint32_t lif_id, 
                                           lif_bcast_filter_t bcast_filter) = 0; 
    virtual sdk_ret_t lif_upd_mcast_filter(uint32_t lif_id, 
                                           lif_mcast_filter_t mcast_filter) = 0; 
    virtual sdk_ret_t lif_upd_rx_en(uint32_t lif_id, bool rx_en) = 0;

    // qos APIs
    virtual sdk_ret_t qos_class_get(uint8_t group, qos_class_info_t *info) = 0;
    virtual sdk_ret_t qos_class_exist(uint8_t group) = 0;
    virtual sdk_ret_t qos_class_create(qos_class_info_t *info) = 0;
    virtual sdk_ret_t qos_class_delete(uint8_t group) = 0;
    virtual sdk_ret_t qos_get_txtc_cos(const string &group, uint32_t uplink_port,
                                       uint8_t *cos) = 0;
    virtual sdk_ret_t qos_class_set_global_pause_type(uint8_t pause_type) = 0;

    // uplink APIs
    virtual sdk_ret_t uplink_create(uint32_t id, uint32_t port, bool is_oob) = 0;
    virtual sdk_ret_t uplink_destroy(uint32_t port) = 0;

    // port APIs
    virtual sdk_ret_t port_get_status(uint32_t port_num,
                                      port_status_t *status /* o/p */) = 0;
    virtual sdk_ret_t port_get_config(uint32_t port_num,
                                      port_config_t *config /* o/p */) = 0;
    virtual sdk_ret_t port_set_config(uint32_t port_num,
                                      port_config_t *config) = 0;

    // generic APIs
    virtual sdk_ret_t set_fwd_mode(sdk::lib::dev_forwarding_mode_t fwd_mode) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t set_micro_seg_en(bool en) {
        return SDK_RET_INVALID_OP;
    }
    static bool is_hal_up(void);

    // single wire management APIs
    virtual sdk_ret_t swm_enable(void) = 0;
    virtual sdk_ret_t swm_disable(void) = 0;
    virtual sdk_ret_t swm_create_channel(uint32_t channel, uint32_t port_num) = 0;
    virtual sdk_ret_t swm_get_channels_info(std::set<channel_info_t *>* channels_info) = 0;
    virtual sdk_ret_t swm_add_mac(mac_t mac, uint32_t channel) = 0;
    virtual sdk_ret_t swm_del_mac(mac_t mac, uint32_t channel) = 0;
    virtual sdk_ret_t swm_add_vlan(vlan_t vlan, uint32_t channel) = 0;
    virtual sdk_ret_t swm_del_vlan(vlan_t vlan, uint32_t channel) = 0;
    virtual sdk_ret_t swm_upd_rx_bmode(bool broadcast, uint32_t channel) = 0;
    virtual sdk_ret_t swm_upd_rx_mmode(bool all_multicast, uint32_t channel) = 0;
    virtual sdk_ret_t swm_upd_rx_pmode(bool promiscuous, uint32_t channel) = 0;
    virtual sdk_ret_t swm_upd_bcast_filter(lif_bcast_filter_t bcast_filter, uint32_t channel) = 0;
    virtual sdk_ret_t swm_upd_mcast_filter(lif_mcast_filter_t mcast_filter, uint32_t channel) = 0;
    virtual sdk_ret_t swm_enable_tx(uint32_t channel) = 0;
    virtual sdk_ret_t swm_disable_tx(uint32_t channel) = 0;
    virtual sdk_ret_t swm_enable_rx(uint32_t channel) = 0;
    virtual sdk_ret_t swm_disable_rx(uint32_t channel) = 0;
    virtual sdk_ret_t swm_upd_vlan_mode(bool enable, uint32_t mode, uint32_t channel) = 0;

    // accel APIs
    virtual sdk_ret_t accel_rgroup_add(string name,
                                       uint64_t metrics_mem_addr,
                                       uint32_t metrics_mem_size) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_del(string name) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_ring_add(string name,
                                            vector<std::pair<const string, uint32_t>>& ring_vec) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_ring_del(string name,
                                            vector<std::pair<const string,uint32_t>>& ring_vec) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_reset_set(string name, uint32_t sub_ring,
                                             bool reset_sense) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_enable_set(string name, uint32_t sub_ring,
                                              bool enable_sense) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_pndx_set(string name, uint32_t sub_ring,
                                            uint32_t val, bool conditional) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_info_get(string name, uint32_t sub_ring,
                                            accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                                            void *user_ctx,
                                            uint32_t *ret_num_entries) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_indices_get(string name, uint32_t sub_ring,
                                               accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                                               void *user_ctx,
                                               uint32_t *ret_num_entries) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_metrics_get(string name, uint32_t sub_ring,
                                               accel_rgroup_rmetrics_rsp_cb_t rsp_cb_func,
                                               void *user_ctx,
                                               uint32_t *ret_num_entries) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t accel_rgroup_misc_get(string name, uint32_t sub_ring,
                                            accel_rgroup_rmisc_rsp_cb_t rsp_cb_func,
                                            void *user_ctx,
                                            uint32_t *ret_num_entries) {
        return SDK_RET_INVALID_OP;
    }
    virtual sdk_ret_t crypto_key_index_upd(uint32_t key_index,
                                           crypto_key_type_t type,
                                           void *key, uint32_t key_size) {
        return SDK_RET_INVALID_OP;
    }

protected:
    devapi() {}
    virtual ~devapi() {}

};

}    // namespace platform
}    // namespace sdk

using sdk::platform::devapi;

#endif    // __DEVAPI_HPP__
