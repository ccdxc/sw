//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <spdlog/fmt/ostr.h>

#include "print.hpp"
#include "swm.hpp"
#include "devapi_mem.hpp"
#include "uplink.hpp"
#include "l2seg.hpp"
#include "vrf.hpp"
#include "enic.hpp"
#include "endpoint.hpp"
#include "multicast.hpp"
#include "devapi_iris.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "nic/include/globals.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/hal/plugins/cfg/ncsi/ncsi_ipc.hpp"

namespace iris {

devapi_swm *devapi_swm::swm_ = NULL;
devapi *devapi_swm::dapi_ = NULL;

devapi_swm *
devapi_swm::factory()
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_swm *v = NULL;

    mem = (devapi_swm *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_SWM,
                                      sizeof(devapi_swm));
    if (mem) {
        v = new (mem) devapi_swm();
        ret = v->init_();
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

end:
    if (ret != SDK_RET_OK) {
        v->~devapi_swm();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM, mem);
        v = NULL;
    }
    return v;
}

sdk_ret_t
devapi_swm::init_()
{
    lif_base_ = NICMGR_NCSI_LIF_MIN;
    tx_channel_ = -1;
    return SDK_RET_OK;
}

void
devapi_swm::free_channel_info_()
{
    channel_info_t *cinfo = NULL;

    for (auto it = channel_state_.cbegin(); it != channel_state_.cend(); it++) {
        cinfo = (channel_info_t *)(it->second);
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO, cinfo);
    }
}

void
devapi_swm::destroy(devapi_swm *v)
{
    v->free_channel_info_();
    v->~devapi_swm();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM, v);
}

devapi_swm::~devapi_swm()
{
    channel_state_.clear();
}

sdk_ret_t
devapi_swm::create_channel(uint32_t channel, uint32_t port_num)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo = NULL;
    std::map<uint32_t, channel_info_t*>::iterator it;
    void *mem = NULL;

    it = channel_state_.find(channel);
    if (it != channel_state_.end()) {
        NIC_LOG_ERR("Duplicate create of channel: {}", channel);
        return SDK_RET_ERR;
    }

    mem = (channel_info_t *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO,
                                          sizeof(channel_info_t));
    cinfo = new (mem) channel_info_t();
    cinfo->channel = channel;
    cinfo->port_num = port_num;
    cinfo->swm_lif_id = lif_base_++;
    cinfo->vlan_enable = 0;
    cinfo->vlan_mode = 0;

    channel_state_[channel] = cinfo;

    NIC_LOG_DEBUG("Creating channel {}, for port: {}",
                  channel, port_num);

    // Create lif, enic in hal
    ret = initialize_lif_(cinfo);    // Create lif, enic, l2seg etc.

    return ret;
}

channel_info_t *
devapi_swm::lookup_channel_info_(uint32_t channel)
{
    if (channel_state_.find(channel) != channel_state_.end()) {
        return channel_state_[channel];
    }
    return NULL;
}

sdk_ret_t
devapi_swm::get_channels_info(std::set<channel_info_t *>* channels_info)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *dst_cinfo = NULL, *src_cinfo = NULL;;
    void *mem = NULL;

    for (auto it = channel_state_.cbegin(); it != channel_state_.cend(); it++) {
        src_cinfo = (channel_info_t *)(it->second);
        NIC_LOG_DEBUG("channel: {}, rx: {}, tx: {}", src_cinfo->channel,
                      src_cinfo->rx_en, src_cinfo->tx_en);
        mem = (channel_info_t *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO,
                                              sizeof(channel_info_t));
        dst_cinfo = new (mem) channel_info_t();
        *dst_cinfo = *src_cinfo;
        channels_info->insert(dst_cinfo); 
    }

    return ret;
}

