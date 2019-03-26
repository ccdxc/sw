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
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/devapi_impl.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"

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
    lif_impl *lif;
    pds_lif_spec_t spec = { 0 };

    spec.key = info->lif_id;
    spec.pinned_ifidx = info->pinned_uplink_port_num;
    lif = lif_impl::factory(&spec);
    if (unlikely(lif == NULL)) {
        return sdk::SDK_RET_OOM;
    }
    lif_impl_db()->insert(lif);
    lif->program_filters(info);
    return SDK_RET_OK;
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
devapi_impl::uplink_create(__UNUSED__ uint32_t uplink_ifidx,
                           pds_ifindex_t ifidx, bool is_oob) {
    sdk_ret_t ret;
    if_entry *intf;
    uint32_t logical_port, tm_port;

    intf = if_db()->find(&ifidx);
    if (intf == NULL) {
        return SDK_RET_INVALID_ARG;
    }
    logical_port = sdk::lib::catalog::ifindex_to_logical_port(ifidx);
    tm_port = g_pds_state.catalogue()->ifindex_to_tm_port(ifidx);
    // we use logical port as uplink hw lif identifier in the system
    ret = sdk::platform::capri::capri_tm_uplink_lif_set(tm_port, logical_port);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program uplink ifidx 0x%x's lif id "
                      "in TM, err %u", ret);
    }
    return ret;
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
