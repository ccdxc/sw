// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include "nic/sdk/include/sdk/types.hpp"

#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"

#include "delphic.hpp"
#include "nicmgrd.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "platform/src/lib/devapi_iris/port.hpp"

using namespace std;
using namespace upgrade;
using grpc::Status;
using delphi::error;
using dobj::PortStatus;
using dobj::NcsiVlanFilter;
using dobj::NcsiMacFilter;
using dobj::NcsiChanRx;
using dobj::NcsiChanTx;
using dobj::NcsiBcastFilter;
using dobj::NcsiMcastFilter;
using dobj::HalStatus;
using sdk::types::port_oper_status_t;

extern DeviceManager *devmgr;
extern UpgradeMode upg_mode;
extern const char* nicmgr_upgrade_state_file;
extern const char* nicmgr_rollback_state_file;

namespace nicmgr {

// port reactors
port_status_handler_ptr_t g_port_status_handler;
hal_status_handler_ptr_t g_hal_status_handler;
ncsi_vlan_filter_handler_ptr_t g_ncsi_vlan_filter_handler;
ncsi_mac_filter_handler_ptr_t g_ncsi_mac_filter_handler;
ncsi_chan_rx_handler_ptr_t g_ncsi_chan_rx_handler;
ncsi_chan_tx_handler_ptr_t g_ncsi_chan_tx_handler;
ncsi_bcast_filter_handler_ptr_t g_ncsi_bcast_filter_handler;
ncsi_mcast_filter_handler_ptr_t g_ncsi_mcast_filter_handler;

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

static void restore_devices()
{
    std::vector <delphi::objects::EthDeviceInfoPtr> eth_proto_list =
        delphi::objects::EthDeviceInfo::List(g_nicmgr_svc->sdk());

    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return;
    }

    NIC_FUNC_DEBUG("Retreiving {} EthDevProto objects from Delphi", eth_proto_list.size());
    for (auto it = eth_proto_list.begin(); it != eth_proto_list.end(); ++it) {

        auto eth_proto_obj = *it;
        struct EthDevInfo *eth_dev_info = new EthDevInfo();
        struct eth_devspec *eth_spec = new eth_devspec();
        struct eth_dev_res *eth_res = new eth_dev_res();

        //populate the eth_res
        eth_res->lif_base = eth_proto_obj->eth_dev_res().lif_base();
        eth_res->intr_base = eth_proto_obj->eth_dev_res().intr_base();
        eth_res->regs_mem_addr = eth_proto_obj->eth_dev_res().regs_mem_addr();
        eth_res->port_info_addr = eth_proto_obj->eth_dev_res().port_info_addr();
        eth_res->cmb_mem_addr = eth_proto_obj->eth_dev_res().cmb_mem_addr();
        eth_res->cmb_mem_size = eth_proto_obj->eth_dev_res().cmb_mem_size();
        eth_res->rom_mem_addr = eth_proto_obj->eth_dev_res().rom_mem_addr();
        eth_res->rom_mem_size = eth_proto_obj->eth_dev_res().rom_mem_size();

        //populate the eth_spec
        eth_spec->dev_uuid = eth_proto_obj->eth_dev_spec().dev_uuid();
        eth_spec->eth_type = (EthDevType)eth_proto_obj->eth_dev_spec().eth_type();
        eth_spec->name = eth_proto_obj->eth_dev_spec().name();
        eth_spec->oprom = (OpromType)eth_proto_obj->eth_dev_spec().oprom();
        eth_spec->pcie_port = eth_proto_obj->eth_dev_spec().pcie_port();
        eth_spec->pcie_total_vfs = eth_proto_obj->eth_dev_spec().pcie_total_vfs();
        eth_spec->host_dev = eth_proto_obj->eth_dev_spec().host_dev();
        eth_spec->uplink_port_num = eth_proto_obj->eth_dev_spec().uplink_port_num();
        eth_spec->qos_group = eth_proto_obj->eth_dev_spec().qos_group();
        eth_spec->lif_count = eth_proto_obj->eth_dev_spec().lif_count();
        eth_spec->rxq_count = eth_proto_obj->eth_dev_spec().rxq_count();
        eth_spec->txq_count = eth_proto_obj->eth_dev_spec().txq_count();
        eth_spec->adminq_count = eth_proto_obj->eth_dev_spec().adminq_count();
        eth_spec->eq_count = eth_proto_obj->eth_dev_spec().eq_count();

        eth_spec->intr_count = eth_proto_obj->eth_dev_spec().intr_count();
        eth_spec->mac_addr = eth_proto_obj->eth_dev_spec().mac_addr();
        eth_spec->enable_rdma = eth_proto_obj->eth_dev_spec().enable_rdma();
        eth_spec->pte_count = eth_proto_obj->eth_dev_spec().pte_count();
        eth_spec->key_count = eth_proto_obj->eth_dev_spec().key_count();
        eth_spec->ah_count = eth_proto_obj->eth_dev_spec().ah_count();
        eth_spec->rdma_sq_count = eth_proto_obj->eth_dev_spec().rdma_sq_count();
        eth_spec->rdma_rq_count = eth_proto_obj->eth_dev_spec().rdma_rq_count();
        eth_spec->rdma_cq_count = eth_proto_obj->eth_dev_spec().rdma_cq_count();
        eth_spec->rdma_eq_count = eth_proto_obj->eth_dev_spec().rdma_eq_count();
        eth_spec->rdma_aq_count = eth_proto_obj->eth_dev_spec().rdma_aq_count();
        eth_spec->rdma_pid_count = eth_proto_obj->eth_dev_spec().rdma_pid_count();
        eth_spec->barmap_size = eth_proto_obj->eth_dev_spec().barmap_size();

        eth_dev_info->eth_res = eth_res;
        eth_dev_info->eth_spec = eth_spec;

        devmgr->RestoreDevice(ETH, eth_dev_info);
    }

}