sdk_ret_t
devapi_swm::add_mac_filters_(uint32_t channel)
{
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    for (auto it = cinfo->mac_table.cbegin(); it != cinfo->mac_table.cend(); it++) {
        devapi_swm::add_mac(*it, channel);
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_swm::remove_mac_filters(channel_info_t *cinfo)
{
    mac_t mac;
    std::set<mac_t>::iterator tmp;

    NIC_LOG_DEBUG("channel-{}: Removing {} Mac Filters", 
                  cinfo->channel, cinfo->mac_table.size());
    for (auto it = cinfo->mac_table.begin(); it != cinfo->mac_table.end();) {
        mac = *it;
        tmp = it;
        ++tmp;
        del_mac(mac, cinfo->channel);
        it = tmp;
    }
    NIC_LOG_DEBUG("# of Mac Filters after delete: {}", 
                  cinfo->mac_table.size());

    return SDK_RET_OK;
}

sdk_ret_t
devapi_swm::add_vlan_filters_(uint32_t channel)
{
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    for (auto it = cinfo->vlan_table.cbegin(); it != cinfo->vlan_table.cend(); it++) {
        devapi_swm::add_vlan(*it, channel);
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_swm::remove_vlan_filters(channel_info_t *cinfo)
{
    vlan_t vlan;
    std::set<vlan_t>::iterator tmp;

    NIC_LOG_DEBUG("channel-{}: Removing {} Vlan Filters", 
                  cinfo->channel, cinfo->vlan_table.size());
    for (auto it = cinfo->vlan_table.begin(); it != cinfo->vlan_table.end();) {
        vlan = *it;
        tmp = it;
        ++tmp;
        del_vlan(vlan, cinfo->channel);
        it = tmp;
    }
    NIC_LOG_DEBUG("# of Vlan Filters after delete: {}", 
                  cinfo->vlan_table.size());
    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// - Initializes SWM lif
//   - Creates SWM Lif
//   - Creates SWM Enic
//   - Creates l2seg on swm uplink
//   - Adds swm uplink on the above l2seg
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_swm::initialize_lif_(channel_info_t *cinfo)
{
    sdk_ret_t ret         = SDK_RET_OK;
    lif_info_t info       = {0};

    // Create lif
    snprintf(info.name, sizeof(info.name), "swm_lif%d", ETH_IFINDEX_TO_PARENT_PORT(cinfo->port_num));
    info.lif_id                   = cinfo->swm_lif_id;
    info.type                     = sdk::platform::LIF_TYPE_SWM;
    info.pinned_uplink_port_num   = cinfo->port_num;
    info.is_management            = true;
    info.receive_broadcast        = true;
    info.receive_all_multicast    = true;
    info.receive_promiscuous      = false;
    info.enable_rdma              = false;
    info.max_vlan_filters         = 32;
    info.max_mac_filters          = 32;
    info.max_mac_vlan_filters     = 0;

    // Creates lif, enic and brings up untag
    ret = dapi_->lif_init(&info);

    return ret;
}

sdk_ret_t
devapi_swm::uninitialize_lif_(channel_info_t *cinfo)
{
    sdk_ret_t ret = SDK_RET_OK;

    // Destroy lif
    ret = dapi_->lif_destroy(cinfo->swm_lif_id);

    return ret;
}

#if 0
sdk_ret_t
devapi_swm::config_oob_l2seg_swm_uplink_change_(devapi_uplink *old_swm_up,
                                                devapi_uplink *new_swm_up)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_vrf *oob_vrf = NULL;
    devapi_l2seg *oob_l2seg = NULL;

    oob_vrf = oob_up_->get_vrf();
    for (auto it = vlan_table_.cbegin(); it != vlan_table_.cend(); it++) {
        oob_l2seg = devapi_l2seg::lookup(oob_vrf, *it);

        NIC_LOG_DEBUG("Configuring oob {}, l2seg: {}, vlan: {} Uplink change: "
                      "{} => {}", 
                      oob_up_->get_id(), oob_l2seg->get_id(), *it,
                      old_swm_up->get_id(), new_swm_up ? new_swm_up->get_id() : 0);
        oob_l2seg->set_undesignated_up(new_swm_up);
        if (old_swm_up != NULL) {
            if (new_swm_up) {
                oob_l2seg->del_uplink(old_swm_up, false);
            } else {
                // swm uplink is being removed. So there is no swm uplink,
                // means that this l2seg is not swm anymore
                oob_l2seg->set_single_wire_mgmt(false);
                oob_l2seg->del_uplink(old_swm_up);
            }
        }
        // Trigger l2seg upd to hal
        if (new_swm_up) {
            oob_l2seg->add_uplink(new_swm_up);
        }
    }
    return ret;
}
#endif

//-----------------------------------------------------------------------------
// Programs OOB with the vlan
// - Input properties for (oob, vlan)
// - Adding swm uplink to flood lists for BMC -> swm_uplink (MC, BC, PRMSC)
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_swm::config_oob_uplink_vlan_(vlan_t vlan, channel_info_t *cinfo)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_vrf *oob_vrf = NULL;
    devapi_l2seg *oob_l2seg = NULL;

    oob_vrf = oob_up_->get_vrf();
    oob_l2seg = devapi_l2seg::lookup(oob_vrf, vlan);
    if (!oob_l2seg) {
        oob_l2seg = devapi_l2seg::factory(oob_vrf, vlan);
        if (vlan == NATIVE_VLAN_ID) {
            oob_up_->set_native_l2seg(oob_l2seg);
            oob_up_->update_hal_native_l2seg(oob_l2seg->get_id());
        }
#if 0
        if (vlan == NATIVE_VLAN_ID) {
            oob_up->set_native_l2seg(oob_l2seg);
            oob_up->update_hal_native_l2seg(oob_l2seg->get_id());
        }
#endif
    }
    NIC_LOG_DEBUG("TX: Configuring oob: {} l2seg: {}, vlan: {}, channel: {}", 
                  oob_up_->get_id(), oob_l2seg->get_id(), vlan,
                  cinfo->channel);
    oob_l2seg->set_single_wire_mgmt(true);
    oob_l2seg->set_undesignated_up(devapi_uplink::get_uplink(cinfo->port_num));
    oob_l2seg->add_uplink(devapi_uplink::get_uplink(cinfo->port_num));// Trigger l2seg upd to hal
#if 0
    NIC_LOG_DEBUG("Adding swm uplink to mcast groups for oob: {} l2seg: {}",
                  oob_up->get_id(), 
                  oob_l2seg->get_id());
    devapi_mcast::trigger_l2seg_mcast(oob_l2seg);   // retrigger mcast. Adds undesignated uplink 
#endif

    return ret;
}

//-----------------------------------------------------------------------------
// Un-Programs OOB with the vlan
// - Input properties for (oob, vlan)
// - Adding swm uplink to flood lists for BMC -> swm_uplink (MC, BC, PRMSC)
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_swm::unconfig_oob_uplink_vlan_(vlan_t vlan, channel_info_t *cinfo)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_vrf *oob_vrf = NULL;
    devapi_l2seg *oob_l2seg = NULL;

    oob_vrf = oob_up_->get_vrf();
    oob_l2seg = devapi_l2seg::lookup(oob_vrf, vlan);
    if (!oob_l2seg) {
        NIC_LOG_ERR("Unable to find oob l2seg with vrf: {} vlan: {}",
                    oob_vrf->get_id(), vlan);
        ret = SDK_RET_ERR;
        goto end;
    }
    NIC_LOG_DEBUG("TX: Unconfiguring oob: {} l2seg: {}, vlan: {}, channel: {}", 
                  oob_up_->get_id(), oob_l2seg->get_id(), vlan,
                  cinfo->channel);
    if ((oob_l2seg->get_vlan() != NATIVE_VLAN_ID && oob_l2seg->num_enics() == 0) ||
        (oob_l2seg->get_vlan() == NATIVE_VLAN_ID && oob_up_->get_num_lifs() == 0)) {
        /*
         * For non-native:
         *  - num_enics show if l2seg is being referred by any enics.
         * For native:
         *  - num_enics can be 0 but l2seg may have been created during lif creation.
         *    We have to check lif count to check if no one is referring to this l2seg.
         */
        // Delete oob l2seg
        devapi_l2seg::destroy(oob_l2seg);
    } else {
        // Update oob l2seg
        oob_l2seg->set_single_wire_mgmt(false);
        oob_l2seg->set_undesignated_up(NULL);
        oob_l2seg->del_uplink(devapi_uplink::get_uplink(cinfo->port_num));      // Trigger l2seg upd to hal
#if 0
        NIC_LOG_DEBUG("Deleting oob from mcast groups for oob uplink's uplink: {} l2seg: {}",
                      oob_up->get_id(), 
                      oob_l2seg->get_id());
        devapi_mcast::trigger_l2seg_mcast(oob_l2seg);   // retrigger mcast. Adds undesignated uplink 
#endif
    }

end:
    return ret;
}

#if 0
sdk_ret_t
devapi_swm::upd_uplink(uint32_t port_num)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_uplink *swm_up = NULL; 

    if (swm_->port_num() == port_num) {
        NIC_LOG_ERR("No change in Uplink port num.");
        goto end;
    }

    if (port_num != INVALID_PORT_NUM) {
        swm_up = devapi_uplink::get_uplink(port_num);
        if (!swm_up) {
            NIC_LOG_ERR("Failed to find uplink for port: {}", port_num);
            return SDK_RET_ERR;
        }
    }

    if (swm_->port_num() != INVALID_PORT_NUM) {
        /*
         * Clean up existing config on uplink
         * - Remove lif
         *   - Will result in removal of all mac, vlan filters
         */
        ret = swm_->uninitialize_lif_();
    }

    // Initialize lif
    swm_->set_port_num(port_num);   // Set new port num

    if (port_num != INVALID_PORT_NUM) {
        ret = swm_->initialize_lif_();    // Create lif, enic, l2seg etc.

        // Create mac and vlan filters
        ret = swm_->add_mac_filters_();
        ret = swm_->add_vlan_filters_();
    }
    
    // Change oob l2seg to the remove old uplink and add new uplink
    ret = swm_->config_oob_l2seg_swm_uplink_change_(swm_->swm_up(), swm_up);

    // Setting new SWM uplink
    swm_->set_swm_up(swm_up);

end:
    return ret;
}
#endif

void 
devapi_swm::set_mac(mac_t mac, channel_info_t *cinfo) 
{
    cinfo->mac_table.insert(mac);
}

void
devapi_swm::set_vlan(vlan_t vlan, channel_info_t *cinfo)
{
    cinfo->vlan_table.insert(vlan);
}

void
devapi_swm::unset_mac(mac_t mac, channel_info_t *cinfo)
{
    cinfo->mac_table.erase(mac);
}

void
devapi_swm::unset_vlan(vlan_t vlan, channel_info_t *cinfo)
{
    cinfo->vlan_table.erase(vlan);
}

void 
devapi_swm::set_rx_bmode(bool broadcast, channel_info_t *cinfo)
{
    cinfo->receive_broadcast = broadcast;
}

void
devapi_swm::set_rx_mmode(bool multicast, channel_info_t *cinfo) 
{
    cinfo->receive_all_multicast = multicast;
}

void
devapi_swm::set_rx_pmode(bool promiscuous, channel_info_t *cinfo) 
{
    cinfo->receive_promiscuous = promiscuous;
}

sdk_ret_t 
devapi_swm::add_mac(mac_t mac, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    ret = dapi_->lif_add_mac(cinfo->swm_lif_id, mac);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to add mac {} to lif {}, channel: {}", 
                    macaddr2str(mac), 
                    cinfo->swm_lif_id, channel);
    }
    swm_->set_mac(mac, cinfo);
    return ret;
}

