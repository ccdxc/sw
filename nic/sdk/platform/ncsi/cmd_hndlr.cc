/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include <cstdio>
#include <cstring>
#include <arpa/inet.h>

#include "cmd_hndlr.h"
#include "lib/logger/logger.hpp"

#define NCSI_CMD_BEGIN_BANNER() \
{\
    SDK_TRACE_INFO("-------------- NCSI Cmd --------------"); \
    SDK_TRACE_INFO("cmd: %s", __FUNCTION__); \
}

#define NCSI_CMD_END_BANNER() \
{ \
    SDK_TRACE_INFO("cmd: %s, response code: 0x%x, reason code: 0x%x", \
            __FUNCTION__, ntohs(resp.rsp.code), ntohs(resp.rsp.reason)); \
    SDK_TRACE_INFO("cmd response: %s, status: %d", __FUNCTION__, ret); \
    SDK_TRACE_INFO("-------------- NCSI Cmd End --------------"); \
}

namespace sdk {
namespace platform {
namespace ncsi {

uint64_t CmdHndler::mac_addr_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_MIXED_MAC_FILTER_COUNT];
uint16_t CmdHndler::vlan_filter_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_VLAN_FILTER_COUNT];
StateMachine * CmdHndler::StateM[NCSI_CAP_CHANNEL_COUNT];
NcsiParamDb CmdHndler::NcsiDb[NCSI_CAP_CHANNEL_COUNT];

struct GetCapRespPkt get_cap_resp;

CmdHndler::CmdHndler(std::shared_ptr<IpcService> IpcObj, transport *XportObj) {
    memset(&stats, 0, sizeof(struct NcsiStats));
    memset(CmdTable, 0, sizeof(CmdTable));
    memset(mac_addr_list, 0, sizeof(mac_addr_list));
    memset(vlan_filter_list, 0, sizeof(vlan_filter_list));
    memset(NcsiDb, 0, sizeof(NcsiDb));

    for (uint8_t ncsi_channel = 0; ncsi_channel< NCSI_CAP_CHANNEL_COUNT; ncsi_channel++) {
        StateM[ncsi_channel] = new StateMachine();
    }

    ipc = IpcObj;
    xport = XportObj;

    CmdTable[CMD_CLEAR_INIT_STATE]        = ClearInitState;
    CmdTable[CMD_SELECT_PACKAGE]          = SelectPackage;
    CmdTable[CMD_DESELECT_PACKAGE]        = DeselectPackage;
    CmdTable[CMD_EN_CHAN]                 = EnableChan;
    CmdTable[CMD_DIS_CHAN]                = DisableChan;
    CmdTable[CMD_RESET_CHAN]              = ResetChan;
    CmdTable[CMD_EN_CHAN_NW_TX]           = EnableChanNwTx;
    CmdTable[CMD_DIS_CHAN_NW_TX]          = DisableChanNwTx;
    CmdTable[CMD_SET_LINK]                = SetLink;
    CmdTable[CMD_GET_LINK_STATUS]         = GetLinkStatus;
    CmdTable[CMD_SET_VLAN_FILTER]         = SetVlanFilter;
    CmdTable[CMD_EN_VLAN]                 = EnableVlan;
    CmdTable[CMD_DIS_VLAN]                = DisableVlan;
    CmdTable[CMD_SET_MAC_ADDR]            = SetMacAddr;
    CmdTable[CMD_EN_BCAST_FILTER]         = EnableBcastFilter;
    CmdTable[CMD_DIS_BCAST_FILTER]        = DisableBcastFilter;
    CmdTable[CMD_EN_GLOBAL_MCAST_FILTER]  = EnableGlobalMcastFilter;
    CmdTable[CMD_DIS_GLOBAL_MCAST_FILTER] = DisableGlobalMcastFilter;
    CmdTable[CMD_GET_VER_ID]              = GetVersionId;
    CmdTable[CMD_GET_CAP]                 = GetCapabilities;
    CmdTable[CMD_GET_PARAMS]              = GetParams;
    CmdTable[CMD_GET_NIC_STATS]           = GetNicPktStats;
    CmdTable[CMD_GET_NCSI_STATS]          = GetNcsiStats;
    CmdTable[CMD_GET_NCSI_PASSTHRU_STATS] = GetNcsiPassthruStats;
    CmdTable[CMD_GET_PACKAGE_STATUS]      = GetPackageStatus;
    CmdTable[CMD_GET_PACKAGE_UUID]        = GetPackageUUID;

    get_cap_resp.cap = (NCSI_CAP_HW_ARB | NCSI_CAP_HOST_NC_DRV_STATUS | NCSI_CAP_NC_TO_MC_FLOW_CTRL | NCSI_CAP_MC_TO_NC_FLOW_CTRL | NCSI_CAP_ALL_MCAST_ADDR_SUPPORT | NCSI_CAP_HW_ARB_IMPL_STATUS);
    get_cap_resp.bc_cap = (NCSI_CAP_BCAST_FILTER_ARP | NCSI_CAP_BCAST_FILTER_DHCP_CLIENT | NCSI_CAP_BCAST_FILTER_DHCP_SERVER | NCSI_CAP_BCAST_FILTER_NETBIOS);
    get_cap_resp.mc_cap = (NCSI_CAP_MCAST_IPV6_NEIGH_ADV | NCSI_CAP_MCAST_IPV6_ROUTER_ADV | NCSI_CAP_MCAST_DHCPV6_RELAY | NCSI_CAP_MCAST_DHCPV6_MCAST_SERVER_TO_CLIENT | NCSI_CAP_MCAST_IPV6_MLD | NCSI_CAP_MCAST_IPV6_NEIGH_SOL);
    get_cap_resp.buf_cap = NCSI_CAP_BUFFERRING;
    get_cap_resp.aen_cap = (NCSI_CAP_AEN_CTRL_LINK_STATUS_CHANGE | NCSI_CAP_AEN_CTRL_CONFIG_REQUIRED | NCSI_CAP_AEN_CTRL_HOST_NC_DRV_STATUS_CHANGE | NCSI_CAP_AEN_CTRL_OEM_SPECIFIC);
    get_cap_resp.vlan_cnt = NCSI_CAP_VLAN_FILTER_COUNT;
    get_cap_resp.mc_cnt = NCSI_CAP_MCAST_FILTER_COUNT;
    get_cap_resp.uc_cnt = NCSI_CAP_UCAST_FILTER_COUNT;
    get_cap_resp.mixed_cnt = NCSI_CAP_MIXED_MAC_FILTER_COUNT;
    get_cap_resp.vlan_mode = NCSI_CAP_VLAN_MODE_SUPPORT;
    get_cap_resp.channel_cnt = NCSI_CAP_CHANNEL_COUNT;
}

int CmdHndler::SendNcsiCmdResponse(const void *buf, ssize_t sz)
{
    ssize_t ret;

    ret = xport->SendPkt(buf, sz);

    if (ret < 0) {
        SDK_TRACE_ERR("%s: sending cmd response failed with error code: %d",
                __FUNCTION__, ret);
        return -1;
    }
    else {
        SDK_TRACE_INFO("%s: Response sent", __FUNCTION__);
    }

    return 0;
}

int CmdHndler::ConfigVlanFilter(uint16_t vlan, uint32_t port, bool enable)
{
    VlanFilterMsg vlan_msg;
 
    vlan_msg.filter_id = port * NCSI_CAP_VLAN_FILTER_COUNT;
    vlan_msg.port = port;
    vlan_msg.vlan_id = vlan;
    vlan_msg.enable = enable;
    
    return this->ipc->PostMsg(vlan_msg);
}

int CmdHndler::ConfigMacFilter(uint64_t mac_addr, uint32_t port, uint8_t type,
        bool enable)
{
    MacFilterMsg mac_filter_msg;
 
    mac_filter_msg.filter_id = port * NCSI_CAP_MIXED_MAC_FILTER_COUNT;
    mac_filter_msg.port = port;
    mac_filter_msg.mac_addr = mac_addr;
    mac_filter_msg.addr_type = type;
    mac_filter_msg.enable = enable;
    
    return this->ipc->PostMsg(mac_filter_msg);
}

int CmdHndler::ConfigVlanMode(uint8_t vlan_mode, uint32_t port, bool enable)
{
    VlanModeMsg vlan_mode_msg;
 
    vlan_mode_msg.filter_id = port;
    vlan_mode_msg.port = port;
    vlan_mode_msg.mode = vlan_mode;
    vlan_mode_msg.enable = enable;
    
    return this->ipc->PostMsg(vlan_mode_msg);
}
void CmdHndler::SetVlanFilter(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct SetVlanFilterCmdPkt *cmd = (SetVlanFilterCmdPkt *)cmd_pkt;
    uint32_t vlan_id = (ntohs(cmd->vlan) & 0xFFF); //ignore user pri/cfi bits
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();

    SDK_TRACE_INFO("ncsi_channel: 0x%x vlan_id: 0x%x, enable: 0x%x ", 
            cmd->cmd.NcsiHdr.channel, vlan_id, cmd->enable & 0x1);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_VLAN_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check vlan validity e.g. vlan can't be 0
    if (vlan_id) {
        ret = hndlr->ConfigVlanFilter(vlan_id, cmd->cmd.NcsiHdr.channel, 
                (cmd->enable & 0x1) ? true:false);
        if (ret) {
            SDK_TRACE_ERR("Failed to set vlan filter");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
        }
    }
    else
    {
        SDK_TRACE_ERR("vlan_id: 0 is not valid");
        resp.rsp.reason = htons(NCSI_REASON_INVLD_VLAN);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SET_VLAN_FILTER);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::ClearInitState(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_CLEAR_INIT_STATE);

    if (sm_ret) {
        if (sm_ret == INVALID) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INVALID_CMD_ERR);
            SDK_TRACE_ERR("cmd: %x failed as its invalid cmd with current ncsi state: 0x%x", cmd, StateM[cmd->cmd.NcsiHdr.channel]->GetCurState());

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement clear initial state command here

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_CLEAR_INIT_STATE);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::SelectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SELECT_PACKAGE);

    if (sm_ret) {
        if (sm_ret == INVALID) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INVALID_CMD_ERR);
            SDK_TRACE_ERR("cmd: %x failed as its invalid cmd with current ncsi state: 0x%x", cmd, StateM[cmd->cmd.NcsiHdr.channel]->GetCurState());

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Enable the filters which were applied on channel

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SELECT_PACKAGE);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::DeselectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DESELECT_PACKAGE);

    if (sm_ret) {
        if (sm_ret == INVALID) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INVALID_CMD_ERR);
            SDK_TRACE_ERR("cmd: %x failed as its invalid cmd with current ncsi state: 0x%x", cmd, StateM[cmd->cmd.NcsiHdr.channel]->GetCurState());

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement deselect package command here

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DESELECT_PACKAGE);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::ChannelEnable(const void *cmd_pkt, ssize_t cmd_sz, bool enable)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct EnableChanMsg enable_ch_msg;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x, enable: 0x%x", 
            cmd->cmd.NcsiHdr.channel, enable);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_VLAN_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    enable_ch_msg.enable = enable;
    enable_ch_msg.port = cmd->cmd.NcsiHdr.channel;
    enable_ch_msg.filter_id = cmd->cmd.NcsiHdr.channel;

    ret = ipc->PostMsg(enable_ch_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to enable/disable channel");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(
            enable ? CMD_EN_CHAN : CMD_DIS_CHAN);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    
    return;
}

