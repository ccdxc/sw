// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr_internal.hpp"

namespace sdk {
namespace linkmgr {

sdk_ret_t
xcvr_poll_timer_cb(void *timer, uint32_t timer_id, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;

    // SDK_TRACE_DEBUG("TIMER: Thread: %s. Invoked", current_thread()->name());

    linkmgr_entry_data_t data;
    data.ctxt  = ctxt;
    data.timer = timer;

    // wake up the hal control thread to process port event
    ret = linkmgr_notify(LINKMGR_OPERATION_XCVR_POLL_TIMER,
                         &data,
                         q_notify_mode_t::Q_NOTIFY_MODE_NON_BLOCKING);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error notifying control-thread for"
                      " xcvr poll timer");
    }

    return ret;
}

sdk_ret_t
port_link_poll_timer_cb(void *timer, uint32_t timer_id, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;

    // SDK_TRACE_DEBUG("TIMER: Thread: %s. Invoked", current_thread()->name());

    linkmgr_entry_data_t data;
    data.ctxt  = ctxt;
    data.timer = timer;

    // wake up the hal control thread to process port event
    ret = linkmgr_notify(LINKMGR_OPERATION_PORT_LINK_POLL_TIMER,
                         &data,
                         q_notify_mode_t::Q_NOTIFY_MODE_NON_BLOCKING);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error notifying control-thread for"
                      " port link poll timer");
    }

    return ret;
}

// invoked by the periodic thread when timer expires
sdk_ret_t
link_bring_up_timer_cb(void *timer, uint32_t timer_id, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;

    // SDK_TRACE_DEBUG("TIMER: Thread: %s. Invoked", current_thread()->name());

    linkmgr_entry_data_t data;
    data.ctxt  = ctxt;
    data.timer = timer;

    // wake up the hal control thread to process port event
    ret = linkmgr_notify(LINKMGR_OPERATION_PORT_BRINGUP_TIMER,
                         &data,
                         q_notify_mode_t::Q_NOTIFY_MODE_NON_BLOCKING);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error notifying control-thread for port timer");
    }

    return ret;
}

// invoked by the periodic thread when debounce timer expires
sdk_ret_t
link_debounce_timer_cb(void *timer, uint32_t timer_id, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;

    // SDK_TRACE_DEBUG("TIMER: Thread: %s. Invoked", current_thread()->name());

    linkmgr_entry_data_t data;
    data.ctxt  = ctxt;
    data.timer = timer;

    // wake up the hal control thread to process port event
    ret = linkmgr_notify(LINKMGR_OPERATION_PORT_DEBOUNCE_TIMER,
                         &data,
                         q_notify_mode_t::Q_NOTIFY_MODE_NON_BLOCKING);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error notifying control-thread for port debounce timer");
    }

    return ret;
}

}    // namespace linkmgr
}    // namespace sdk