sdk_ret_t 
devapi_swm::del_mac(mac_t mac, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    ret = dapi_->lif_del_mac(cinfo->swm_lif_id, mac);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to del mac {} from lif {}, channel: {}", 
                    macaddr2str(mac), 
                    cinfo->swm_lif_id, channel);
    }
    swm_->unset_mac(mac, cinfo);
    return ret;
}

sdk_ret_t 
devapi_swm::add_vlan(vlan_t vlan, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    if (vlan == 0) {
        vlan = 8192;
    }
    NIC_LOG_DEBUG("Adding NCSI Vlan filter vlan: {}, channel: {}", vlan, channel);
    // Add vlan to lif. Sets up swm_uplink => oob_uplink
    cinfo = lookup_channel_info_(channel);
    if (cinfo) {
        ret = dapi_->lif_add_vlan(cinfo->swm_lif_id, vlan);
        if (ret != SDK_RET_OK) {
            NIC_LOG_ERR("Failed to add vlan {} to lif {}, channel: {}", vlan, 
                        cinfo->swm_lif_id, channel);
        }
        swm_->set_vlan(vlan, cinfo);
    } else {
        NIC_LOG_ERR("Unable to find the channel info. channel: {}", channel);
    }
    return ret;
}

sdk_ret_t 
devapi_swm::del_vlan(vlan_t vlan, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    if (vlan == 0) {
        vlan = 8192;
    }
    // Del vlan from lif. 
    cinfo = lookup_channel_info_(channel);
    ret = dapi_->lif_del_vlan(cinfo->swm_lif_id, vlan);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to del vlan {} from lif {}, channel: {}", vlan, 
                    cinfo->swm_lif_id, channel);
    }

