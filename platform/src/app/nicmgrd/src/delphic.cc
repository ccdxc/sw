// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include "nic/sdk/include/sdk/types.hpp"

#include "platform/src/lib/nicmgr/include/dev.hpp"

#include "delphic.hpp"
#include "nicmgrd.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "platform/src/lib/devapi_iris/port.hpp"

using namespace std;
using namespace upgrade;
using grpc::Status;
using delphi::error;
using dobj::PortStatus;
using dobj::HalStatus;
using sdk::types::port_oper_status_t;

extern DeviceManager *devmgr;
extern const char* nicmgr_upgrade_state_file;

namespace nicmgr {

// port reactors
port_status_handler_ptr_t g_port_status_handler;
hal_status_handler_ptr_t g_hal_status_handler;
shared_ptr<NicMgrService> g_nicmgr_svc;
system_spec_handler_ptr_t g_system_spec_handler;

// nicmgr delphi service
NicMgrService::NicMgrService(delphi::SdkPtr sk) {
    sdk_ = sk;
    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<nicmgr_upg_hndlr>(),
                                  "nicmgr", NON_AGENT,
                                  (UpgAgentHandlerPtr)NULL);
    sysmgr_ = nicmgr::create_sysmgr_client(sdk_);
}

static void retrieve_devices(std::vector <struct EthDevInfo *> &eth_info)
{
    std::vector <delphi::objects::EthDeviceInfoPtr> EthDevProtoObjs = delphi::objects::EthDeviceInfo::List(g_nicmgr_svc->sdk());

    NIC_FUNC_DEBUG("Retrieving {} EthDevProto objects from Delphi", EthDevProtoObjs.size());
    for (uint32_t idx = 0; idx < EthDevProtoObjs.size(); idx++) {
        struct EthDevInfo *eth_dev_info = new EthDevInfo();
        struct eth_devspec *eth_spec = new eth_devspec();
        struct eth_dev_res *eth_res = new eth_dev_res();

        //populate the eth_res
        eth_res->lif_base = EthDevProtoObjs[idx]->eth_dev_res().lif_base();
        eth_res->intr_base = EthDevProtoObjs[idx]->eth_dev_res().intr_base();
        eth_res->regs_mem_addr = EthDevProtoObjs[idx]->eth_dev_res().regs_mem_addr();
        eth_res->port_info_addr = EthDevProtoObjs[idx]->eth_dev_res().port_info_addr();
        eth_res->cmb_mem_addr = EthDevProtoObjs[idx]->eth_dev_res().cmb_mem_addr();
        eth_res->cmb_mem_size = EthDevProtoObjs[idx]->eth_dev_res().cmb_mem_size();
        eth_res->rom_mem_addr = EthDevProtoObjs[idx]->eth_dev_res().rom_mem_addr();
        eth_res->rom_mem_size = EthDevProtoObjs[idx]->eth_dev_res().rom_mem_size();

        //populate the eth_spec
        eth_spec->dev_uuid = EthDevProtoObjs[idx]->eth_dev_spec().dev_uuid();
        eth_spec->eth_type = (EthDevType)EthDevProtoObjs[idx]->eth_dev_spec().eth_type();
        eth_spec->name = EthDevProtoObjs[idx]->eth_dev_spec().name();
        eth_spec->oprom = (OpromType)EthDevProtoObjs[idx]->eth_dev_spec().oprom();
        eth_spec->pcie_port = EthDevProtoObjs[idx]->eth_dev_spec().pcie_port();
        eth_spec->pcie_total_vfs = EthDevProtoObjs[idx]->eth_dev_spec().pcie_total_vfs();
        eth_spec->host_dev = EthDevProtoObjs[idx]->eth_dev_spec().host_dev();
        eth_spec->uplink_port_num = EthDevProtoObjs[idx]->eth_dev_spec().uplink_port_num();
        eth_spec->qos_group = EthDevProtoObjs[idx]->eth_dev_spec().qos_group();
        eth_spec->lif_count = EthDevProtoObjs[idx]->eth_dev_spec().lif_count();
        eth_spec->rxq_count = EthDevProtoObjs[idx]->eth_dev_spec().rxq_count();
        eth_spec->txq_count = EthDevProtoObjs[idx]->eth_dev_spec().txq_count();
        eth_spec->adminq_count = EthDevProtoObjs[idx]->eth_dev_spec().adminq_count();
        eth_spec->eq_count = EthDevProtoObjs[idx]->eth_dev_spec().eq_count();

        eth_spec->intr_count = EthDevProtoObjs[idx]->eth_dev_spec().intr_count();
        eth_spec->mac_addr = EthDevProtoObjs[idx]->eth_dev_spec().mac_addr();
        eth_spec->enable_rdma = EthDevProtoObjs[idx]->eth_dev_spec().enable_rdma();
        eth_spec->pte_count = EthDevProtoObjs[idx]->eth_dev_spec().pte_count();
        eth_spec->key_count = EthDevProtoObjs[idx]->eth_dev_spec().key_count();
        eth_spec->ah_count = EthDevProtoObjs[idx]->eth_dev_spec().ah_count();
        eth_spec->rdma_sq_count = EthDevProtoObjs[idx]->eth_dev_spec().rdma_sq_count();
        eth_spec->rdma_rq_count = EthDevProtoObjs[idx]->eth_dev_spec().rdma_rq_count();
        eth_spec->rdma_cq_count = EthDevProtoObjs[idx]->eth_dev_spec().rdma_cq_count();
        eth_spec->rdma_eq_count = EthDevProtoObjs[idx]->eth_dev_spec().rdma_eq_count();
        eth_spec->rdma_aq_count = EthDevProtoObjs[idx]->eth_dev_spec().rdma_aq_count();
        eth_spec->rdma_pid_count = EthDevProtoObjs[idx]->eth_dev_spec().rdma_pid_count();
        eth_spec->barmap_size = EthDevProtoObjs[idx]->eth_dev_spec().barmap_size();

        eth_dev_info->eth_res = eth_res;
        eth_dev_info->eth_spec = eth_spec;

        eth_info.push_back(eth_dev_info);
    }

}

