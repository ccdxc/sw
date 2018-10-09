//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "upgrade.hpp"
#include "delphi.hpp"
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"

using nicmgr::EthDeviceInfo;
using nicmgr::MacAddrVlans;
using nicmgr::MacAddrs;
using nicmgr::Vlans;
using nicmgr::LifInfo;
using nicmgr::QstateAddr;

// will put it in eth_dev.cc later.
Eth::DevObjSave() {
    delphi::objects::EthDeviceInfoPtr eth_dev_obj;

    eth_dev_obj->set_key(spec->dev_uuid);
    eth_dev_obj->mutable_lif()->set_hw_lif_id(info.hw_lif_id);
    for (int i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto qstate_addr = eth_dev_obj->mutable_lif()->add_qstate_addr();
        qstate_addr->set_qstate_addr(info.qstate_addr[i]);
    }
    eth_dev_obj->set_rss_type(rss_type);
    eth_dev_obj->set_rss_key(rss_key);
    eth_dev_obj->set_rss_indir(rss_indir);
    for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {
        vlan = it->second;
        auto vlans = eth_dev_obj->add_vlans();
        vlans->set_vlan(vlan);
    }
    for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend(); it++) {
        mac_addr = it->second;
        auto mac_addresses = eth_dev_obj->add_mac_addrs();
        mac_addresses->set_mac_addr(mac_addr);
    }
    sdk_->SetObject(eth_dev_obj);
}

namespace nicmgr {

namespace upgrade {

// Constructor method
nicmgr_upg_hndlr::nicmgr_upg_hndlr()
{
}

// Perform compat check
HdlrResp
nicmgr_upg_hndlr::CompatCheckHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Bring link-down
HdlrResp
nicmgr_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Bring data plane down
HdlrResp
nicmgr_upg_hndlr::DataplaneDowntimeStartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Post-binary restart handling
HdlrResp
nicmgr_upg_hndlr::PostRestartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Bring link-up
HdlrResp
nicmgr_upg_hndlr::LinkUpHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Is system ready
HdlrResp
nicmgr_upg_hndlr::IsSystemReadyHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Handle upgrade success
void
nicmgr_upg_hndlr::SuccessHandler(UpgCtx& upgCtx)
{
}

// Handle upgrade failure
void
nicmgr_upg_hndlr::FailedHandler(UpgCtx& upgCtx)
{
}

// Handle upgrade state cleanup
HdlrResp
nicmgr_upg_hndlr::CleanupHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Handle upgrade abort
void
nicmgr_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
}

} // namespace upgrade

} // namespace nicmgr