#if 0
    // Bringup OOB uplink with the vlan. Sets up swm_uplink => oob_uplink
    ret = swm_->unconfig_oob_uplink_vlan_(vlan);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to del vlan {} from oob", vlan);
    }
#endif

    swm_->unset_vlan(vlan, cinfo);
    return ret;
}

sdk_ret_t 
devapi_swm::upd_rx_bmode(bool broadcast, uint32_t channel)
{
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    swm_->set_rx_bmode(broadcast, cinfo);
    return dapi_->lif_upd_rx_bmode(cinfo->swm_lif_id,
                                   broadcast);
}

sdk_ret_t 
devapi_swm::upd_rx_mmode(bool all_multicast, uint32_t channel)
{
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    swm_->set_rx_mmode(all_multicast, cinfo);
    return dapi_->lif_upd_rx_mmode(cinfo->swm_lif_id,
                                   all_multicast);
}

sdk_ret_t 
devapi_swm::upd_rx_pmode(bool promiscuous, uint32_t channel)
{
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    swm_->set_rx_pmode(promiscuous, cinfo);
    return dapi_->lif_upd_rx_pmode(cinfo->swm_lif_id,
                                   promiscuous);
}

sdk_ret_t
devapi_swm::upd_bcast_filter(lif_bcast_filter_t bcast_filter, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    ret = dapi_->lif_upd_bcast_filter(cinfo->swm_lif_id, bcast_filter);
    cinfo->bcast_filter = bcast_filter;
    return ret;
}

sdk_ret_t
devapi_swm::upd_mcast_filter(lif_mcast_filter_t mcast_filter, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    cinfo = lookup_channel_info_(channel);
    ret = dapi_->lif_upd_mcast_filter(cinfo->swm_lif_id, mcast_filter);
    cinfo->mcast_filter = mcast_filter;
    return ret;
}

#if 0
bool
devapi_swm::swm_changed_(uint32_t port_num, uint32_t vlan, mac_t mac)
{
    if ((swm_ == NULL) || (port_num != swm_->port_num() ||
                           vlan != swm_->vlan() || mac != swm_->mac())) {
        return true;
    }
    return false;
}
#endif

