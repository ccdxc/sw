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
    port_num_ = INVALID_PORT_NUM;
    return SDK_RET_OK;
}

void
devapi_swm::destroy(devapi_swm *v)
{
    v->~devapi_swm();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM, v);
}

devapi_swm::~devapi_swm()
{
    mac_table_.clear();
    vlan_table_.clear();
}

sdk_ret_t
devapi_swm::add_mac_filters_()
{
    for (auto it = mac_table_.cbegin(); it != mac_table_.cend(); it++) {
        devapi_swm::add_mac(*it);
    }
}

sdk_ret_t
devapi_swm::add_vlan_filters_()
{
    for (auto it = vlan_table_.cbegin(); it != vlan_table_.cend(); it++) {
        devapi_swm::add_vlan(*it);
    }
}

//-----------------------------------------------------------------------------
// - Initializes SWM lif
//   - Creates SWM Lif
//   - Creates SWM Enic
//   - Creates l2seg on swm uplink
//   - Adds swm uplink on the above l2seg
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_swm::initialize_lif_()
{
    sdk_ret_t ret         = SDK_RET_OK;
    lif_info_t info       = {0};

    swm_lif_id_ = oob_up_->get_lif_id();

    // Create lif
    info.lif_id                   = swm_lif_id_;
    strcpy(info.name, "swm_lif");
    info.type                     = sdk::platform::LIF_TYPE_SWM;
    info.pinned_uplink_port_num   = port_num_;
    info.is_management            = true;
    info.receive_broadcast        = true;
    info.receive_all_multicast    = true;
    info.receive_promiscuous      = false;
    info.enable_rdma              = false;
    info.max_vlan_filters         = 32;
    info.max_mac_filters          = 32;
    info.max_mac_vlan_filters     = 0;

    // Creates lif, enic and brings up untag
    ret = dapi_->lif_create(&info);

    return ret;
}

sdk_ret_t
devapi_swm::uninitialize_lif_()
{
    sdk_ret_t ret = SDK_RET_OK;

    // Destroy lif
    ret = dapi_->lif_destroy(swm_lif_id_);

    return ret;
}

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

//-----------------------------------------------------------------------------
// Programs OOB with the vlan
// - Input properties for (oob, vlan)
// - Adding swm uplink to flood lists for BMC -> swm_uplink (MC, BC, PRMSC)
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_swm::config_oob_uplink_vlan_(vlan_t vlan)
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
    NIC_LOG_DEBUG("Configuring oob: {} l2seg: {}, vlan: {}", 
                  oob_up_->get_id(), oob_l2seg->get_id(), vlan);
    oob_l2seg->set_single_wire_mgmt(true);
    oob_l2seg->set_undesignated_up(swm_up_);
    oob_l2seg->add_uplink(swm_up_);          // Trigger l2seg upd to hal
#if 0
    NIC_LOG_DEBUG("Adding swm uplink to mcast groups for oob: {} l2seg: {}",
                  oob_up->get_id(), 
                  oob_l2seg->get_id());
    devapi_mcast::trigger_l2seg_mcast(oob_l2seg);   // retrigger mcast. Adds undesignated uplink 
#endif
}

//-----------------------------------------------------------------------------
// Un-Programs OOB with the vlan
// - Input properties for (oob, vlan)
// - Adding swm uplink to flood lists for BMC -> swm_uplink (MC, BC, PRMSC)
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_swm::unconfig_oob_uplink_vlan_(vlan_t vlan)
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
    NIC_LOG_DEBUG("Unconfiguring oob: {} l2seg: {}, vlan: {}", 
                  oob_up_->get_id(), oob_l2seg->get_id(), vlan);
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
        oob_l2seg->del_uplink(swm_up_);      // Trigger l2seg upd to hal
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
        ret - swm_->add_vlan_filters_();
    }
    
    // Change oob l2seg to the remove old uplink and add new uplink
    ret = swm_->config_oob_l2seg_swm_uplink_change_(swm_->swm_up(), swm_up);

    // Setting new SWM uplink
    swm_->set_swm_up(swm_up);

end:
    return ret;
}

sdk_ret_t 
devapi_swm::add_mac(mac_t mac)
{
    sdk_ret_t ret = SDK_RET_OK;
    ret = dapi_->lif_add_mac(swm_->swm_lif(), mac);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to add mac {} to lif {}", macaddr2str(mac), 
                    swm_lif_id_);
    }
    swm_->set_mac(mac);
    return ret;
}

sdk_ret_t 
devapi_swm::del_mac(mac_t mac)
{
    sdk_ret_t ret = SDK_RET_OK;
    ret = dapi_->lif_del_mac(swm_->swm_lif(), mac);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to del mac {} from lif {}", macaddr2str(mac), 
                    swm_lif_id_);
    }
    swm_->unset_mac(mac);
    return ret;
}

sdk_ret_t 
devapi_swm::add_vlan(vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (vlan == 0) {
        vlan = 8192;
    }
    // Add vlan to lif. Sets up swm_uplink => oob_uplink
    ret = dapi_->lif_add_vlan(swm_->swm_lif(), vlan);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to add vlan {} to lif {}", vlan, swm_lif_id_);
    }

    // Bringup OOB uplink with the vlan. Sets up swm_uplink => oob_uplink
    ret = swm_->config_oob_uplink_vlan_(vlan);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to add vlan {} to oob", vlan);
    }

    swm_->set_vlan(vlan);
    return ret;
}

sdk_ret_t 
devapi_swm::del_vlan(vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;

    // Del vlan from lif. 
    ret = dapi_->lif_del_vlan(swm_->swm_lif(), vlan);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to del vlan {} from lif {}", vlan, swm_lif_id_);
    }

    // Bringup OOB uplink with the vlan. Sets up swm_uplink => oob_uplink
    ret = swm_->unconfig_oob_uplink_vlan_(vlan);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to del vlan {} from oob", vlan);
    }

    swm_->unset_vlan(vlan);
    return ret;
}

sdk_ret_t 
devapi_swm::upd_rx_bmode(bool broadcast)
{
    swm_->set_rx_bmode(broadcast);
    return dapi_->lif_upd_rx_bmode(swm_->swm_lif(), 
                                                       broadcast);
}

sdk_ret_t 
devapi_swm::upd_rx_mmode(bool all_multicast)
{
    swm_->set_rx_mmode(all_multicast);
    return dapi_->lif_upd_rx_mmode(swm_->swm_lif(), 
                                                       all_multicast);
}

sdk_ret_t 
devapi_swm::upd_rx_pmode(bool promiscuous)
{
    swm_->set_rx_pmode(promiscuous);
    return dapi_->lif_upd_rx_pmode(swm_->swm_lif(), 
                                                       promiscuous);
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
    devapi_uplink *oob_up = NULL, *swm_up = NULL; 
    devapi_vrf *oob_vrf = NULL, *swm_vrf = NULL;
    devapi_l2seg *oob_l2seg = NULL, *swm_l2seg = NULL;
    devapi_ep *bmc_ep = NULL;

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

    return SDK_RET_OK;
}

sdk_ret_t
devapi_swm::swm_uninitialize(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    // Uninitialize lif
    swm_->uninitialize_lif_();

    // Remove swm_up from oob_l2segs
    ret = swm_->config_oob_l2seg_swm_uplink_change_(swm_->swm_up(), NULL);

    // free up memory for swm
    devapi_swm::destroy(swm_);


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

} // namespace iris
