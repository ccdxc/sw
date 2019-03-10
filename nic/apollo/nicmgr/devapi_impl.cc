//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// device api implementation for this pipeline
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/nicmgr/devapi_impl.hpp"

namespace api {
namespace impl {

devapi *
devapi_impl::factory(void) {
    devapi_impl    *impl;

    impl = (devapi_impl *)SDK_CALLOC(SDK_MEM_ALLOC_DEVAPI_IMPL,
                                     sizeof(devapi_impl));
    if (!impl) {
        return NULL;
    }
    new (impl) devapi_impl();
    return impl;
}

void
devapi_impl::destroy(devapi *impl) {
    (dynamic_cast<devapi_impl *>(impl))->~devapi_impl();
    SDK_FREE(SDK_MEM_ALLOC_DEVAPI_IMPL, impl);
}

sdk_ret_t
devapi_impl::lif_create(lif_info_t *info) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_destroy(uint32_t lif_id) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_reset(uint32_t lif_id) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_add_mac(uint32_t lif_id, mac_t mac) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_del_mac(uint32_t lif_id, mac_t mac) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_add_vlan(uint32_t lif_id, vlan_t vlan) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_del_vlan(uint32_t lif_id, vlan_t vlan) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_add_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_del_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_upd_vlan_offload(uint32_t lif_id, bool vlan_strip,
                                  bool vlan_insert) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_upd_rx_mode(uint32_t lif_id, bool broadcast,
                             bool all_multicast, bool promiscuous) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::lif_upd_name(uint32_t lif_id, string name) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::qos_get_txtc_cos(const string &group, uint32_t uplink_port,
                              uint8_t *cos) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::port_get_status(uint32_t port_num, port_status_t *status) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::port_get_config(uint32_t port_num, port_config_t *config) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
devapi_impl::port_set_config(uint32_t port_num, port_config_t *config) {
    return SDK_RET_INVALID_OP;
}


}    // namespace impl
}    // namespace api
