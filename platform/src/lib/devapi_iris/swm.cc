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
#include "platform/src/lib/nicmgr/include/logger.hpp"

namespace iris {

devapi_swm *devapi_swm::swm_ = NULL;

devapi_swm *
devapi_swm::factory(uint32_t port_num, uint32_t vlan, mac_t mac)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_swm *v = NULL;

    mem = (devapi_swm *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_SWM,
                                      sizeof(devapi_swm));
    if (mem) {
        v = new (mem) devapi_swm();
        ret = v->init_(port_num, vlan, mac);
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
devapi_swm::init_(uint32_t port_num, uint32_t vlan, mac_t mac)
{
    port_num_ = port_num;
    vlan_ = vlan;
    mac_ = mac;

    return SDK_RET_OK;
}

void
devapi_swm::destroy(devapi_swm *v)
{
    v->~devapi_swm();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM, v);
}

bool
devapi_swm::swm_changed_(uint32_t port_num, uint32_t vlan, mac_t mac)
{
    if ((swm_ == NULL) || (port_num != swm_->port_num() ||
                           vlan != swm_->vlan() || mac != swm_->mac())) {
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// SWM configuration
// ----------------------------------------------------------------------------
sdk_ret_t 
devapi_swm::swm_configure(uint32_t port_num, uint32_t vlan, mac_t mac)
{
    devapi_swm *swm = NULL;
    devapi_uplink *oob_up = NULL, *swm_up = NULL; 
    devapi_vrf *oob_vrf = NULL, *swm_vrf = NULL;
    devapi_l2seg *oob_l2seg = NULL, *swm_l2seg = NULL;
    devapi_ep *bmc_ep = NULL;

    api_trace("swm configure");
    NIC_LOG_DEBUG("SWM configuration for port: {}, vlan: {}, mac: {} ",
                  port_num, vlan, mac);

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

    return SDK_RET_OK;
}

sdk_ret_t
devapi_swm::swm_unconfigure(void)
{
    sdk_ret_t ret = SDK_RET_OK;
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
    return ret;
}

} // namespace iris