static void restore_uplinks()
{
    std::vector <delphi::objects::UplinkInfoPtr> UplinkProtoObjs = delphi::objects::UplinkInfo::List(g_nicmgr_svc->sdk());

    NIC_FUNC_DEBUG("Retrieving {} UplinkProto objects from Delphi", UplinkProtoObjs.size());
    for (uint32_t idx = 0; idx < UplinkProtoObjs.size(); idx++) {
        devmgr->CreateUplinks(UplinkProtoObjs[idx]->id(), UplinkProtoObjs[idx]->port(), UplinkProtoObjs[idx]->is_oob());
    }
}

// OnMountComplete() gets called after all delphi objects are mounted
void NicMgrService::OnMountComplete() {
    std::vector <struct EthDevInfo *> eth_info;

    NIC_LOG_DEBUG("On mount complete got called");

    this->sysmgr_->init_done();

    restore_uplinks();
    retrieve_devices(eth_info);
    NIC_FUNC_DEBUG("Retrieved {} eth_info objects from Delphi", eth_info.size());

    // Restore eth devices retrieved from delphi db
    if (eth_info.size()) {
        //if we see the delphi object in normal boot then we should not use it and clean it up
        if (devmgr->GetUpgradeMode() == FW_MODE_NORMAL_BOOT) {
            // walk all objects and delete them
            vector<delphi::objects::EthDeviceInfoPtr> objlist = delphi::objects::EthDeviceInfo::List(g_nicmgr_svc->sdk());
            for (vector<delphi::objects::EthDeviceInfoPtr>::iterator obj=objlist.begin(); obj !=objlist.end(); ++obj) {
                g_nicmgr_svc->sdk()->DeleteObject(*obj);
            }
        }
        else {
            devmgr->RestoreDevicesState(eth_info);

            // All nicmgr objects are restored now so we can delete the file if exists
            unlink(nicmgr_upgrade_state_file);
        }
    }

    // walk all port status objects and handle them
    vector <delphi::objects::PortStatusPtr> list =
        delphi::objects::PortStatus::List(sdk_);
    for (vector<delphi::objects::PortStatusPtr>::iterator port=list.begin();
         port!=list.end(); ++port) {
        g_port_status_handler->update_port_status(*port);
    }
    devmgr->DelphiMountEventHandler(true);
}

