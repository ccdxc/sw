//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "nic/hal/hal_devapi.hpp"
#include "devapi_iris.hpp"
#include "devapi_mem.hpp"
#include "lif.hpp"
#include "uplink.hpp"
#include "qos.hpp"
#include "port.hpp"
#include "accel.hpp"
#include "swm.hpp"
#include "utils.hpp"

namespace iris {

//-----------------------------------------------------------------------------
// Factory method to instantiate the class
//-----------------------------------------------------------------------------
devapi *
devapi_iris::factory(void)
{
    sdk_ret_t ret      = SDK_RET_OK;
    void *mem          = NULL;
    devapi_iris *diris = NULL;

    mem = (devapi_iris *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_DEVAPI_IRIS,
                                       sizeof(devapi_iris));
    if (mem) {
        diris = new (mem) devapi_iris();
        ret = diris->init_();
        if (ret != SDK_RET_OK) {
            diris->~devapi_iris();
            DEVAPI_FREE(DEVAPI_MEM_ALLOC_DEVAPI_IRIS, mem);
            diris = NULL;
        }
    }
    return diris;
}

//-----------------------------------------------------------------------------
// Initializing fields in the class
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_iris::init_(void)
{
    mirco_seg_en_ = false;
    num_int_mgmt_mnics_ = 0;
    // Initialize grpc
    hal_grpc::factory();
    // Populate hal_grpc handler in object
    devapi_object::populate_hal_grpc();
    return SDK_RET_OK;
}

void
devapi_iris::destroy(devapi *dapi)
{
    NIC_LOG_DEBUG("Destroy devapi_iris");
    devapi_iris *dapi_iris = dynamic_cast<devapi_iris*>(dapi);
    dapi_iris->~devapi_iris();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_DEVAPI_IRIS, dapi_iris);
}

sdk_ret_t
devapi_iris::lif_create(lif_info_t *info)
{
    // XXX IRIS does not create the lif until lif_init.
    return SDK_RET_OK;
}

sdk_ret_t
devapi_iris::lif_init(lif_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;
    lif = devapi_lif::factory(info, this);
    if (!lif) {
        NIC_LOG_ERR("Failed to create lif. id: {}", info->lif_id);
        ret = SDK_RET_ERR;
    }
    return ret;
}

sdk_ret_t
devapi_iris::lif_destroy(uint32_t lif_id)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (lif) {
        devapi_lif::destroy(lif, this);
    } else {
        // XXX IRIS will not find a lif that was not initialized.
        //
        // We can't determine with the lif_id alone whether the lif has been
        // initialized.  Therefore, it will be normal to see the following
        // error logged for iris, if the lif was not initialized.
        //
        NIC_LOG_ERR("Failed to destroy lif. id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
    }
    return ret;
}

