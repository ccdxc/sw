//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "gen/proto/nicmgr/nicmgr.pb.h"

#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#include "upgrade.hpp"

using namespace upgrade;
using namespace nicmgr;

extern DeviceManager *devmgr;
extern char* nicmgr_upgrade_state_file;
extern char* nicmgr_rollback_state_file;

evutil_timer ServiceTimer;
UpgradeEvent UpgEvent;
UpgradeState ExpectedState;
uint32_t ServiceTimeout;
bool MoveToNextState;
bool SendAppResp;
bool IsUpgFailed = false;

namespace nicmgr {

static int
writefile(const char *file, const void *buf, const size_t bufsz)
{
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        goto error_out;
    }
    if (fwrite(buf, bufsz, 1, fp) != 1) {
        goto error_out;
    }
    if (fclose(fp) == EOF) {
        fp = NULL;
        goto error_out;
    }
    return bufsz;

 error_out:
    if (fp) (void)fclose(fp);
    (void)unlink(file);
    return -1;
}

// Constructor method
nicmgr_upg_hndlr::nicmgr_upg_hndlr()
{
}

// Perform compat check
HdlrResp
nicmgr_upg_hndlr::CompatCheckHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    IsUpgFailed = false;

    NIC_LOG_INFO("Upgrade: CompatCheck");

    if (!devmgr->UpgradeCompatCheck())
        resp.resp=FAIL;

    return resp;
}

HdlrResp
nicmgr_upg_hndlr::ProcessQuiesceHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: ProcessQuiesce");

    return resp;
}

// Bring link-down
HdlrResp
nicmgr_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=INPROGRESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: LinkDown");

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_FUNC_DEBUG("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are quiesced or not ...");

    ExpectedState = DEVICES_QUIESCED_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = false;
    SendAppResp = true;
    evutil_timer_start(EV_DEFAULT_ &ServiceTimer, nicmgr_upg_hndlr::upg_timer_func, this, 0.0, 0.1);

    return resp;
}

// Post-binary restart handling
HdlrResp
nicmgr_upg_hndlr::PostRestartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: PostRestart");

    return resp;
}

// Bring link-up
HdlrResp
nicmgr_upg_hndlr::LinkUpHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: LinkUp");

    return resp;
}

// Handle upgrade success
void
nicmgr_upg_hndlr::SuccessHandler(UpgCtx& upgCtx)
{
    NIC_LOG_INFO("Upgrade: Success");

    NIC_FUNC_DEBUG("Deleting all objects of EthDevInfo from Delphi");
    // walk all objects and delete them
    vector<delphi::objects::EthDeviceInfoPtr> objlist = delphi::objects::EthDeviceInfo::List(g_nicmgr_svc->sdk());
    for (vector<delphi::objects::EthDeviceInfoPtr>::iterator obj=objlist.begin(); obj !=objlist.end(); ++obj) {
        g_nicmgr_svc->sdk()->DeleteObject(*obj);
    }
    
    unlink(nicmgr_upgrade_state_file);
    unlink(nicmgr_rollback_state_file);
}

// Handle upgrade failure
HdlrResp
nicmgr_upg_hndlr::FailedHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    IsUpgFailed = true;

    NIC_LOG_INFO("Upgrade: Failed");

    // walk all objects and delete them
    NIC_FUNC_DEBUG("Deleting all objects of EthDevInfo from Delphi");
    vector<delphi::objects::EthDeviceInfoPtr> EthDevList = delphi::objects::EthDeviceInfo::List(g_nicmgr_svc->sdk());
    for (vector<delphi::objects::EthDeviceInfoPtr>::iterator obj = EthDevList.begin(); obj != EthDevList.end(); ++obj) {
        g_nicmgr_svc->sdk()->DeleteObject(*obj);
    }

    NIC_FUNC_DEBUG("Deleting all objects of UplinkInfo from Delphi");
    vector<delphi::objects::UplinkInfoPtr> UplinkList = delphi::objects::UplinkInfo::List(g_nicmgr_svc->sdk());
    for (vector<delphi::objects::UplinkInfoPtr>::iterator obj = UplinkList.begin(); obj != UplinkList.end(); ++obj) {
        g_nicmgr_svc->sdk()->DeleteObject(*obj);
    }

    unlink(nicmgr_upgrade_state_file); 

    if (UpgCtxApi::UpgCtxGetUpgState(upgCtx) == CompatCheck) {
        NIC_FUNC_DEBUG("FW upgrade failed during compat check");
        return resp;
    }

    // If fw upgrade failed after compat_check then we will rollback to running fw
    writefile(nicmgr_rollback_state_file, "in progress", 11);
    resp.resp = INPROGRESS;

    // we will run through all the state machine for nicmgr upgrade if somehow upgrade failed
    NIC_FUNC_DEBUG("Sending LinkDown event to eth drivers");

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_FUNC_DEBUG("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Waiting for LinkDown event response from eth drivers");

    ExpectedState = DEVICES_QUIESCED_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = true;
    SendAppResp = false;
    evutil_timer_start(EV_DEFAULT_ &ServiceTimer, nicmgr_upg_hndlr::upg_timer_func, this, 0.0, 0.1);

    return resp;
}

// Handle upgrade abort
void
nicmgr_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
    NIC_LOG_INFO("Upgrade: Abort");
}

void SaveUplinkInfo(uplink_t *up, delphi::objects::UplinkInfoPtr proto_obj)
{
    proto_obj->set_key(up->id);
    proto_obj->set_id(up->id);
    proto_obj->set_port(up->port);
    proto_obj->set_is_oob(up->is_oob);

    g_nicmgr_svc->sdk()->SetObject(proto_obj);
}