// ----------------------------------------------------------------------------
// SWM configuration
// ----------------------------------------------------------------------------
sdk_ret_t 
devapi_swm::swm_initialize(devapi *dapi)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_swm *swm = NULL;
    devapi_uplink *oob_up = NULL/*, *swm_up = NULL*/; 
    // devapi_vrf *oob_vrf = NULL, *swm_vrf = NULL;
    // devapi_l2seg *oob_l2seg = NULL, *swm_l2seg = NULL;
    // devapi_ep *bmc_ep = NULL;

    api_trace("swm initialize");

    dapi_ = dapi;

    swm = factory();
    if (!swm) {
        NIC_LOG_ERR("Failed to allocate swm. Fatal.");
        return SDK_RET_ERR;
    }
    swm_ = swm;

    // Find OOB Uplink
    oob_up = devapi_uplink::get_oob_uplink();
    if (!oob_up) {
        NIC_LOG_ERR("Failed to find oob uplink port");
        return SDK_RET_ERR;
    }

    swm->set_oob_up(oob_up);

#if 0
    if (vlan == 0) {
        vlan = NATIVE_VLAN_ID;
    }

    if (!swm_changed_(port_num, vlan, mac)) {
        NIC_LOG_DEBUG("No change in swm configuration: "
                      "port_num: {}, vlan: {}, mac: {}", port_num, vlan, mac);
        return SDK_RET_OK;
    }

    if (swm_ != NULL) {
        NIC_LOG_DEBUG("SWM configuration change from port: {}, vlan: {}, mac: {} "
                      "to port: {}, vlan: {}, mac: {}",
                      swm_->port_num(), swm_->vlan(), swm_->mac(),
                      port_num, vlan, mac);
        devapi_swm::swm_unconfigure();
    } else {
        NIC_LOG_DEBUG("First SWM configuration port: {}, vlan: {}, mac: {} ",
                      port_num, vlan, mac);
    }

    swm = factory(port_num, vlan, mac);
    if (!swm) {
        NIC_LOG_ERR("Failed to allocate swm. Fatal.");
        return SDK_RET_ERR;
    }
    swm_ = swm;

    // Find OOB Uplink & SWM Uplink
    swm_up = devapi_uplink::get_uplink(port_num);
    if (!swm_up) {
        NIC_LOG_ERR("Failed to find uplink for port: {}", port_num);
        return SDK_RET_ERR;
    }
    oob_up = devapi_uplink::get_oob_uplink();
    if (!oob_up) {
        NIC_LOG_ERR("Failed to find oob uplink port");
        return SDK_RET_ERR;
    }

    // Initialize swm lif, enic

    // On SWM Uplink, 
    // - L2Seg New / Existing
    //   - HAL Update with SWM set
    // - Retrigger Mcast groups with undesignated uplink
    swm_vrf = swm_up->get_vrf();
    swm_l2seg = devapi_l2seg::lookup(swm_vrf, vlan);
    if (!swm_l2seg) {
        swm_l2seg = devapi_l2seg::factory(swm_vrf, vlan);
        if (vlan == NATIVE_VLAN_ID) {
            swm_up->set_native_l2seg(swm_l2seg);
            swm_up->update_hal_native_l2seg(swm_l2seg->get_id());
        }
    }
    NIC_LOG_DEBUG("Setting fwm mode/add oob on swm uplink's uplink: {} l2seg: {}", 
                  swm_up->get_id(), 
                  swm_l2seg->get_id());
    swm_l2seg->set_single_wire_mgmt(true);
    swm_l2seg->add_uplink(oob_up);              // retrigger l2seg update
    swm_l2seg->set_undesignated_up(oob_up);     // for mcast triggers
    NIC_LOG_DEBUG("Adding oob to mcast groups for swm uplink's uplink: {} l2seg: {}",
                  swm_up->get_id(), 
                  swm_l2seg->get_id());
    devapi_mcast::trigger_l2seg_mcast(swm_l2seg);   // retrigger mcast. Adds undesignated uplink 
    // Create BMC's EP in uplink's swm l2seg
    bmc_ep = devapi_ep::factory(swm_l2seg, swm->mac(), NULL, oob_up);
    if (bmc_ep == NULL) {
        NIC_LOG_ERR("Failed to create BMC EP: vlan: {}, mac: {}, uplink: {}",
                    swm->vlan(), macaddr2str(swm->mac()), oob_up->get_id());
    }
    swm->set_bmc_ep(bmc_ep);


    // On OOB Uplink, 
    // - L2Seg New / Existing
    //   - HAL Update with SWM set
    // - Retrigger Mcast groups with undesignated uplink
    oob_vrf = oob_up->get_vrf();
    oob_l2seg = devapi_l2seg::lookup(oob_vrf, vlan);
    if (!oob_l2seg) {
        oob_l2seg = devapi_l2seg::factory(oob_vrf, vlan);
        if (vlan == NATIVE_VLAN_ID) {
            oob_up->set_native_l2seg(oob_l2seg);
            oob_up->update_hal_native_l2seg(oob_l2seg->get_id());
        }
    }
    NIC_LOG_DEBUG("Setting fwm mode/add swm's uplink on oob: {} l2seg: {}", 
                  oob_up->get_id(), 
                  oob_l2seg->get_id());
    oob_l2seg->set_single_wire_mgmt(true);
    oob_l2seg->set_undesignated_up(swm_up);
    oob_l2seg->add_uplink(swm_up);                  // retrigger l2mcast
    NIC_LOG_DEBUG("Adding swm uplink to mcast groups for oob: {} l2seg: {}",
                  oob_up->get_id(), 
                  oob_l2seg->get_id());
    devapi_mcast::trigger_l2seg_mcast(oob_l2seg);   // retrigger mcast. Adds undesignated uplink 