void CmdHndler::EnableChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    CmdHndler *hndlr = (CmdHndler *)obj;

    hndlr->ChannelEnable(cmd_pkt, cmd_sz, true);

    return;
}

void CmdHndler::DisableChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    CmdHndler *hndlr = (CmdHndler *)obj;

    /* we are ignoring Allow Link Down (ALD) in DisableChannel command */
    hndlr->ChannelEnable(cmd_pkt, cmd_sz, false);

    return;
}

void CmdHndler::ResetChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct ResetChanMsg reset_ch_msg;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct ResetChanCmdPkt *cmd = (ResetChanCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_RESET_CHAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    reset_ch_msg.reset = true;
    reset_ch_msg.port = cmd->cmd.NcsiHdr.channel;
    reset_ch_msg.filter_id = cmd->cmd.NcsiHdr.channel;

    ret = hndlr->ipc->PostMsg(reset_ch_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to reset channel");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_RESET_CHAN);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    
    return;

}

void CmdHndler::ChannelEnableNwTx(const void *cmd_pkt, ssize_t cmd_sz, 
        bool enable)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct EnableChanTxMsg enable_ch_tx_msg;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x, enable: 0x%x", 
            cmd->cmd.NcsiHdr.channel, enable);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(enable ? CMD_EN_CHAN_NW_TX : CMD_DIS_CHAN_NW_TX);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    enable_ch_tx_msg.enable = enable;
    enable_ch_tx_msg.port = cmd->cmd.NcsiHdr.channel;
    enable_ch_tx_msg.filter_id = cmd->cmd.NcsiHdr.channel;

    ret = ipc->PostMsg(enable_ch_tx_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to enable/disable channel nw tx");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(
            enable ? CMD_EN_CHAN_NW_TX : CMD_DIS_CHAN_NW_TX);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    
    return;
}
void CmdHndler::EnableChanNwTx(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    CmdHndler *hndlr = (CmdHndler *)obj;

    hndlr->ChannelEnableNwTx(cmd_pkt, cmd_sz, true);

    return;
}