void
SaveEthDevInfo(struct EthDevInfo *eth_dev_info, delphi::objects::EthDeviceInfoPtr proto_obj)
{
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
    proto_obj->mutable_eth_dev_spec()->set_uplink_port_num(eth_dev_info->eth_spec->uplink_port_num);
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

    g_nicmgr_svc->sdk()->SetObject(proto_obj);
    return;
}

HdlrResp
nicmgr_upg_hndlr::SaveStateHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: SaveState");

    std::vector <struct EthDevInfo*> dev_info;
    std::map<uint32_t, uplink_t*> up_links = devmgr->GetUplinks();

    dev_info = devmgr->GetEthDevStateInfo();
    NIC_FUNC_DEBUG("Saving {} objects of EthDevInfo to delphi", dev_info.size());
    //for each element in dev_info convert it to protobuf and then setobject to delphi
    for (uint32_t eth_dev_idx = 0; eth_dev_idx < dev_info.size(); eth_dev_idx++) {
        delphi::objects::EthDeviceInfoPtr eth_dev_info = make_shared<delphi::objects::EthDeviceInfo>();
        NIC_FUNC_DEBUG("Saving EthDevInfo for eth_dev_idx {}", eth_dev_idx);
        SaveEthDevInfo(dev_info[eth_dev_idx], eth_dev_info);
    }

    NIC_FUNC_DEBUG("Saving {} objects of UplinkInfo to delphi", up_links.size());
    for (auto it = up_links.begin(); it != up_links.end(); it++) {
        uplink_t *up = it->second;
        delphi::objects::UplinkInfoPtr uplink_info = make_shared<delphi::objects::UplinkInfo>();
        NIC_FUNC_DEBUG("Saving Uplink info for uplink id: {}", up->id);
        SaveUplinkInfo(up, uplink_info);
    }
    std::vector <delphi::objects::UplinkInfoPtr> UplinkProtoObjs = delphi::objects::UplinkInfo::List(g_nicmgr_svc->sdk());
    NIC_FUNC_DEBUG("Retrieved {} UplinkProto objects from Delphi", UplinkProtoObjs.size());
    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::PostLinkUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: PostLinkUp");

    return resp;
}

string UpgStateToStr(UpgradeState state)
{
    switch (state) {
        case (DEVICES_ACTIVE_STATE):
            return "DEVICES_ACTIVE_STATE";
        case (DEVICES_QUIESCED_STATE):
            return "DEVICES_QUIESCED_STATE";
        case (DEVICES_RESET_STATE):
            return "DEVICES_RESET_STATE";
        default:
            return "UNKNOWN_STATE";
    }
}

void
nicmgr_upg_hndlr::upg_timer_func(void *obj)
{
    static uint32_t max_retry = 0;
    int ret = 0;

    if ((devmgr->GetUpgradeState() < ExpectedState) && (++max_retry < ServiceTimeout)) {
        return;
    }
    else {
        if (max_retry < ServiceTimeout)
            NIC_FUNC_DEBUG("All devices are in {} state after {} retry", UpgStateToStr(devmgr->GetUpgradeState()), max_retry);
        else
            NIC_LOG_ERR("Timeout occurred while waiting for all devices to go in {} state", ExpectedState);
    }

    if (MoveToNextState) {

        if (ExpectedState == DEVICES_QUIESCED_STATE) {
            UpgEvent = UPG_EVENT_DEVICE_RESET;
            ExpectedState = DEVICES_RESET_STATE;
            devmgr->HandleUpgradeEvent(UpgEvent);
            MoveToNextState = false;
            SendAppResp = true;
            max_retry = 0;

        }
        else if (ExpectedState == DEVICES_ACTIVE_STATE) {
            UpgEvent = UPG_EVENT_QUIESCE;
            ExpectedState = DEVICES_QUIESCED_STATE;
            devmgr->HandleUpgradeEvent(UpgEvent);
            MoveToNextState = true;
            SendAppResp = false;
            max_retry = 0;
        }
        else {
            NIC_LOG_ERR("Undefined device's next state from current state: {}", UpgStateToStr(devmgr->GetUpgradeState()));
        }

        NIC_FUNC_DEBUG("Moving upgrade state machine to next level {}", UpgStateToStr(ExpectedState));
        return;
    }

    if (SendAppResp) {
        if (ExpectedState == DEVICES_RESET_STATE) {
            if (!IsUpgFailed) {
                ret = writefile(nicmgr_upgrade_state_file, "in progress", 11);
                if (ret == -1)
                    return;
            }
        }

        NIC_FUNC_DEBUG("Sending App Response to upgrade manager");

        if (max_retry >= ServiceTimeout) {
            g_nicmgr_svc->upgsdk()->SendAppRespFail("Timeout occurred");
        }
        else {
            g_nicmgr_svc->upgsdk()->SendAppRespSuccess();
        }
        evutil_timer_stop(EV_DEFAULT_ &ServiceTimer);

        return;
    }
}

HdlrResp
nicmgr_upg_hndlr::HostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=INPROGRESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: HostDown");

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_DEVICE_RESET)) {
        NIC_FUNC_DEBUG("UPG_EVENT_DEVICE_RESET event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are in reset or not ...");
    ExpectedState = DEVICES_RESET_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = false;
    SendAppResp = true;
    evutil_timer_start(EV_DEFAULT_ &ServiceTimer, nicmgr_upg_hndlr::upg_timer_func, this, 0.0, 0.1);

    return resp;
}

HdlrResp
nicmgr_upg_hndlr::PostHostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: PostHostDown");

    return resp;
}

HdlrResp
nicmgr_upg_hndlr::HostUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: HostUp");

    return resp;
}


} // namespace nicmgr
