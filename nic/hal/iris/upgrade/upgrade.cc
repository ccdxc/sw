//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/system.hpp"
#include "nic/hal/iris/upgrade/upg_ipc.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace upgrade {

std::string empty_str("");

//------------------------------------------------------------------------------
// perform compat check
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::CompatCheckHandler(UpgCtx& upgCtx)
{
    std::string preVer, postVer;
    HdlrResp    rsp;

    ::upgrade::UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, preVer);
    ::upgrade::UpgCtxApi::UpgCtxGetPostUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, postVer);
    HAL_TRACE_DEBUG("[upgrade] Handling compat checks msg ... preVer {} postVer {}", preVer, postVer);

    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handler to bring link(s) down
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::LinkDownHandler (UpgCtx& upgCtx)
{
    sdk_ret_t sdk_ret;
    hal_ret_t ret;
    HdlrResp rsp;

    HAL_TRACE_DEBUG("[upgrade] Handling link down msg ...");

    // send TCP FIN on sessions with local EPs
    if (hal::g_hal_cfg.features != hal::HAL_FEATURE_SET_GFT) {
        ret = session_handle_upgrade();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("[upgrade] Session handle upgrade failed, err {}",
                            ret);
            rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
            goto err;
        }
    }

    // disable all uplink ports
    // - as part of this delphi notifications will be sent out per port
    ret = linkmgr::port_disable(0);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("[upgrade] Port disable failed, err {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }

    // flush PB/TM for all uplinks
    sdk_ret = asicpd_tm_enable_disable_uplink_port(false, TM_PORT_UPLINK_ALL);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("[upgrade] Unable to flush PB for uplinks. err: {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }

    rsp = HdlrResp(::upgrade::SUCCESS, empty_str);
err:
    return rsp;
}

//------------------------------------------------------------------------------
// handle Post link up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostLinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling Post LinkUp msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle Quiesce
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::ProcessQuiesceHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling quiesce msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}


//------------------------------------------------------------------------------
// handle State save
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::SaveStateHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling save state msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle Upgrade failure
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::FailedHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling failed msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle Host down
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::HostDownHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling host down msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle Host up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::HostUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling host up msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle post host down message by shutting of scheduler
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostHostDownHandler(UpgCtx& upgCtx)
{
    sdk_ret_t sdk_ret;
    hal_ret_t ret = HAL_RET_OK;
    HdlrResp  rsp;

    HAL_TRACE_DEBUG("[upgrade] Handling post host down msg ...");

    // disable TX scheduler for all LIFs
    ret = lif_disable_tx_scheduler();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to disable TX scheduler for LIFs. err: {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }

    // quiesece the pipeline
    sdk_ret = asicpd_quiesce_start();
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to quiesce. Failing ...");
        rsp = HdlrResp(::upgrade::INPROGRESS, HAL_RET_ENTRIES_str(ret));
        goto err;
        // return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    // Reset tables
    ret = upgrade_table_reset();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to reset tables. err: {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }
    rsp = HdlrResp(::upgrade::SUCCESS, empty_str);
err:
    return rsp;
}

//------------------------------------------------------------------------------
// take care post restart activities during upgrade
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostRestartHandler(UpgCtx& upgCtx)
{
    std::string preVer, postVer;
    HdlrResp    hal_rsp, rsp;
    const char *dev_conf_a = "1";
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("[upgrade] Handling host up msg ...");

    ::upgrade::UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx,
                                                      ::upgrade::DEVCONFVER,
                                                      preVer);
    ::upgrade::UpgCtxApi::UpgCtxGetPostUpgTableVersion(upgCtx,
                                                       ::upgrade::DEVCONFVER,
                                                       postVer);
    HAL_TRACE_DEBUG("[upgrade] Handling compat checks msg ... "
                    "preVer {} postVer {}",
                    preVer, postVer);

    // Check if A => B. Come up in microseg-enf if device.conf has hostpin
    if (!strcmp(preVer.c_str(), dev_conf_a)) {
        ret = hal::system_handle_a_to_b();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to handle A -> B");
            rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
            goto err;
        }
    }

    rsp = HdlrResp(::upgrade::SUCCESS, empty_str);
err:
    return rsp;
#if 0
    HAL_TRACE_DEBUG("[upgrade] Handling post restart msg ...");
    // TODO: mostly regular asic init path should work here, no special handling
    // needed
#endif
}

//------------------------------------------------------------------------------
// bring link up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::LinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling link up msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle upgrade success by releasing any transient state we are holding
//------------------------------------------------------------------------------
void
upgrade_handler::SuccessHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling success msg ...");
}

//------------------------------------------------------------------------------
// handle upgrade abort
//------------------------------------------------------------------------------
void
upgrade_handler::AbortHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling abort msg ...");
}

}    // namespace upgrade
}    // namespace hal