sdk_ret_t
devapi_iris::lif_reset(uint32_t lif_id)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to reset lif. id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->reset();
end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_add_mac(uint32_t lif_id, mac_t mac)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to add mac. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->add_mac(mac);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_del_mac(uint32_t lif_id, mac_t mac)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to del mac. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->del_mac(mac);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_add_vlan(uint32_t lif_id, vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    NIC_LOG_DEBUG("lif_add_vlan: Adding lif: {}, vlan: {}",
                  lif_id, vlan);
    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to add vlan. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->add_vlan(vlan);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_del_vlan(uint32_t lif_id, vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to del vlan. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->del_vlan(vlan);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_add_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to add vlan. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->add_macvlan(mac, vlan);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_del_macvlan(uint32_t lif_id, mac_t mac, vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to del vlan. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->del_macvlan(mac, vlan);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_vlan_offload(uint32_t lif_id, bool vlan_strip,
                                  bool vlan_insert)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update vlan offl. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->upd_vlanoff(vlan_strip, vlan_insert);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_rx_mode(uint32_t lif_id, bool broadcast,
                             bool all_multicast, bool promiscuous)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx mode. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->upd_rxmode(broadcast, all_multicast, promiscuous);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_rx_bmode(uint32_t lif_id, bool broadcast)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx mode. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->update_recbcast(broadcast);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_rx_mmode(uint32_t lif_id, bool all_multicast)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx mode. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->update_recallmc(all_multicast);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_rx_pmode(uint32_t lif_id, bool promiscuous)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx mode. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->update_recprom(promiscuous);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_bcast_filter(uint32_t lif_id, 
                                  lif_bcast_filter_t bcast_filter)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx mode. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->update_bcast_filters(bcast_filter);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_mcast_filter(uint32_t lif_id, 
                                  lif_mcast_filter_t mcast_filter)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx mode. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->update_mcast_filters(mcast_filter);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_rx_en(uint32_t lif_id, bool rx_en)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update rx enable. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->upd_rx_en(rx_en);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_name(uint32_t lif_id, std::string name)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update name. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->upd_name(name);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_get_max_filters(uint32_t *ucast_filters,
                                 uint32_t *mcast_filters)
{
    return devapi_lif::get_max_filters(ucast_filters,
                                       mcast_filters);
}

sdk_ret_t
devapi_iris::qos_class_get(uint8_t group, qos_class_info_t *info)
{
    return devapi_qos::qos_class_get(group, info);
}

sdk_ret_t
devapi_iris::qos_class_exist(uint8_t group)
{
    return devapi_qos::qos_class_exist(group);
}

sdk_ret_t
devapi_iris::qos_class_create(qos_class_info_t *info)
{
    return devapi_qos::qos_class_create(info);
}

sdk_ret_t
devapi_iris::qos_class_delete(uint8_t group)
{
    return devapi_qos::qos_class_delete(group);
}

sdk_ret_t
devapi_iris::qos_get_txtc_cos(const std::string &group, uint32_t uplink_port,
                              uint8_t *cos)
{
    *cos = devapi_qos::get_txtc_cos(group, uplink_port);
    return SDK_RET_OK;
}

sdk_ret_t
devapi_iris::qos_class_set_global_pause_type(uint8_t pause_type)
{
    return devapi_qos::qos_class_set_global_pause_type(pause_type);
}

sdk_ret_t
devapi_iris::set_fwd_mode(sdk::lib::dev_forwarding_mode_t fwd_mode)
{
#if 0
    fwd_mode_ = fwd_mode;
    hal_grpc::get_hal_grpc()->set_fwd_mode(fwd_mode);
#endif
    return SDK_RET_OK;
}

sdk_ret_t 
devapi_iris::micro_seg_halupdate_(bool en)
{
    grpc::Status         status;
    MicroSegRequestMsg req_msg;
    MicroSegResponseMsg rsp_msg;
    MicroSegSpec *req;
    MicroSegResponse rsp;

    req = req_msg.add_request();
    req->set_micro_seg_mode(en ? sys::MICRO_SEG_ENABLE : 
                            sys::MICRO_SEG_DISABLE);
    req->set_status(sys::MICRO_SEG_STATUS_SUCCESS);

    // VERIFY_HAL();
    status = hal_grpc::get_hal_grpc()->micro_seg_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Updated micro_seg mode: Mode: {}:{}, Status: {}:{}",
                          req->micro_seg_mode(), 
                          sys::MicroSegMode_Name(req->micro_seg_mode()), 
                          req->status(), sys::MicroSegStatus_Name(req->status()));
        } else {
            NIC_LOG_DEBUG("Failed to update micro_seg mode: Mode: {}:{}, "
                          "Status: {}:{}. err: {}",
                          req->micro_seg_mode(), MicroSegMode_Name(req->micro_seg_mode()), 
                          req->status(), MicroSegStatus_Name(req->status()),
                          rsp.api_status());
        }
    } else {
        NIC_LOG_DEBUG("Failed to update micro_seg mode: Mode: {}:{}, "
                      "Status: {}:{}. err: {}:{}",
                      req->micro_seg_mode(), MicroSegMode_Name(req->micro_seg_mode()), 
                      req->status(), MicroSegStatus_Name(req->status()),
                      status.error_code(), status.error_message());
    }

    return SDK_RET_OK;
}

sdk_ret_t
devapi_iris::set_micro_seg_en(bool en)
{
    if (mirco_seg_en_ == en) {
        NIC_LOG_DEBUG("No change in micro-seg: {}", en);
        goto end;
    }

    // Remove Vlan filters on all Host LIFs
    devapi_lif::set_micro_seg_en(en);

    // Send micro seg update status to HAL
    micro_seg_halupdate_(en);

    mirco_seg_en_ = en;
    if (hal_grpc::get_hal_grpc()) {
        hal_grpc::get_hal_grpc()->set_micro_seg_en(en);
    }
end:
    return SDK_RET_OK;
}

bool
devapi_iris::get_micro_seg_cfg_en(void)
{
    grpc::Status        status;
    SysSpecGetRequest   req;
    SysSpecGetResponse  rsp;
    bool                micro_seg_cfg_en = false;

    if (hal_grpc::get_hal_grpc()) {
        status = hal_grpc::get_hal_grpc()->sys_spec_get(req, rsp);
        if (status.ok()) {
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_LOG_DEBUG("Got micro_seg_cfg_en: {}",
                              sys::ForwardMode_Name(rsp.spec().fwd_mode()));
                micro_seg_cfg_en = (rsp.spec().fwd_mode() == sys::FWD_MODE_MICROSEG);
            } else {
                NIC_LOG_DEBUG("Failed to get micro seg cfg en. err: {}:{}", 
                              status.error_code(), status.error_message());
            }
        }
    }

    return micro_seg_cfg_en;
}