#endif

    return ret;
}

sdk_ret_t
devapi_swm::swm_uninitialize_lif(void)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo = NULL;

    for (auto it = channel_state_.cbegin(); it != channel_state_.cend(); it++) {
        cinfo = (channel_info_t *)(it->second);
        // Uninitialize lif
        ret = swm_->uninitialize_lif_(cinfo);
    }
    return ret;
}

sdk_ret_t
devapi_swm::swm_uninitialize(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    ret = swm_->swm_uninitialize_lif();

    // free up memory for swm
    devapi_swm::destroy(swm_);
#if 0
    swm_->uninitialize_lif_();

    // Remove swm_up from oob_l2segs
    ret = swm_->config_oob_l2seg_swm_uplink_change_(swm_->swm_up(), NULL);

    // free up memory for swm
    devapi_swm::destroy(swm_);
#endif


#if 0
    devapi_uplink *oob_up = NULL, *swm_up = NULL; 
    devapi_vrf *oob_vrf = NULL, *swm_vrf = NULL;
    devapi_l2seg *oob_l2seg = NULL, *swm_l2seg = NULL;

    if (!swm_) {
        NIC_LOG_DEBUG("Nothing to swm unconfigure.");
        return SDK_RET_OK;
    }

    api_trace("swm un-configure");
    NIC_LOG_DEBUG("SWM unconfiguration for port: {}, vlan: {}, mac: {} ",
                  swm_->port_num(), swm_->vlan(), swm_->mac());

    // Find OOB Uplink & SWM Uplink
    swm_up = devapi_uplink::get_uplink(swm_->port_num());
    if (!swm_up) {
        NIC_LOG_ERR("Failed to find uplink for port: {}", swm_->port_num());
        return SDK_RET_ERR;
    }
    oob_up = devapi_uplink::get_oob_uplink();
    if (!oob_up) {
        NIC_LOG_ERR("Failed to find oob uplink port");
        return SDK_RET_ERR;
    }
    
    swm_vrf = swm_up->get_vrf();
    swm_l2seg = devapi_l2seg::lookup(swm_vrf, swm_->vlan());
    if (!swm_l2seg) {
        NIC_LOG_ERR("Unable to find swm l2seg with vrf: {} vlan: {}",
                    swm_vrf->get_id(), swm_->vlan());
        return SDK_RET_ERR;
    }

    // Remove BMC EP
    devapi_ep::destroy(swm_->bmc_ep());
    swm_->set_bmc_ep(NULL);

    NIC_LOG_DEBUG("swm vlan: {}, uplink: {},  num_enics: {}, num_lifs: {}",
                  swm_l2seg->get_id(), swm_up->get_id(), 
                  swm_l2seg->num_enics(), swm_up->get_num_lifs());
    if ((swm_l2seg->get_vlan() != NATIVE_VLAN_ID && swm_l2seg->num_enics() == 0) ||
        (swm_l2seg->get_vlan() == NATIVE_VLAN_ID && swm_up->get_num_lifs() == 0)) {
        /*
         * For non-native:
         *  - num_enics show if l2seg is being referred by any enics.
         * For native:
         *  - num_enics can be 0 but l2seg may have been created during lif creation.
         *    We have to check lif count to check if no one is referring to this l2seg.
         */
        // Delete swm l2seg
        devapi_l2seg::destroy(swm_l2seg);
    } else {
        // Update swm l2seg
        swm_l2seg->set_single_wire_mgmt(false);
        swm_l2seg->set_undesignated_up(NULL);
        swm_l2seg->del_uplink(oob_up);                  // retrigger l2mcast
        NIC_LOG_DEBUG("Deleting oob from mcast groups for swm uplink's uplink: {} l2seg: {}",
                      swm_up->get_id(), 
                      swm_l2seg->get_id());
        devapi_mcast::trigger_l2seg_mcast(swm_l2seg);   // retrigger mcast. Adds undesignated uplink 
    }

    oob_vrf = oob_up->get_vrf();
    oob_l2seg = devapi_l2seg::lookup(oob_vrf, swm_->vlan());
    if (!oob_l2seg) {
        NIC_LOG_ERR("Unable to find oob l2seg with vrf: {} vlan: {}",
                    oob_vrf->get_id(), swm_->vlan());
        ret = SDK_RET_ERR;
        goto end;
    }
    if ((oob_l2seg->get_vlan() != NATIVE_VLAN_ID && oob_l2seg->num_enics() == 0) ||
        (oob_l2seg->get_vlan() == NATIVE_VLAN_ID && oob_up->get_num_lifs() == 0)) {
        /*
         * For non-native:
         *  - num_enics show if l2seg is being referred by any enics.
         * For native:
         *  - num_enics can be 0 but l2seg may have been created during lif creation.
         *    We have to check lif count to check if no one is referring to this l2seg.
         */
        // Delete oob l2seg
        devapi_l2seg::destroy(oob_l2seg);
    } else {
        // Update oob l2seg
        oob_l2seg->set_single_wire_mgmt(false);
        oob_l2seg->set_undesignated_up(NULL);
        oob_l2seg->del_uplink(oob_up);                  // retrigger l2mcast
        NIC_LOG_DEBUG("Deleting oob from mcast groups for oob uplink's uplink: {} l2seg: {}",
                      oob_up->get_id(), 
                      oob_l2seg->get_id());
        devapi_mcast::trigger_l2seg_mcast(oob_l2seg);   // retrigger mcast. Adds undesignated uplink 
    }

    swm_ = NULL;
end:
#endif
    return ret;
}

