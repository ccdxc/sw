//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/hal/core/core.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "platform/src/lib/devapi_iris/devapi_iris.hpp"
#include "gen/proto/nicmgr/nicmgr.delphi.hpp"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/iris/delphi/nicmgr_delphic.hpp"
#include "upgrade_rel_a2b.hpp"

extern DeviceManager *devmgr;

namespace nicmgr {

bool g_device_restored = true;
static delphi::SdkPtr delphi_sdk;
static volatile bool mount_completed = false;

static void
restore_devices(void)
{
    std::vector <delphi::objects::EthDeviceInfoPtr> eth_proto_list =
        delphi::objects::EthDeviceInfo::List(delphi_sdk);

    NIC_FUNC_DEBUG("Retreiving {} EthDevProto objects from Delphi", eth_proto_list.size());
    for (auto it = eth_proto_list.begin(); it != eth_proto_list.end(); ++it) {

        auto eth_proto_obj = *it;
        struct EthDevInfo *eth_dev_info = new EthDevInfo();
        struct eth_devspec *eth_spec = new eth_devspec();
        struct eth_dev_res *eth_res = new eth_dev_res();

        // populate the eth_res
        eth_res->lif_base = eth_proto_obj->eth_dev_res().lif_base();
        eth_res->intr_base = eth_proto_obj->eth_dev_res().intr_base();
        eth_res->regs_mem_addr = eth_proto_obj->eth_dev_res().regs_mem_addr();
        eth_res->port_info_addr = eth_proto_obj->eth_dev_res().port_info_addr();
        eth_res->cmb_mem_addr = eth_proto_obj->eth_dev_res().cmb_mem_addr();
        eth_res->cmb_mem_size = eth_proto_obj->eth_dev_res().cmb_mem_size();
        eth_res->rom_mem_addr = eth_proto_obj->eth_dev_res().rom_mem_addr();
        eth_res->rom_mem_size = eth_proto_obj->eth_dev_res().rom_mem_size();

        // populate the eth_spec
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

        NIC_LOG_DEBUG("Restore eth dev {} uplink {}",
                      eth_proto_obj->eth_dev_spec().name(), eth_spec->uplink_port_num);
        // convert uplink number to new format
        if (eth_spec->uplink_port_num == 1) {
            eth_spec->uplink_port_num = 285278209;
        } else if (eth_spec->uplink_port_num == 5) {
            eth_spec->uplink_port_num = 285343745;
        } else if (eth_spec->uplink_port_num == 9) {
            eth_spec->uplink_port_num = 285409281;
        }

        eth_dev_info->eth_res = eth_res;
        eth_dev_info->eth_spec = eth_spec;

        devmgr->RestoreDevice(ETH, eth_dev_info);
    }
}

static void
restore_uplinks(void)
{
    std::vector <delphi::objects::UplinkInfoPtr> uplink_proto_list =
        delphi::objects::UplinkInfo::List(delphi_sdk);
    uint32_t id, port;

    NIC_FUNC_DEBUG("Retreiving {} UplinkProto objects from Delphi", uplink_proto_list.size());
    for (auto it = uplink_proto_list.begin(); it != uplink_proto_list.end(); ++it) {
        auto uplink_proto_obj = *it;
        if (uplink_proto_obj->port() == 1) {
            id = 1359020033;
            port = 285278209;
        } else if (uplink_proto_obj->port() == 5) {
            id = 1359085569;
            port = 285343745;
        } else if (uplink_proto_obj->port() == 9) {
            id = 1359151105;
            port = 285409281;
        }
        devmgr->CreateUplink(id, port, uplink_proto_obj->is_oob());
    }
}

// below function is called from delphi thread
void
nicmgr_delphi_mount_complete (void)
{
    mount_completed = true;
    delphi_sdk = hal::svc::nicmgr_delphic_sdk();
}

static void
mount_complete_handler (void)
{
    NIC_LOG_DEBUG("In mount complete handler");
    if (mount_completed && !g_device_restored) {
        restore_uplinks();
        restore_devices();
        g_device_restored = true;
        if (devapi_iris::is_hal_up()) {
            devmgr->HalEventHandler(true);
        }
    }
}



void
restore_from_delphi (void)
{
    NIC_LOG_DEBUG("Restore from delphi..");
    g_device_restored = false;
    while (!mount_completed) {
        pthread_yield();
    }
    mount_complete_handler();
}

}   // namespace nicmgr
