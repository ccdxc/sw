//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"

#include "nic/hal/hal_trace.hpp"
#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/iris/delphi/nicmgr_delphic.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "platform/src/lib/nicmgr/include/nicmgr_delphic_ipc.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"

using namespace std;

extern DeviceManager *devmgr;

namespace hal {
namespace svc {

shared_ptr<NicMgrDelphic> g_nicmgr_delphic;

using grpc::Status;
using delphi::error;

static void
uplinkinfo_delphi_save (void)
{
    uplink_t *up;
    std::map<uint32_t, uplink_t*> up_links = devmgr->GetUplinks();

    HAL_TRACE_DEBUG("Saving {} objects of UplinkInfo to delphi", up_links.size());
    for (auto it = up_links.begin(); it != up_links.end(); it++) {
        up = it->second;
        delphi::objects::UplinkInfoPtr proto_obj = make_shared<delphi::objects::UplinkInfo>();
        HAL_TRACE_DEBUG("Saving Uplink info for uplink id: {}", up->id);

        if (up->port == 285278209) {
            proto_obj->set_key(128);
            proto_obj->set_id(128);
            proto_obj->set_port(1);
        } else if (up->port == 285343745) {
            proto_obj->set_key(129);
            proto_obj->set_id(129);
            proto_obj->set_port(5);
        } else if (up->port == 285409281) {
            proto_obj->set_key(130);
            proto_obj->set_id(130);
            proto_obj->set_port(9);
        }
        proto_obj->set_is_oob(up->is_oob);

        g_nicmgr_delphic->sdk()->SetObject(proto_obj);

        HAL_TRACE_DEBUG("Commiting to Delphi uplink id {} port {}", proto_obj->id(), proto_obj->port());
    }
}

static void
uplinkinfo_delphi_clear (void)
{
    std::vector <delphi::objects::UplinkInfoPtr> uplink_list =
        delphi::objects::UplinkInfo::List(g_nicmgr_delphic->sdk());

    for (vector<delphi::objects::UplinkInfoPtr>::iterator obj=uplink_list.begin();
         obj != uplink_list.end(); ++obj) {
        g_nicmgr_delphic->sdk()->DeleteObject(*obj);
    }

    HAL_TRACE_DEBUG("Cleared objects of UplinkInfo from delphi");
}


static void
ethdevinfo_delphi_save (void)
{
    struct EthDevInfo *eth_dev_info;
    std::vector <struct EthDevInfo*> dev_info;
    dev_info = devmgr->GetEthDevStateInfo();

    HAL_TRACE_DEBUG("Saving {} objects of EthDevInfo to delphi", dev_info.size());
    //for each element in dev_info convert it to protobuf and then setobject to delphi
    for (uint32_t eth_dev_idx = 0; eth_dev_idx < dev_info.size(); eth_dev_idx++) {
        delphi::objects::EthDeviceInfoPtr proto_obj = make_shared<delphi::objects::EthDeviceInfo>();
        eth_dev_info = dev_info[eth_dev_idx];
        HAL_TRACE_DEBUG("Saving EthDevInfo for eth_dev_idx {}", eth_dev_idx);

        //device name is the key
        proto_obj->set_key(eth_dev_info->eth_spec->name);

        //save eth_dev resources
        proto_obj->mutable_eth_dev_res()->set_lif_base(eth_dev_info->eth_res->lif_base);
        proto_obj->mutable_eth_dev_res()->set_intr_base(eth_dev_info->eth_res->intr_base);
        proto_obj->mutable_eth_dev_res()->set_regs_mem_addr(eth_dev_info->eth_res->regs_mem_addr);
        proto_obj->mutable_eth_dev_res()->set_port_info_addr(eth_dev_info->eth_res->port_info_addr);
        proto_obj->mutable_eth_dev_res()->set_cmb_mem_addr(eth_dev_info->eth_res->cmb_mem_addr);
        proto_obj->mutable_eth_dev_res()->set_cmb_mem_size(eth_dev_info->eth_res->cmb_mem_size);
        proto_obj->mutable_eth_dev_res()->set_rom_mem_addr(eth_dev_info->eth_res->rom_mem_addr);
        proto_obj->mutable_eth_dev_res()->set_rom_mem_size(eth_dev_info->eth_res->rom_mem_size);

        //save eth_dev specs
        proto_obj->mutable_eth_dev_spec()->set_dev_uuid(eth_dev_info->eth_spec->dev_uuid);
        proto_obj->mutable_eth_dev_spec()->set_eth_type(eth_dev_info->eth_spec->eth_type);
        proto_obj->mutable_eth_dev_spec()->set_name(eth_dev_info->eth_spec->name);
        proto_obj->mutable_eth_dev_spec()->set_oprom(eth_dev_info->eth_spec->oprom);
        proto_obj->mutable_eth_dev_spec()->set_pcie_port(eth_dev_info->eth_spec->pcie_port);
        proto_obj->mutable_eth_dev_spec()->set_pcie_total_vfs(eth_dev_info->eth_spec->pcie_total_vfs);
        proto_obj->mutable_eth_dev_spec()->set_host_dev(eth_dev_info->eth_spec->host_dev);
        proto_obj->mutable_eth_dev_spec()->set_qos_group(eth_dev_info->eth_spec->qos_group);
        proto_obj->mutable_eth_dev_spec()->set_lif_count(eth_dev_info->eth_spec->lif_count);
        proto_obj->mutable_eth_dev_spec()->set_rxq_count(eth_dev_info->eth_spec->rxq_count);
        proto_obj->mutable_eth_dev_spec()->set_txq_count(eth_dev_info->eth_spec->txq_count);
        proto_obj->mutable_eth_dev_spec()->set_eq_count(eth_dev_info->eth_spec->eq_count);
        proto_obj->mutable_eth_dev_spec()->set_adminq_count(eth_dev_info->eth_spec->adminq_count);
        proto_obj->mutable_eth_dev_spec()->set_intr_count(eth_dev_info->eth_spec->intr_count);
        proto_obj->mutable_eth_dev_spec()->set_mac_addr(eth_dev_info->eth_spec->mac_addr);
        proto_obj->mutable_eth_dev_spec()->set_enable_rdma(eth_dev_info->eth_spec->enable_rdma);
        proto_obj->mutable_eth_dev_spec()->set_pte_count(eth_dev_info->eth_spec->pte_count);
        proto_obj->mutable_eth_dev_spec()->set_key_count(eth_dev_info->eth_spec->key_count);
        proto_obj->mutable_eth_dev_spec()->set_ah_count(eth_dev_info->eth_spec->ah_count);
        proto_obj->mutable_eth_dev_spec()->set_rdma_sq_count(eth_dev_info->eth_spec->rdma_sq_count);
        proto_obj->mutable_eth_dev_spec()->set_rdma_rq_count(eth_dev_info->eth_spec->rdma_rq_count);
        proto_obj->mutable_eth_dev_spec()->set_rdma_cq_count(eth_dev_info->eth_spec->rdma_eq_count);
        proto_obj->mutable_eth_dev_spec()->set_rdma_eq_count(eth_dev_info->eth_spec->rdma_eq_count);
        proto_obj->mutable_eth_dev_spec()->set_rdma_aq_count(eth_dev_info->eth_spec->rdma_aq_count);
        proto_obj->mutable_eth_dev_spec()->set_rdma_pid_count(eth_dev_info->eth_spec->rdma_pid_count);
        proto_obj->mutable_eth_dev_spec()->set_barmap_size(eth_dev_info->eth_spec->barmap_size);

        // convert uplink number to old format
        if (eth_dev_info->eth_spec->uplink_port_num == 285278209) {
            proto_obj->mutable_eth_dev_spec()->set_uplink_port_num(1);
        } else if (eth_dev_info->eth_spec->uplink_port_num == 285343745) {
            proto_obj->mutable_eth_dev_spec()->set_uplink_port_num(5);
        } else if (eth_dev_info->eth_spec->uplink_port_num == 285409281) {
            proto_obj->mutable_eth_dev_spec()->set_uplink_port_num(9);
        }

        g_nicmgr_delphic->sdk()->SetObject(proto_obj);

        HAL_TRACE_DEBUG("committing  to Delphi eth dev {}", eth_dev_info->eth_spec->name);
    }

    return;
}

static void
ethdevinfo_delphi_clear (void)
{
    std::vector <delphi::objects::EthDeviceInfoPtr> ethdev_list =
        delphi::objects::EthDeviceInfo::List(g_nicmgr_delphic->sdk());

    for (vector<delphi::objects::EthDeviceInfoPtr>::iterator obj=ethdev_list.begin();
         obj != ethdev_list.end(); ++obj) {
        g_nicmgr_delphic->sdk()->DeleteObject(*obj);
    }

    HAL_TRACE_DEBUG("Cleared objects of EthDeviceInfo from delphi");
}

// delphic event handler
static void
nicmgr_delphic_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    nicmgr_delphic_msg_t *delphic_msg = (nicmgr_delphic_msg_t*)msg->data();
    nicmgr_delphic_msg_id_t msg_id = delphic_msg->msg_id;