sdk_ret_t
devapi_iris::uplink_create(uint32_t id, uint32_t port, bool is_oob)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_uplink *up = NULL;
    up = devapi_uplink::factory(id, port, is_oob);
    if (!up) {
        NIC_LOG_ERR("Failed to create uplink. id: {}", id);
        ret = SDK_RET_ERR;
    }
    // Create vrf
    up->create_vrf();
    return ret;
}

sdk_ret_t
devapi_iris::uplink_destroy(uint32_t port)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_uplink *up = NULL;

    up = devapi_uplink::get_uplink(port);
    if (!up) {
        NIC_LOG_ERR("Failed to destroy uplink. port: {}", port);
        ret = SDK_RET_ERR;
        goto end;
    }
    // Remove vrf
    up->delete_vrf();
    // Destroy uplink
    devapi_uplink::destroy(up);

end:
    return ret;
}

sdk_ret_t
devapi_iris::port_get_status(uint32_t port_num,
                             port_status_t *status)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_port *port = NULL;

    port = devapi_port::find_or_create(port_num);
    if (!port) {
        NIC_LOG_ERR("Failed to find/create port: {}", port_num);
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = port->port_hal_get_status(status);

end:
    return ret;
}

sdk_ret_t
devapi_iris::port_get_config(uint32_t port_num,
                             port_config_t *config)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_port *port = NULL;

    port = devapi_port::find_or_create(port_num);
    if (!port) {
        NIC_LOG_ERR("Failed to find/create port: {}", port_num);
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = port->port_hal_get_config(config);

end:
    return ret;
}

sdk_ret_t
devapi_iris::port_set_config(uint32_t port_num,
                             port_config_t *config)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_port *port = NULL;

    port = devapi_port::find_or_create(port_num);
    if (!port) {
        NIC_LOG_ERR("Failed to find/create port: {}", port_num);
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = port->port_hal_update_config(config);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_add(string name,
                              uint64_t metrics_mem_addr,
                              uint32_t metrics_mem_size)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_add(name, metrics_mem_addr, metrics_mem_size);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_del(string name)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_del(name);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_ring_add(string name,
                                   std::vector<std::pair<const std::string,uint32_t>>& ring_vec)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_ring_add(name, ring_vec);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_ring_del(string name,
                                   std::vector<std::pair<const std::string,uint32_t>>& ring_vec)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_ring_del(name, ring_vec);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_reset_set(string name, uint32_t sub_ring,
                                    bool reset_sense)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_reset_set(name, sub_ring, reset_sense);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_enable_set(string name, uint32_t sub_ring,
                                     bool enable_sense)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_enable_set(name, sub_ring, enable_sense);

end:
    return ret;
}


sdk_ret_t
devapi_iris::accel_rgroup_pndx_set(string name, uint32_t sub_ring,
                                   uint32_t val, bool conditional)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_pndx_set(name, sub_ring, val, conditional);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_info_get(string name, uint32_t sub_ring,
                                   accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                                   void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_info_get(name, sub_ring, rsp_cb_func,
                                       user_ctx, ret_num_entries);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_indices_get(string name,uint32_t sub_ring,
                                      accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                                      void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_indices_get(name, sub_ring, rsp_cb_func,
                                          user_ctx, ret_num_entries);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_metrics_get(string name, uint32_t sub_ring,
                                      accel_rgroup_rmetrics_rsp_cb_t rsp_cb_func,
                                      void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_metrics_get(name, sub_ring, rsp_cb_func,
                                          user_ctx, ret_num_entries);

end:
    return ret;
}

sdk_ret_t
devapi_iris::accel_rgroup_misc_get(string name, uint32_t sub_ring,
                                   accel_rgroup_rmisc_rsp_cb_t rsp_cb_func,
                                   void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->accel_rgroup_misc_get(name, sub_ring, rsp_cb_func,
                                       user_ctx, ret_num_entries);

end:
    return ret;
}

sdk_ret_t
devapi_iris::crypto_key_index_upd(uint32_t key_index, crypto_key_type_t type,
                                  void *key, uint32_t key_size)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_accel *accel = NULL;

    accel = devapi_accel::find_or_create();
    if (!accel) {
        NIC_LOG_ERR("Failed to find/create accel");
        ret = SDK_RET_ERR;
        goto end;
    }
    ret = accel->crypto_key_index_upd(key_index, type, key, key_size);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_state(uint32_t lif_id, lif_state_t state)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update name. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->upd_state(state);

end:
    return ret;
}

sdk_ret_t
devapi_iris::lif_upd_rdma_sniff(uint32_t lif_id, bool rdma_sniff)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_lif *lif = NULL;

    lif = devapi_lif::lookup(lif_id);
    if (!lif) {
        NIC_LOG_ERR("Failed to update RDMA sniff. lif id: {}. Not found",
                    lif_id);
        ret = SDK_RET_ERR;
        goto end;
    }
    return lif->upd_rdma_sniff(rdma_sniff);

end:
    return ret;
}