static void restore_uplinks()
{
    std::vector <delphi::objects::UplinkInfoPtr> uplink_proto_list =
        delphi::objects::UplinkInfo::List(g_nicmgr_svc->sdk());

    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return;
    }

    NIC_FUNC_DEBUG("Retreiving {} UplinkProto objects from Delphi", uplink_proto_list.size());
    for (auto it = uplink_proto_list.begin(); it != uplink_proto_list.end(); ++it) {
        auto uplink_proto_obj = *it;
        devmgr->CreateUplink(uplink_proto_obj->id(), uplink_proto_obj->port(),
            uplink_proto_obj->is_oob());
    }
}

// OnMountComplete() gets called after all delphi objects are mounted
void NicMgrService::OnMountComplete() {
    NIC_LOG_DEBUG("On mount complete got called");

    this->sysmgr_->init_done();

    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return;
    }

    if (upg_mode == FW_MODE_UPGRADE) {
        restore_uplinks();
        restore_devices();
    }

    // handle hal status
    std::vector <delphi::objects::HalStatusPtr> halstatus_list =
        delphi::objects::HalStatus::List(sdk_);
    for (auto it = halstatus_list.begin(); it != halstatus_list.end(); ++it) {
        g_hal_status_handler->update_hal_status(*it);
    }

    // handle port status
    std::vector <delphi::objects::PortStatusPtr> portstatus_list =
        delphi::objects::PortStatus::List(sdk_);
    for (auto it = portstatus_list.begin(); it != portstatus_list.end(); ++it) {
        g_port_status_handler->update_port_status(*it);
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
    return update_port_status(portStatus);
}

// OnPortStatusUpdate gets called when PortStatus object is updated
error port_status_handler::OnPortStatusUpdate(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status update");
    return update_port_status(portStatus);
}

// OnPortStatusDelete gets called when PortStatus object is deleted
error port_status_handler::OnPortStatusDelete(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status delete");
    return error::OK();
}