// Name returns the name of the delphi service
std::string NicMgrService::Name() {
    return "nicmgr";
}

// get_port_handler gets the port reactor object
port_status_handler_ptr_t get_port_status_handler (void) {
    return g_port_status_handler;
}

// init_port_handler creates a port reactor
Status init_port_status_handler (delphi::SdkPtr sdk) {
    // create the PortStatus reactor
    g_port_status_handler = std::make_shared<port_status_handler>(sdk);

    // mount objects
    delphi::objects::PortStatus::Mount(sdk, delphi::ReadMode);

    // Register PortStatus reactor
    delphi::objects::PortStatus::Watch(sdk, g_port_status_handler);

    return Status::OK;
}

// OnPortStatusCreate gets called when PortStatus object is created
error port_status_handler::OnPortStatusCreate(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status create");
    update_port_status(portStatus);
    return error::OK();
}

// OnPortStatusUpdate gets called when PortStatus object is updated
error port_status_handler::OnPortStatusUpdate(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status update");
    update_port_status(portStatus);
    return error::OK();
}

// OnPortStatusDelete gets called when PortStatus object is deleted
error port_status_handler::OnPortStatusDelete(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status delete");
    return error::OK();
}

// update_port_status_handler updates port status in delphi
error port_status_handler::update_port_status(PortStatusPtr port) {
    // create port status object
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    // ignore if there is no link/xcvr event
    if (!port->has_link_status() && !port->has_xcvr_status()) {
        return error::OK();
    }

    port_status_t st = {0};

    st.id = port->key_or_handle().port_id();
    if (port->has_link_status()) {
        NIC_FUNC_DEBUG("Received link status: id: {} oper_state: {}, speed: {}",
                       port->key_or_handle().port_id(),
                       port->link_status().oper_state(),
                       port->link_status().port_speed());
        st.status = port->link_status().oper_state();
        // st.speed = HalClient::PortSpeedInMbps(port->port_speed());
        st.speed = devapi_port::port_speed_enum_to_mbps(
                                        port->link_status().port_speed());
        devmgr->LinkEventHandler(&st);
    }
    if (port->has_xcvr_status()) {
        NIC_FUNC_DEBUG("Received xcvr status: id: {} state: {}, "
                       "cable_type: {}, pid: {}",
                       port->key_or_handle().port_id(),
                       port->xcvr_status().state(),
                       port->xcvr_status().cable_type(),
                       port->xcvr_status().pid());
        st.xcvr.state = port->xcvr_status().state();
        st.xcvr.pid = port->xcvr_status().pid();
        st.xcvr.phy = port->xcvr_status().cable_type();
        memcpy(st.xcvr.sprom,
            port->xcvr_status().xcvr_sprom().c_str(),
            MIN(port->xcvr_status().xcvr_sprom().size(),
            sizeof (st.xcvr.sprom)));
        devmgr->XcvrEventHandler(&st);
    }

    NIC_FUNC_DEBUG("id {} status {} speed {} "
        " xcvr.state {} xcvr.phy {} xcvr.pid {}",
        st.id, st.status, st.speed,
        st.xcvr.state, st.xcvr.phy, st.xcvr.pid);

    return error::OK();
}

system_spec_handler_ptr_t get_system_spec_handler (void) {
    return g_system_spec_handler;
}

