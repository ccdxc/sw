//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// device api implementation for this pipeline
///
//----------------------------------------------------------------------------

#include <cmath>
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/lib/utils/port_utils.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/sdk/asic/pd/scheduler.hpp"
// TODO: replace this with asic pd
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/internal/metrics.hpp"
#include "nic/apollo/api/impl/devapi_impl.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"
#include "nic/apollo/api/internal/pds_if.hpp"
#include "gen/platform/mem_regions.hpp"

namespace api {
namespace impl {

extern lif_impl_state *lif_impl_db(void);

#define MAX_FILTERS_CLASSIC 32

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

static void
lif_spec_from_info (pds_lif_spec_t &spec, lif_info_t *info)
{
    spec.key = uuid_from_objid(LIF_IFINDEX(info->lif_id));
    spec.id = info->lif_id;
    strncpy(spec.name, info->name, sizeof(spec.name));
    spec.pinned_ifidx = info->pinned_uplink_port_num;
    spec.type = info->type;
    spec.vlan_strip_en = info->vlan_strip_en;
    MAC_ADDR_COPY(spec.mac, info->mac);
}

sdk_ret_t
devapi_impl::lif_create(lif_info_t *info) {
    lif_impl *lif;
    pds_lif_spec_t spec = { 0 };
    static mem_addr_t lif_stats_base_addr =
        g_pds_state.mempartition()->start_addr(MEM_REGION_LIF_STATS_NAME);
    static uint64_t block_size =
        g_pds_state.mempartition()->block_size(MEM_REGION_LIF_STATS_NAME);

    lif_spec_from_info(spec, info);
    lif = lif_impl::factory(&spec);
    if (unlikely(lif == NULL)) {
        return sdk::SDK_RET_OOM;
    }
    lif_impl_db()->insert(lif);
    PDS_TRACE_DEBUG("Inserted lif %u %s %u %s",
                    info->lif_id, info->name, info->type,
                    macaddr2str(info->mac));
    // register for lif metrics
    sdk::metrics::row_address(g_pds_state.hostif_metrics_handle(),
                      *(sdk::metrics::key_t *)spec.key.id,
                      (void *)(lif_stats_base_addr +
                                   (info->lif_id * block_size)));
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_destroy(uint32_t lif_id) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_init(lif_info_t *info) {
    pds_lif_spec_t spec = { 0 };
    lif_impl *lif;
    sdk_ret_t ret;

    lif = lif_impl_db()->find((pds_lif_id_t *)&info->lif_id);
    if (unlikely(lif == NULL)) {
        PDS_TRACE_ERR("Lif %u not found", info->lif_id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    lif_spec_from_info(spec, info);
    // program tx scheduler
    lif_program_tx_scheduler(info);
    ret = lif->create(&spec);
    if (ret == SDK_RET_OK) {
        PDS_TRACE_DEBUG("Created lif %s, id %u %s %u %s",
                        spec.key.str(), info->lif_id, info->name, info->type,
                        macaddr2str(info->mac));
        lif->set_tx_sched_info(info->tx_sched_table_offset,
                               info->tx_sched_num_table_entries);
    } else {
        PDS_TRACE_ERR("lif %s create failed!, id %u %s %u %s ret %u",
                        spec.key.str(), info->lif_id, info->name, info->type,
                        macaddr2str(info->mac),
                        ret);
    }

    return ret;
}

sdk_ret_t
devapi_impl::lif_reset(uint32_t lif_id) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_add_mac(uint32_t lif_id, mac_t mac) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_del_mac(uint32_t lif_id, mac_t mac) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_add_vlan(uint32_t lif_id, vlan_t vlan) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_del_vlan(uint32_t lif_id, vlan_t vlan) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_add_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_del_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_vlan_offload(uint32_t lif_id, bool vlan_strip,
                                  bool vlan_insert) {
    // TODO: handle vlan strip
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_rx_mode(uint32_t lif_id, bool broadcast,
                             bool all_multicast, bool promiscuous) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_rx_bmode(uint32_t lif_id, bool broadcast) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_rx_mmode(uint32_t lif_id, bool all_multicast) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_rx_pmode(uint32_t lif_id, bool promiscuous) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_name(uint32_t lif_id, string name) {
    lif_impl *lif;

    lif = lif_impl_db()->find((pds_lif_id_t *)&lif_id);
    if (unlikely(lif == NULL)) {
        PDS_TRACE_ERR("Lif %u not found", lif_id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    lif->set_name(name.c_str());
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_state(uint32_t lif_id, lif_state_t state) {
    lif_impl *lif;
    ::core::event_t event;

    // lookup lif and update the state
    lif = lif_impl_db()->find((pds_lif_id_t *)&lif_id);
    if (unlikely(lif == NULL)) {
        PDS_TRACE_ERR("Lif %u not found", lif_id);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    lif->set_state(state);

    // notify rest of the system
    memset(&event, 0, sizeof(event));
    event.lif.ifindex = LIF_IFINDEX(lif_id);
    event.lif.state = state;
    memcpy(event.lif.name, lif->name(), sizeof(event.lif.name));
    memcpy(event.lif.mac, lif->mac(), ETH_ADDR_LEN);
    sdk::ipc::broadcast(EVENT_ID_LIF_STATUS, &event, sizeof(event));
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_rdma_sniff(uint32_t lif_id, bool rdma_sniff) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_bcast_filter(uint32_t lif_id,
                                  lif_bcast_filter_t bcast_filter) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_mcast_filter(uint32_t lif_id,
                                  lif_mcast_filter_t mcast_filter) {
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_upd_rx_en(uint32_t lif_id, bool rx_en){
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_get_max_filters(uint32_t *ucast_filters,
                                 uint32_t *mcast_filters) {
    *ucast_filters = *mcast_filters = MAX_FILTERS_CLASSIC;
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::qos_class_get(uint8_t group, qos_class_info_t *info) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::qos_class_exist(uint8_t group) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}


sdk_ret_t
devapi_impl::qos_class_create(qos_class_info_t *info) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::qos_class_delete(uint8_t group, bool clear_stats) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::qos_clear_stats(uint32_t port_num, uint8_t qos_group_bitmap) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::qos_get_txtc_cos(const string &group, uint32_t uplink_port,
                              uint8_t *cos) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::qos_class_set_global_pause_type(uint8_t pause_type) {
    PDS_TRACE_WARN("Not implemented");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::uplink_create(__UNUSED__ uint32_t uplink_ifidx,
                           pds_ifindex_t ifidx, bool is_oob) {
    return SDK_RET_OK;
}

void
devapi_impl::port_get_config_(sdk::linkmgr::port_args_t *port_args,
                              void *ctxt) {
    port_config_t *config = (port_config_t *)ctxt;

    config->state =
        sdk::lib::port_admin_state_enum_to_uint(port_args->user_admin_state);
    config->speed = sdk::lib::port_speed_enum_to_mbps(port_args->port_speed);
    config->mtu = port_args->mtu;
    config->an_enable = port_args->auto_neg_cfg;
    config->fec_type = (uint8_t)port_args->user_fec_type;
    config->pause_type = (uint8_t)port_args->pause;
    if (port_args->tx_pause_enable == true) {
        config->pause_type |= PORT_CFG_PAUSE_F_TX;
    }
    if (port_args->rx_pause_enable == true) {
        config->pause_type |= PORT_CFG_PAUSE_F_RX;
    }
    config->loopback_mode = (uint8_t)port_args->loopback_mode;

    PDS_TRACE_VERBOSE(
            "if 0x%x, speed %u, mtu %u, state %u, an_enable %u, "
            "fec_type %u, pause_type %u, loopback_mode %u",
            sdk::lib::catalog::logical_port_to_ifindex(port_args->port_num),
            config->speed, config->mtu, config->state,
            config->an_enable, config->fec_type, config->pause_type,
            config->loopback_mode);
}

sdk_ret_t
devapi_impl::port_get_config(pds_ifindex_t ifidx, port_config_t *config) {
    return api::port_get(&ifidx, devapi_impl::port_get_config_, config);
}

void
devapi_impl::port_get_status_(sdk::linkmgr::port_args_t *port_args,
                              void *ctxt) {
    port_status_t *status = (port_status_t *)ctxt;

    status->id = port_args->port_num;
    status->status =
        sdk::lib::port_oper_state_enum_to_uint(port_args->oper_status);
    status->speed = sdk::lib::port_speed_enum_to_mbps(port_args->port_speed);
    status->fec_type = (uint8_t)port_args->fec_type;

    status->xcvr.state = port_args->xcvr_event_info.state;
    status->xcvr.phy = port_args->xcvr_event_info.cable_type;
    status->xcvr.pid = port_args->xcvr_event_info.pid;
    memcpy(status->xcvr.sprom, port_args->xcvr_event_info.xcvr_sprom,
           sizeof(status->xcvr.sprom));

    PDS_TRACE_VERBOSE(
            "if 0x%x, status %u, fec_type %u, xcvr state %u, pid %u",
            sdk::lib::catalog::logical_port_to_ifindex(port_args->port_num),
            status->status, status->fec_type, status->xcvr.state,
            status->xcvr.pid);
}

sdk_ret_t
devapi_impl::port_get_status(pds_ifindex_t ifidx, port_status_t *status) {
    return api::port_get(&ifidx, devapi_impl::port_get_status_, status);
}

sdk_ret_t
devapi_impl::populate_port_args_(sdk::linkmgr::port_args_t *port_args,
                                 port_config_t *config) {
    sdk_ret_t ret;
    port_speed_t port_speed;
    uint32_t num_lanes;

    port_args->user_admin_state = port_args->admin_state =
                    sdk::lib::port_admin_state_uint_to_enum(config->state);
    ret = sdk::lib::port_speed_mbps_to_enum(config->speed, &port_speed);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    port_args->port_speed = port_speed;
    port_args->mtu = config->mtu;
    port_args->auto_neg_cfg = port_args->auto_neg_enable = config->an_enable;
    port_args->fec_type = (port_fec_type_t)config->fec_type;
    port_args->pause = (port_pause_type_t)
                       (config->pause_type & PORT_CFG_PAUSE_TYPE_MASK);
    port_args->tx_pause_enable = config->pause_type & PORT_CFG_PAUSE_F_TX;
    port_args->rx_pause_enable = config->pause_type & PORT_CFG_PAUSE_F_RX;
    port_args->loopback_mode = (port_loopback_mode_t)config->loopback_mode;
    port_args->mac_stats_reset = config->reset_mac_stats;

    // TODO
    // For NRZ serdes, num_lanes can be assumed based on speed.
    // However, for PAM4, num_lanes needs to be decided based on how the
    // link must be brought up - with single lane or multiple lanes.
    // set the number of lanes based on speed if AN is disabled
    if (config->an_enable == 0) {
        ret = sdk::lib::port_num_lanes_speed(config->speed, &num_lanes);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        port_args->num_lanes = num_lanes;
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::port_set_config(pds_ifindex_t ifidx, port_config_t *config) {
    if_entry *intf;
    sdk_ret_t ret;
    port_args_t port_args = { 0 };

    intf = if_db()->find(&ifidx);
    if (intf == NULL)  {
        PDS_TRACE_ERR("Port 0x%x not found", ifidx);
        return SDK_RET_INVALID_ARG;
    }

    ret = sdk::linkmgr::port_get(intf->port_info(), &port_args);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Port 0x%x get failed", ifidx);
        return SDK_RET_ERR;
    }

    ret = populate_port_args_(&port_args, config);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // update port_args based on the xcvr state
    sdk::linkmgr::port_args_set_by_xcvr_state(&port_args);

    ret = sdk::linkmgr::port_update(intf->port_info(), &port_args);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update port for ifidx 0x%x", ifidx);
    }
    return ret;
}

sdk_ret_t
devapi_impl::swm_enable(void)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_disable(void)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_create_channel (uint32_t channel, uint32_t port_num, uint32_t lif_id)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_get_channels_info(std::set<channel_info_t *>* channels_info)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_add_mac(mac_t mac, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_del_mac(mac_t mac, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_add_vlan(vlan_t vlan, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_del_vlan(vlan_t vlan, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_upd_rx_bmode(bool broadcast, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_upd_rx_mmode(bool all_multicast, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_upd_rx_pmode(bool promiscuous, uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_upd_bcast_filter(lif_bcast_filter_t bcast_filter,
                                  uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_upd_mcast_filter(lif_mcast_filter_t mcast_filter,
                                  uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_enable_tx(uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_disable_tx(uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_enable_rx(uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_disable_rx(uint32_t channel)
{
    PDS_TRACE_VERBOSE("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::swm_upd_vlan_mode(bool enable, uint32_t mode, uint32_t channel)
{
    PDS_TRACE_DEBUG("Not provided");
    return SDK_RET_OK;
}

sdk_ret_t
devapi_impl::lif_program_tx_scheduler(lif_info_t *info) {
    sdk_ret_t ret;
    asicpd_scheduler_lif_params_t   apd_lif;

    apd_lif.lif_id = info->lif_id;
    apd_lif.hw_lif_id = info->lif_id;
    apd_lif.total_qcount = lif_get_qcount_(info);
    apd_lif.cos_bmp = lif_get_cos_bmp_(info);


    // allocate tx-scheduler resource, or use existing allocation
    apd_lif.tx_sched_table_offset = info->tx_sched_table_offset;
    apd_lif.tx_sched_num_table_entries = info->tx_sched_num_table_entries;
    ret = sdk::asic::pd::asicpd_tx_scheduler_map_alloc(&apd_lif);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to alloc tx sched. lif %lu, err %u",
                      info->lif_id, ret);
        return ret;
    }
    // save allocation in lif info
    info->tx_sched_table_offset = apd_lif.tx_sched_table_offset;
    info->tx_sched_num_table_entries = apd_lif.tx_sched_num_table_entries;

    // program tx scheduler and policer
    ret = sdk::asic::pd::asicpd_tx_scheduler_map_program(&apd_lif);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program tx sched. lif %lu, err %u",
                      info->lif_id, ret);
        return ret;
    }
    return ret;
}

uint32_t
devapi_impl::lif_get_qcount_(lif_info_t *info) {
    uint32_t qcount = 0;

     for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
         auto & qinfo = info->queue_info[i];
         if (qinfo.size < 1) {
             continue;
         }
         PDS_TRACE_DEBUG("Queue type_num %lu, entries %lu, purpose %lu",
                         qinfo.type_num, qinfo.entries, qinfo.purpose);
         qcount += pow(2, qinfo.entries);
     }
     PDS_TRACE_DEBUG("Lifid %u, qcount %lu", info->lif_id, qcount);
     return qcount;
}

uint16_t
devapi_impl::lif_get_cos_bmp_(lif_info_t *info) {
    uint16_t cos_bmp = 0;
    uint16_t cosA = DEVAPI_IMPL_ADMIN_COS;
    uint16_t cosB = 0;
    uint16_t cos_control = 2;

    cos_bmp =  ((1 << cosA) | (1 << cosB) | (1 << cos_control));
    return cos_bmp;
}

}    // namespace impl
}    // namespace api