    HAL_TRACE_DEBUG("HAL delphi thread received nicmgr delphic event {}", msg_id);

    switch(msg_id) {
    case NICMGR_DELPHIC_MSG_SET_UPG_ETHDEVINFO: {
        ethdevinfo_delphi_save();
        break;
    }
    case NICMGR_DELPHIC_MSG_CLR_UPG_ETHDEVINFO: {
        ethdevinfo_delphi_clear();
        break;
    }
    case NICMGR_DELPHIC_MSG_SET_UPG_UPLINKINFO: {
        uplinkinfo_delphi_save();
        break;
    }
    case NICMGR_DELPHIC_MSG_CLR_UPG_UPLINKINFO: {
        uplinkinfo_delphi_clear();
        break;
    }
    default:
        HAL_TRACE_ERR("HAL delphi thread received nicmgr invalid msg...");
    }
}

delphi::SdkPtr
nicmgr_delphic_sdk()
{
    return g_nicmgr_delphic->sdk();
}

// init_eth_objects mounts eth_dev objects
void init_eth_objects(delphi::SdkPtr sdk) {
    delphi::objects::EthDeviceInfo::Mount(sdk, delphi::ReadWriteMode);
    delphi::objects::UplinkInfo::Mount(sdk, delphi::ReadWriteMode);
}

// init Nicmgr in Delphic Thread
Status
nicmgr_delphic_init (delphi::SdkPtr sdk) {
    g_nicmgr_delphic = make_shared<NicMgrDelphic>(sdk);
    HAL_TRACE_DEBUG("Delphi: Entering nicmgr delphi init...");

    init_eth_objects(sdk);

    HAL_TRACE_DEBUG("Delphi: init nicmgr delphis...");
    // subscribe for events from nicmgr thread
    sdk::ipc::reg_request_handler(event_id_t::EVENT_ID_NICMGR_DELPHIC,
                                  nicmgr_delphic_event_handler, NULL);
    HAL_TRACE_DEBUG("Delphi: Subscribed for nicmgr events...");

    return Status::OK;
}

}    // namespace svc
}    // namespace hal