// update_port_status_handler updates port status in delphi
error port_status_handler::update_port_status(PortStatusPtr port) {
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    // ignore if there is no link/xcvr event
    if (!port->has_link_status() && !port->has_xcvr_status()) {
        NIC_LOG_ERR("Rcvd port status without link & xcvr status");
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

error hal_status_handler::update_hal_status(HalStatusPtr halStatus) {
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (!status && halStatus->state() == ::hal::HAL_STATE_UP) {
        status = true;
        devmgr->HalEventHandler(status);
    } else if (status && halStatus->state() != ::hal::HAL_STATE_UP) {
        status = false;
        devmgr->HalEventHandler(status);
    }

    return error::OK();
}

// OnHalStatusCreate gets called when HalStatus object is created
error hal_status_handler::OnHalStatusCreate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusCreate notification for {}", halStatus->ShortDebugString());
    return update_hal_status(halStatus);
}

// OnHalStatusUpdate gets called when HalStatus object is updated
error hal_status_handler::OnHalStatusUpdate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusUpdate notification for {}", halStatus->ShortDebugString());
    return update_hal_status(halStatus);
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

//------------------------------------------------------------------------------
// Proto Message dump
//------------------------------------------------------------------------------
void
proto_msg_dump (Message& msg)
{
    std::string    msg_str;

    google::protobuf::util::MessageToJsonString(msg, &msg_str);
    NIC_LOG_DEBUG("{}", msg_str.c_str());
}

//------------------------------------------------------------------------------
// Vlan filter 
//------------------------------------------------------------------------------
Status init_ncsi_vlan_filter_handler (delphi::SdkPtr sdk) {
    g_ncsi_vlan_filter_handler = std::make_shared<ncsi_vlan_filter_handler>(sdk);
    delphi::objects::NcsiVlanFilter::Mount(sdk, delphi::ReadMode);
    delphi::objects::NcsiVlanFilter::Watch(sdk, g_ncsi_vlan_filter_handler);
    return Status::OK;
}
error 
ncsi_vlan_filter_handler::update_vlan_filter(NcsiVlanFilterPtr obj, 
                                             bool create)
{
    proto_msg_dump(*obj); 
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (create) {
        devmgr->DevApi()->swm_add_vlan(obj->vlan_id());
    } else {
        devmgr->DevApi()->swm_del_vlan(obj->vlan_id());
    }
    return error::OK();
}
error ncsi_vlan_filter_handler::OnNcsiVlanFilterCreate(NcsiVlanFilterPtr obj) {
    NIC_LOG_DEBUG("Rcvd ncsi vlan filter create");
    return update_vlan_filter(obj, true);
}
error ncsi_vlan_filter_handler::OnNcsiVlanFilterDelete(NcsiVlanFilterPtr obj) {
    NIC_LOG_DEBUG("Rcvd ncsi vlan filter delete");
    return update_vlan_filter(obj, false);
}

//------------------------------------------------------------------------------
// MAC filter 
//------------------------------------------------------------------------------
Status init_ncsi_mac_filter_handler (delphi::SdkPtr sdk) {
    g_ncsi_mac_filter_handler = std::make_shared<ncsi_mac_filter_handler>(sdk);
    delphi::objects::NcsiMacFilter::Mount(sdk, delphi::ReadMode);
    delphi::objects::NcsiMacFilter::Watch(sdk, g_ncsi_mac_filter_handler);
    return Status::OK;
}
error 
ncsi_mac_filter_handler::update_mac_filter(NcsiMacFilterPtr obj, 
                                           bool create)
{
    proto_msg_dump(*obj); 
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (create) {
        devmgr->DevApi()->swm_add_mac(obj->mac_addr());
    } else {
        devmgr->DevApi()->swm_del_mac(obj->mac_addr());
    }
    return error::OK();
}
error ncsi_mac_filter_handler::OnNcsiMacFilterCreate(NcsiMacFilterPtr obj) {
    NIC_LOG_DEBUG("Rcvd ncsi mac filter create");
    return update_mac_filter(obj, true);
}
error ncsi_mac_filter_handler::OnNcsiMacFilterDelete(NcsiMacFilterPtr obj) {
    NIC_LOG_DEBUG("Rcvd ncsi mac filter delete");
    return update_mac_filter(obj, false);
}

//------------------------------------------------------------------------------
// RX Enable
//------------------------------------------------------------------------------
Status init_ncsi_chan_rx_handler(delphi::SdkPtr sdk)
{
    g_ncsi_chan_rx_handler = std::make_shared<ncsi_chan_rx_handler>(sdk);
    delphi::objects::NcsiChanRx::Mount(sdk, delphi::ReadMode);
    delphi::objects::NcsiChanRx::Watch(sdk, g_ncsi_chan_rx_handler);
    return Status::OK;
}
error
ncsi_chan_rx_handler::update_rx(NcsiChanRxPtr obj, bool create)
{
    proto_msg_dump(*obj); 
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (create) {
    } else {
    }
    return error::OK();
}
error ncsi_chan_rx_handler::OnNcsiChanRxCreate(NcsiChanRxPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi RX create");
    return update_rx(obj, true);
}
error ncsi_chan_rx_handler::OnNcsiChanRxDelete(NcsiChanRxPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi RX delete");
    return update_rx(obj, true);
}

//------------------------------------------------------------------------------
// TX Enable
//------------------------------------------------------------------------------
Status init_ncsi_chan_tx_handler(delphi::SdkPtr sdk)
{
    g_ncsi_chan_tx_handler = std::make_shared<ncsi_chan_tx_handler>(sdk);
    delphi::objects::NcsiChanTx::Mount(sdk, delphi::ReadMode);
    delphi::objects::NcsiChanTx::Watch(sdk, g_ncsi_chan_tx_handler);
    return Status::OK;
}
error
ncsi_chan_tx_handler::update_tx(NcsiChanTxPtr obj, bool create)
{
    proto_msg_dump(*obj); 
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (create) {
    } else {
    }
    return error::OK();
}
error ncsi_chan_tx_handler::OnNcsiChanTxCreate(NcsiChanTxPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi TX create");
    return update_tx(obj, true);
}
error ncsi_chan_tx_handler::OnNcsiChanTxDelete(NcsiChanTxPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi TX delete");
    return update_tx(obj, true);
}

//------------------------------------------------------------------------------
// Bcast
//------------------------------------------------------------------------------
Status init_ncsi_bcast_filter_handler(delphi::SdkPtr sdk)
{
    g_ncsi_bcast_filter_handler = std::make_shared<ncsi_bcast_filter_handler>(sdk);
    delphi::objects::NcsiBcastFilter::Mount(sdk, delphi::ReadMode);
    delphi::objects::NcsiBcastFilter::Watch(sdk, g_ncsi_bcast_filter_handler);
    return Status::OK;
}
error
ncsi_bcast_filter_handler::update_bcast(NcsiBcastFilterPtr obj, 
                                        bool create_update)
{
    lif_bcast_filter_t bcast_filter = {0};

    proto_msg_dump(*obj); 
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (create_update) {
        bcast_filter.arp = obj->enable_arp();
        bcast_filter.dhcp_client = obj->enable_dhcp_client();
        bcast_filter.dhcp_server = obj->enable_dhcp_server();
        bcast_filter.netbios = obj->enable_netbios();
    }
    devmgr->DevApi()->swm_upd_bcast_filter(bcast_filter);
    return error::OK();
}
error ncsi_bcast_filter_handler::OnNcsiBcastFilterCreate(NcsiBcastFilterPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi bcast create");
    return update_bcast(obj, true);
}
error ncsi_bcast_filter_handler::OnNcsiBcastFilterDelete(NcsiBcastFilterPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi bcast delete");
    return update_bcast(obj, false);
}
error ncsi_bcast_filter_handler::OnNcsiBcastFilterUpdate(NcsiBcastFilterPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi bcast update");
    return update_bcast(obj, true);
}

//------------------------------------------------------------------------------
// Mcast
//------------------------------------------------------------------------------
Status init_ncsi_mcast_filter_handler(delphi::SdkPtr sdk)
{
    g_ncsi_mcast_filter_handler = std::make_shared<ncsi_mcast_filter_handler>(sdk);
    delphi::objects::NcsiMcastFilter::Mount(sdk, delphi::ReadMode);
    delphi::objects::NcsiMcastFilter::Watch(sdk, g_ncsi_mcast_filter_handler);
    return Status::OK;
}
error
ncsi_mcast_filter_handler::update_mcast(NcsiMcastFilterPtr obj, 
                                        bool create_update)
{
    lif_mcast_filter_t mcast_filter = {0};

    proto_msg_dump(*obj); 
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    if (create_update) {
        mcast_filter.ipv6_neigh_adv = obj->enable_ipv6_neigh_adv();
        mcast_filter.ipv6_router_adv = obj->enable_ipv6_router_adv();
        mcast_filter.dhcpv6_relay = obj->enable_dhcpv6_relay();
        mcast_filter.dhcpv6_mcast = obj->enable_dhcpv6_mcast();
        mcast_filter.ipv6_mld = obj->enable_ipv6_mld();
        mcast_filter.ipv6_neigh_sol = obj->enable_ipv6_neigh_sol();
    }
    devmgr->DevApi()->swm_upd_mcast_filter(mcast_filter);
    return error::OK();
}
error ncsi_mcast_filter_handler::OnNcsiMcastFilterCreate(NcsiMcastFilterPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi mcast create");
    return update_mcast(obj, true);
}
error ncsi_mcast_filter_handler::OnNcsiMcastFilterDelete(NcsiMcastFilterPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi mcast delete");
    return update_mcast(obj, false);
}
error ncsi_mcast_filter_handler::OnNcsiMcastFilterUpdate(NcsiMcastFilterPtr obj)
{
    NIC_LOG_DEBUG("Rcvd ncsi mcast update");
    return update_mcast(obj, true);
}


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

    //init ncsi handlers
    init_ncsi_vlan_filter_handler(sdk);
    init_ncsi_mac_filter_handler(sdk);
    init_ncsi_chan_rx_handler(sdk);
    init_ncsi_chan_tx_handler(sdk);
    init_ncsi_bcast_filter_handler(sdk);
    init_ncsi_mcast_filter_handler(sdk);

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