sdk_ret_t 
devapi_swm::enable_tx(uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo = NULL;

    if (tx_channel_ != -1) {
        NIC_LOG_WARN("Channel {} already is tx enabled. Force enabling tx on channel: {}.",
                    tx_channel_, channel);
        disable_tx(tx_channel_);
    }

    if (tx_channel_ == (int)channel) {
        NIC_LOG_WARN("channel {} already tx-enabled. noop", channel);
        goto end;
    }

    NIC_LOG_DEBUG("TX enable for channel: {}", channel);

    cinfo = lookup_channel_info_(channel);
    for (auto it = cinfo->vlan_table.cbegin(); it != cinfo->vlan_table.cend(); it++) {
        ret = swm_->config_oob_uplink_vlan_(*it, cinfo);
    }
    ret = swm_->config_oob_uplink_vlan_(NATIVE_VLAN_ID, cinfo);

    tx_channel_ = channel;
    cinfo->tx_en = true;

end:
    return ret;
}

sdk_ret_t 
devapi_swm::disable_tx(uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo = NULL;

    if (tx_channel_ == -1) {
        NIC_LOG_DEBUG("Tx channel {} already disabled. noop", channel);
        goto end;
    }

    if (tx_channel_ != (int)channel) {
        NIC_LOG_ERR("Current TX Channel {}. Trying to disable: {}",
                    tx_channel_, channel);
    }

    NIC_LOG_DEBUG("TX disable for channel: {}", channel);

    cinfo = lookup_channel_info_(channel);
    for (auto it = cinfo->vlan_table.cbegin(); it != cinfo->vlan_table.cend(); it++) {
        ret = swm_->unconfig_oob_uplink_vlan_(*it, cinfo);
    }
    ret = swm_->unconfig_oob_uplink_vlan_(NATIVE_VLAN_ID, cinfo);

    tx_channel_ = -1;
    cinfo->tx_en = false;

end:
    return ret;
}

sdk_ret_t
devapi_swm::enable_rx (uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    NIC_LOG_DEBUG("RX enable for channel: {}", channel);
    cinfo = lookup_channel_info_(channel);
    if (cinfo->rx_en) {
        NIC_LOG_DEBUG("Channel {} already is enabled. noop", channel);
        goto end;
    }
    ret = dapi_->lif_upd_rx_en(cinfo->swm_lif_id, true);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to enable rx on lif: {}, channel: {}", 
                    cinfo->swm_lif_id, channel);
    }
    cinfo->rx_en = true;

end:
    return ret;
}

sdk_ret_t
devapi_swm::disable_rx (uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    NIC_LOG_DEBUG("RX disable for channel: {}", channel);
    cinfo = lookup_channel_info_(channel);
    if (!cinfo->rx_en) {
        NIC_LOG_DEBUG("Channel {} already is disabled. noop", channel);
        goto end;
    }
    ret = dapi_->lif_upd_rx_en(cinfo->swm_lif_id, false);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to disable rx on lif: {}, channel: {}", 
                    cinfo->swm_lif_id, channel);
    }
    cinfo->rx_en = false;

end:
    return ret;
}