Status init_system_spec_handler(delphi::SdkPtr sdk)
{
    g_system_spec_handler = std::make_shared<system_spec_handler>(sdk);
    delphi::objects::SystemSpec::Mount(sdk, delphi::ReadMode);
    delphi::objects::SystemSpec::Watch(sdk, g_system_spec_handler);

    return Status::OK;
}

error system_spec_handler::OnSystemSpecCreate(SystemSpecPtr obj)
{
    NIC_LOG_DEBUG("Rcvd system spec create");
    update_system_spec(obj);
    return error::OK();
}

error system_spec_handler::OnSystemSpecUpdate(SystemSpecPtr obj)
{
    NIC_LOG_DEBUG("Rcvd system spec update");
    update_system_spec(obj);
    return error::OK();
}

error system_spec_handler::OnSystemSpecDelete(SystemSpecPtr obj)
{
    NIC_LOG_DEBUG("Rcvd system spec delete");
    return error::OK();
}

error system_spec_handler::update_system_spec(SystemSpecPtr obj)
{
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    NIC_LOG_DEBUG("System spec update: micro_seg_en: {}", 
                  obj->micro_seg_mode() == device::MICRO_SEG_ENABLE);
    
    devmgr->SystemSpecEventHandler(obj->micro_seg_mode() == 
                                   device::MICRO_SEG_ENABLE);

    return error::OK();
}



// get_hal_status_handler gets the port reactor object
hal_status_handler_ptr_t get_hal_status_handler (void) {
    return g_hal_status_handler;
}

// init_hal_status_handler creates a HAL status reactor
Status init_hal_status_handler (delphi::SdkPtr sdk) {
    // create the HalStatus reactor
    g_hal_status_handler = std::make_shared<hal_status_handler>(sdk);

    // mount objects
    HalStatus::Mount(sdk, delphi::ReadMode);

    // register HalStatus reactor
    HalStatus::Watch(sdk, g_hal_status_handler);

    return Status::OK;
}

// OnHalStatusCreate gets called when HalStatus object is created
error hal_status_handler::OnHalStatusCreate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusCreate notification for {}", halStatus->ShortDebugString());
    devmgr->HalEventHandler(halStatus->state() == ::hal::HAL_STATE_UP);
    return error::OK();
}

// OnHalStatusUpdate gets called when HalStatus object is updated
error hal_status_handler::OnHalStatusUpdate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusUpdate notification for {}", halStatus->ShortDebugString());
    devmgr->HalEventHandler(halStatus->state() == ::hal::HAL_STATE_UP);
    return error::OK();
}

// init_accel_objects mounts accelerator objects
Status init_accel_objects(delphi::SdkPtr sdk) {
    dobj::AccelPfInfo::Mount(sdk, delphi::ReadWriteMode);
    return Status::OK;
}

// init_eth_objects mounts eth_dev objects
Status init_eth_objects(delphi::SdkPtr sdk) {
    delphi::objects::EthDeviceInfo::Mount(sdk, delphi::ReadWriteMode);
    delphi::objects::UplinkInfo::Mount(sdk, delphi::ReadWriteMode);
    return Status::OK;
}
// initialization function for delphi
void
delphi_init (void)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    g_nicmgr_svc = make_shared<NicMgrService>(sdk);

    // override delphi's logger with our logger
    delphi::SetLogger(std::shared_ptr<spdlog::logger>(utils::logger::logger()));

    // register NicMgr as Delphi Service
    sdk->RegisterService(g_nicmgr_svc);

    // init port status handler
    init_port_status_handler(sdk);

    // init hal status handler
    init_hal_status_handler(sdk);

    // init system handler
    init_system_spec_handler(sdk);

    // init accel dev related handlers
    init_accel_objects(sdk);

    // init eth_dev delphi objects
    init_eth_objects(sdk);

    // connect to delphi
    sdk->Connect();
}

}    // namespace nicmgr