void CmdHndler::DisableChanNwTx(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    CmdHndler *hndlr = (CmdHndler *)obj;

    hndlr->ChannelEnableNwTx(cmd_pkt, cmd_sz, false);

    return;
}

void CmdHndler::SetLink(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct SetLinkMsg set_link_msg;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct SetLinkCmdPkt *cmd = (SetLinkCmdPkt *)cmd_pkt;
    uint32_t oem_field_valid = (cmd->mode) & (1 << 11);
    uint8_t set_link = cmd->oem_mode & 0x1;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x oem_field_valid: 0x%x, set_link: 0x%x ", 
            cmd->cmd.NcsiHdr.channel, oem_field_valid, set_link);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_LINK);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    if (oem_field_valid) {
        set_link_msg.link_up = set_link;
        set_link_msg.port = cmd->cmd.NcsiHdr.channel;
        set_link_msg.filter_id = cmd->cmd.NcsiHdr.channel;

        ret = hndlr->ipc->PostMsg(set_link_msg);
        if (ret) {
            SDK_TRACE_ERR("Failed to set link");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
        }
    }
    else
    {
        SDK_TRACE_ERR("Only OEM specific link settings are allowed");
        resp.rsp.reason = htons(NCSI_REASON_INVALID_PARAM);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SET_LINK);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::GetLinkStatus(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetLinkStatusRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_LINK_STATUS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }


    //TODO: Implement the get link state command

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_LINK_STATUS);
    resp.rsp.NcsiHdr.length = NCSI_GET_LINK_STATUS_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::EnableVlan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct EnVlanCmdPkt *cmd = (EnVlanCmdPkt *)cmd_pkt;
    uint32_t vlan_mode = cmd->mode;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x vlan_mode: 0x%x", 
            cmd->cmd.NcsiHdr.channel, vlan_mode);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_EN_VLAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check vlan mode validity. we support only vlan mode 1 and 2
    if (vlan_mode > 0 && vlan_mode < 2) {
        ret = hndlr->ConfigVlanMode(vlan_mode, cmd->cmd.NcsiHdr.channel, true);
        if (ret) {
            SDK_TRACE_ERR("Failed to set vlan Mode");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);

            goto error_out;
        }
    }
    else
    {
        SDK_TRACE_ERR("vlan_mode: 0x%x is not supported", vlan_mode);
        resp.rsp.reason = htons(NCSI_REASON_INVALID_PARAM);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);

        goto error_out;
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_VLAN);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::DisableVlan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DIS_VLAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check vlan mode validity. we support only vlan mode 1 and 2
    ret = hndlr->ConfigVlanMode( /*don't care*/ 0, cmd->cmd.NcsiHdr.channel,
            false);
    if (ret) {
        SDK_TRACE_ERR("Failed to set vlan Mode");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_VLAN);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::SetMacAddr(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct SetMacAddrCmdPkt *cmd = (SetMacAddrCmdPkt *)cmd_pkt;
    uint8_t mac_addr_type = ((cmd->at_e & 0xE0) >> 5);
    uint64_t mac_addr = *((uint64_t *)cmd->mac);
    bool enable = (cmd->at_e & 0x1) ? true:false;
    uint8_t mac_filter_num = cmd->index;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
    
    NCSI_CMD_BEGIN_BANNER();

    SDK_TRACE_INFO("ncsi_channel: 0x%x, mac_addr: %x:%x:%x:%x:%x:%x, "
            "mac_addr_type: 0x%x, enable: 0x%x ", cmd->cmd.NcsiHdr.channel, 
            cmd->mac[5], cmd->mac[4], cmd->mac[3], cmd->mac[2], cmd->mac[1], 
            cmd->mac[0], mac_addr_type, enable);

    //valid mac address type are 0(Ucast) & 1(Mcast) only
    if (mac_addr_type > 1) {
        SDK_TRACE_ERR("mac_addr_type: 0x%x is not valid", mac_addr_type);
        resp.rsp.reason = htons(NCSI_REASON_INVLD_MAC_ADDR);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

    if(mac_filter_num > 8) {
        SDK_TRACE_ERR("mac_filter_num: 0x%x is not out of range", 
                mac_filter_num);
        resp.rsp.reason = htons(NCSI_REASON_INVLD_MAC_ADDR);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_MAC_ADDR);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check mac addr validity e.g. mac addr can't be 0
    if (mac_addr) {
        if (mac_addr_list[cmd->cmd.NcsiHdr.channel][mac_filter_num]) {

            ret = hndlr->ConfigMacFilter(mac_addr_list[cmd->cmd.NcsiHdr.channel][mac_filter_num], 
                    cmd->cmd.NcsiHdr.channel, mac_addr_type, false);

            if (ret) {
                SDK_TRACE_ERR("Failed to remove old mac filter");
                resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
                resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);

                goto error_out;
            }
        }

        ret = hndlr->ConfigMacFilter(mac_addr, cmd->cmd.NcsiHdr.channel, 
                mac_addr_type, enable);

        if (ret) {
            SDK_TRACE_ERR("Failed to set mac filter");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            
            goto error_out;
        }
        else {
            mac_addr_list[cmd->cmd.NcsiHdr.channel][mac_filter_num] = mac_addr;
        }
    }
    else
    {
        SDK_TRACE_ERR("All zero mac_addr is not supported");
        resp.rsp.reason = htons(NCSI_REASON_INVLD_MAC_ADDR);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SET_MAC_ADDR);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::EnableBcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    struct EnableBcastFilterMsg bcast_filter_msg;

    const struct EnBcastFilterCmdPkt *cmd = (EnBcastFilterCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_EN_BCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    bcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.enable_arp = !!(cmd->mode & NCSI_CAP_BCAST_FILTER_ARP);
    bcast_filter_msg.enable_dhcp_client = !!(cmd->mode & NCSI_CAP_BCAST_FILTER_DHCP_CLIENT);
    bcast_filter_msg.enable_dhcp_server = !!(cmd->mode & NCSI_CAP_BCAST_FILTER_DHCP_SERVER);
    bcast_filter_msg.enable_netbios = !!(cmd->mode & NCSI_CAP_BCAST_FILTER_NETBIOS);

    ret = hndlr->ipc->PostMsg(bcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to program bcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_BCAST_FILTER);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::DisableBcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    EnableBcastFilterMsg bcast_filter_msg;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DIS_BCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    bcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.enable_arp = false;
    bcast_filter_msg.enable_dhcp_client = false;
    bcast_filter_msg.enable_dhcp_server = false;
    bcast_filter_msg.enable_netbios = false;

    ret = hndlr->ipc->PostMsg(bcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to disable bcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_BCAST_FILTER);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::EnableGlobalMcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    struct EnableGlobalMcastFilterMsg mcast_filter_msg;
    const struct EnGlobalMcastFilterCmdPkt *cmd = 
        (EnGlobalMcastFilterCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_EN_GLOBAL_MCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    mcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.enable_ipv6_neigh_adv = !!(cmd->mode & NCSI_CAP_MCAST_IPV6_NEIGH_ADV);
    mcast_filter_msg.enable_ipv6_router_adv = !!(cmd->mode & NCSI_CAP_MCAST_IPV6_ROUTER_ADV);
    mcast_filter_msg.enable_dhcpv6_relay = !!(cmd->mode & NCSI_CAP_MCAST_DHCPV6_RELAY);
    mcast_filter_msg.enable_dhcpv6_mcast = !!(cmd->mode & NCSI_CAP_MCAST_DHCPV6_MCAST_SERVER_TO_CLIENT);
    mcast_filter_msg.enable_ipv6_mld = !!(cmd->mode & NCSI_CAP_MCAST_IPV6_MLD);
    mcast_filter_msg.enable_ipv6_neigh_sol = !!(cmd->mode & NCSI_CAP_MCAST_IPV6_NEIGH_SOL);

    ret = hndlr->ipc->PostMsg(mcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to program mcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_GLOBAL_MCAST_FILTER);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::DisableGlobalMcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct EnableGlobalMcastFilterMsg mcast_filter_msg;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DIS_GLOBAL_MCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    mcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.enable_ipv6_neigh_adv = false;
    mcast_filter_msg.enable_ipv6_router_adv = false;
    mcast_filter_msg.enable_dhcpv6_relay = false;
    mcast_filter_msg.enable_dhcpv6_mcast = false;
    mcast_filter_msg.enable_ipv6_mld = false;
    mcast_filter_msg.enable_ipv6_neigh_sol = false;

    ret = hndlr->ipc->PostMsg(mcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to dsiable mcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_GLOBAL_MCAST_FILTER);
    resp.rsp.NcsiHdr.length = NCSI_FIXED_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetVersionId(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    struct GetVersionIdRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    //FIXME: As of now we are ignoring the ncsi state machine for this cmd
    
    //TODO: Implement the logic here

    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_VER_ID);
    resp.rsp.NcsiHdr.length = NCSI_GET_VER_ID_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetCapabilities(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetCapRespPkt& resp = get_cap_resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_CAP);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_CAP);
    resp.rsp.NcsiHdr.length = NCSI_GET_CAP_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetParams(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetParamRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_PARAMS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement the logic here

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_PARAMS);
    resp.rsp.NcsiHdr.length = NCSI_GET_PARAM_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetNicPktStats(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetNicStatsRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_NIC_STATS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement the logic here

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_NIC_STATS);
    resp.rsp.NcsiHdr.length = NCSI_GET_NIC_STATS_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetNcsiStats(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetNCSIStatsRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_NCSI_STATS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement the get link state command

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_NCSI_STATS);
    resp.rsp.NcsiHdr.length = NCSI_GET_NCSI_STATS_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetNcsiPassthruStats(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetPassThruStatsRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_NCSI_PASSTHRU_STATS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement the logic here

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_NCSI_PASSTHRU_STATS);
    resp.rsp.NcsiHdr.length = NCSI_GET_PASSTHRU_STATS_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetPackageStatus(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    struct GetPkgStatusRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    //FIXME: As of now ignoring the state machine for this cmd
    //TODO: Implement the logic here

    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_PACKAGE_STATUS);
    resp.rsp.NcsiHdr.length = NCSI_GET_PKG_STATUS_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetPackageUUID(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    struct GetPkgUUIDRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
 
    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
 
    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    //FIXME: As of now ignoring the state machine for this cmd

    //TODO: Implement the logic here 

    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_PACKAGE_UUID);
    resp.rsp.NcsiHdr.length = NCSI_GET_PKG_UUID_RSP_PAYLOAD_LEN;
    
    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

int CmdHndler::ValidateCmdPkt(const void *pkt, ssize_t sz)
{
    const uint8_t *buf = (uint8_t *)pkt;

    if ( !buf || !sz ) {
        SDK_TRACE_ERR("Zero sized packets cannot be validated");
        return -1;
    }

    if (sz < MIN_802_3_FRAME_SZ) {
        SDK_TRACE_ERR("NCSI packet size (%d) is less than min frame size "
                "for 802.3", sz);
        stats.rx_drop_cnt++;
    }

    if (buf[NCSI_HDR_REV_OFFSET] != SUPPORTED_NCSI_REV) {
        SDK_TRACE_ERR("NCSI Header Rev %d not supported. Expected Rev is %d",
                buf[NCSI_HDR_REV_OFFSET], SUPPORTED_NCSI_REV);
        stats.rx_drop_cnt++;
    }

    stats.valid_cmd_rx_cnt++;

    return 0;
}

int CmdHndler::HandleCmd(const void* pkt, ssize_t sz)
{
    const uint8_t *buf = (uint8_t *)pkt;
    void *rsp = NULL;
    ssize_t rsp_sz = 0;
    uint8_t opcode;

    opcode = buf[NCSI_CMD_OPCODE_OFFSET];

    if (CmdTable[opcode]) {
        SDK_TRACE_INFO("Handling Ncsi command: 0x%x ", opcode);
        CmdTable[opcode](this, pkt, sz);

        if (rsp && rsp_sz) {
            SDK_TRACE_INFO("Sending Ncsi Response for cmd: 0x%x ", opcode);
            
        }
    }
    else {
        SDK_TRACE_ERR("Ncsi command 0x%x is not supported",
                buf[NCSI_CMD_OPCODE_OFFSET]);
        stats.unsup_cmd_rx_cnt++;
    }

    return 0;
}

} // namespace ncsi
} // namespace platform
} // namespace sdk