sdk_ret_t
devapi_swm::reset_channel (uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    NIC_LOG_DEBUG("Reset for channel: {}", channel);
    cinfo = lookup_channel_info_(channel);

    // Remove mac filters
    ret = remove_mac_filters(cinfo);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to remove mac filters for channel: {}. err: {}", 
                    channel, ret);
    }

    // Remove vlan filters
    ret = remove_vlan_filters(cinfo);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to remove vlan filters for channel: {}. err: {}", 
                    channel, ret);
    }

    // Disable RX
    ret = disable_rx(channel);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to disable rx for channel: {}. err: {}", 
                    channel, ret);
    }

    // Disable TX
    ret = disable_tx(channel);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to disable tx for channel: {}. err: {}", 
                    channel, ret);
    }

    // Disable vlan mode
    ret = upd_vlan_mode(false, 0, channel);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to disable vlan mode for channel: {}. err: {}", 
                    channel, ret);
    }

end:
    return ret;
}

sdk_ret_t
devapi_swm::upd_vlan_mode(bool enable, uint32_t vlan_mode, uint32_t channel)
{
    sdk_ret_t ret = SDK_RET_OK;
    channel_info_t *cinfo;

    NIC_LOG_DEBUG("channel: {}, vlan_enable: {}, mode: {}", 
                  channel, enable, vlan_mode);
    cinfo = lookup_channel_info_(channel);
    if (cinfo->vlan_enable == enable && cinfo->vlan_mode == vlan_mode) {
        NIC_LOG_DEBUG("Channel {}. No change in vlan mode.", channel);
        goto end;
    }

    if (cinfo->vlan_enable != enable) {
        NIC_LOG_DEBUG("channel: {} vlan_enable change {} -> {}",
                      channel, cinfo->vlan_enable, enable);
        if (enable) {
            switch(vlan_mode) {
             case hal::NCSI_VLAN_MODE1_VLAN:
                 // Remove native vlan
                 ret = dapi_->lif_del_vlan(cinfo->swm_lif_id, 0);
                 if (ret != SDK_RET_OK) {
                     NIC_LOG_ERR("Failed to del vlan {} from lif {}, channel: {}", 0, 
                                 cinfo->swm_lif_id, channel);
                 }
                 break;
             case hal::NCSI_VLAN_MODE2_VLAN_NATIVE:
             case hal::NCSI_VLAN_MODE0_RSVD:
             case hal::NCSI_VLAN_MODE3_ANY_VLAN_NATIVE:
             case hal::NCSI_VLAN_MODE4_RSVD:
             default:
                 NIC_LOG_DEBUG("Nothing to do for vlan mode change.");
            }
        } else {
            // Disable vlan, Enable native if it moved from NCSI_VLAN_MODE1_VLAN
            if (cinfo->vlan_mode == hal::NCSI_VLAN_MODE1_VLAN) {
                 // Enable native vlan
                 ret = dapi_->lif_add_vlan(cinfo->swm_lif_id, 0);
                 if (ret != SDK_RET_OK) {
                     NIC_LOG_ERR("Failed to add vlan {} to lif {}, channel: {}", 0, 
                                 cinfo->swm_lif_id, channel);
                 }
            }
        }
    } else {
        // No enable change
        if (enable) {
            // Mode change
            switch(vlan_mode) {
            case hal::NCSI_VLAN_MODE1_VLAN:
                // Any mode => Only vlan
                // Remove native vlan
                ret = dapi_->lif_del_vlan(cinfo->swm_lif_id, 0);
                if (ret != SDK_RET_OK) {
                    NIC_LOG_ERR("Failed to del vlan {} from lif {}, channel: {}", 0, 
                                cinfo->swm_lif_id, channel);
                }
                break;
            case hal::NCSI_VLAN_MODE2_VLAN_NATIVE:
                if (cinfo->vlan_mode == hal::NCSI_VLAN_MODE1_VLAN) {
                    // Enable native vlan
                    ret = dapi_->lif_add_vlan(cinfo->swm_lif_id, 0);
                    if (ret != SDK_RET_OK) {
                        NIC_LOG_ERR("Failed to add vlan {} to lif {}, channel: {}", 0, 
                                    cinfo->swm_lif_id, channel);
                    }
                }
                break;
            case hal::NCSI_VLAN_MODE0_RSVD:
                if (cinfo->vlan_mode == hal::NCSI_VLAN_MODE1_VLAN) {
                    // Enable native vlan
                    ret = dapi_->lif_add_vlan(cinfo->swm_lif_id, 0);
                    if (ret != SDK_RET_OK) {
                        NIC_LOG_ERR("Failed to add vlan {} to lif {}, channel: {}", 0, 
                                    cinfo->swm_lif_id, channel);
                    }
                }
                break;
            case hal::NCSI_VLAN_MODE3_ANY_VLAN_NATIVE:
            case hal::NCSI_VLAN_MODE4_RSVD:
            default:
                NIC_LOG_DEBUG("Not supported vlan mode change");
            }
        }
    }
    cinfo->vlan_enable = enable;
    cinfo->vlan_mode = vlan_mode;

end:
    return ret;
}

} // namespace iris
