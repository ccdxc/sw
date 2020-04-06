//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "platform/src/lib/nicmgr/include/upgrade.hpp"


namespace nicmgr {
namespace lib {

static UpgradeEvent UpgEvent;
static UpgradeState ExpectedState;
static uint32_t ServiceTimeout;
static bool MoveToNextState;
static bool SendAppResp;
static bool IsUpgFailed = false;
static DeviceManager *devmgr;

static upg_device_reset_status_cb_t reset_status_app_cb;
static upg_event_response_cb_t event_response_app_cb;

evutil_timer ServiceTimer;

static string
UpgStateToStr (UpgradeState state)
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

static void
upg_timer_func (void *arg)
{
    static uint32_t max_retry = 0;
    int ret = 0;
    sdk_ret_t status;

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
                ret = reset_status_app_cb();
                if (ret == -1)
                    return;
            }
        }

        NIC_FUNC_DEBUG("Sending App Response to hal upgrade client");

        if (max_retry >= ServiceTimeout) {
            status = SDK_RET_ERR;
        } else {
            status = SDK_RET_OK;
        }
        event_response_app_cb(status, arg);

        evutil_timer_stop(devmgr->ev_loop(), &ServiceTimer);

        return;
    }
}

sdk_ret_t
upg_compat_check_handler (void)
{
    NIC_FUNC_DEBUG("In compatcheck handler");
    if (!devmgr->UpgradeCompatCheck()) {
        NIC_LOG_ERR("Compat check failed");
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

// Bring link-down
sdk_ret_t
upg_link_down_handler (void *cookie)
{
    NIC_FUNC_DEBUG("In linkdown handler");
    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_LOG_ERR("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        return SDK_RET_ERR;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are quiesced or not ...");

    ExpectedState = DEVICES_QUIESCED_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = false;
    SendAppResp = true;
    evutil_timer_start(devmgr->ev_loop(), &ServiceTimer, upg_timer_func, cookie, 0.0, 0.1);

    return sdk_ret_t::SDK_RET_IN_PROGRESS;
}

sdk_ret_t
upg_failed_handler (void *cookie)
{
    // we will run through all the state machine for nicmgr upgrade if somehow upgrade failed
    NIC_FUNC_DEBUG("In failed handler");
    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_LOG_ERR("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        return sdk_ret_t::SDK_RET_UPG_CRITICAL;
    }

    NIC_FUNC_DEBUG("Waiting for LinkDown event response from eth drivers");

    ExpectedState = DEVICES_QUIESCED_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = true;
    SendAppResp = false;
    evutil_timer_start(devmgr->ev_loop(), &ServiceTimer, upg_timer_func, cookie, 0.0, 0.1);

    return sdk_ret_t::SDK_RET_IN_PROGRESS;
}

sdk_ret_t
upg_host_down_handler (void *cookie)
{
    NIC_FUNC_DEBUG("In host down handler");
    if (devmgr->HandleUpgradeEvent(UPG_EVENT_DEVICE_RESET)) {
        NIC_LOG_ERR("UPG_EVENT_DEVICE_RESET event Failed! Cannot continue upgrade FSM");
        return SDK_RET_ERR;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are in reset or not ...");
    ExpectedState = DEVICES_RESET_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = false;
    SendAppResp = true;
    evutil_timer_start(devmgr->ev_loop(), &ServiceTimer, upg_timer_func, cookie, 0.0, 0.1);

    return sdk_ret_t::SDK_RET_IN_PROGRESS;
}

void
upg_ev_init (upg_device_reset_status_cb_t reset_status_cb,
             upg_event_response_cb_t event_response_cb)
{
    reset_status_app_cb = reset_status_cb;
    event_response_app_cb = event_response_cb;
    devmgr = DeviceManager::GetInstance();
}

}   // namespace lib
}   // namespace nicmgr