sdk_ret_t
devapi_iris::swm_enable(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    ret = devapi_swm::swm_initialize(this);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to configure single wire management. err: {}", ret);
        goto end;
    }

end:
    return ret;
}

sdk_ret_t
devapi_iris::swm_disable(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    ret = devapi_swm::swm_uninitialize();
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("Failed to uniinitialize single wire management. err: {}", ret);
        goto end;
    }

end:
    return ret;
}

sdk_ret_t
devapi_iris::swm_create_channel(uint32_t channel, uint32_t port_num)
{
    return devapi_swm::swm()->create_channel(channel, port_num);
}

sdk_ret_t
devapi_iris::swm_get_channels_info(std::set<channel_info_t *>* channels_info)
{
    return devapi_swm::swm()->get_channels_info(channels_info);
}

sdk_ret_t
devapi_iris::swm_add_mac(mac_t mac, uint32_t channel)
{
    return devapi_swm::swm()->add_mac(mac, channel);
}

sdk_ret_t
devapi_iris::swm_del_mac(mac_t mac, uint32_t channel)
{
    return devapi_swm::swm()->del_mac(mac, channel);
}

sdk_ret_t
devapi_iris::swm_add_vlan(vlan_t vlan, uint32_t channel)
{
    return devapi_swm::swm()->add_vlan(vlan, channel);
}

sdk_ret_t
devapi_iris::swm_del_vlan(vlan_t vlan, uint32_t channel)
{
    return devapi_swm::swm()->del_vlan(vlan, channel);
}

sdk_ret_t
devapi_iris::swm_upd_rx_bmode(bool broadcast, uint32_t channel)
{
    return devapi_swm::swm()->upd_rx_bmode(broadcast, channel);
}

sdk_ret_t
devapi_iris::swm_upd_rx_mmode(bool all_multicast, uint32_t channel)
{
    return devapi_swm::swm()->upd_rx_mmode(all_multicast, channel);
}

sdk_ret_t
devapi_iris::swm_upd_rx_pmode(bool promiscuous, uint32_t channel)
{
    return devapi_swm::swm()->upd_rx_pmode(promiscuous, channel);
}

sdk_ret_t
devapi_iris::swm_upd_bcast_filter(lif_bcast_filter_t bcast_filter, uint32_t channel)
{
    return devapi_swm::swm()->upd_bcast_filter(bcast_filter, channel);
}

sdk_ret_t
devapi_iris::swm_upd_mcast_filter(lif_mcast_filter_t mcast_filter, uint32_t channel)
{
    return devapi_swm::swm()->upd_mcast_filter(mcast_filter, channel);
}

sdk_ret_t
devapi_iris::swm_enable_tx(uint32_t channel)
{
    return devapi_swm::swm()->enable_tx(channel);
}

sdk_ret_t
devapi_iris::swm_disable_tx(uint32_t channel)
{
    return devapi_swm::swm()->disable_tx(channel);
}

sdk_ret_t
devapi_iris::swm_enable_rx(uint32_t channel)
{
    return devapi_swm::swm()->enable_rx(channel);
}

sdk_ret_t
devapi_iris::swm_disable_rx(uint32_t channel)
{
    return devapi_swm::swm()->disable_rx(channel);
}

sdk_ret_t
devapi_iris::swm_reset_channel(uint32_t channel)
{
    return devapi_swm::swm()->reset_channel(channel);
}

sdk_ret_t
devapi_iris::swm_upd_vlan_mode(bool enable, uint32_t mode, uint32_t channel)
{
    return devapi_swm::swm()->upd_vlan_mode(enable, mode, channel);;
}

bool
devapi_iris::is_hal_up(void)
{
    return g_devapi_hal.hal_up();
}

void
devapi_iris::inc_num_int_mgmt_mnics(void)
{
    num_int_mgmt_mnics_++;
}

void
devapi_iris::dec_num_int_mgmt_mnics(void)
{
    num_int_mgmt_mnics_--;
}

}    // namespace iris
