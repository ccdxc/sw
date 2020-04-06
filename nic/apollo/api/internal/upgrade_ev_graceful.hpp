//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines event handlers during process upgrade
///
//----------------------------------------------------------------------------

/// \defgroup UPGRADE Upgrade event handlers
/// \ingroup  UPG_EVENT_HDLR
/// @{
///

#ifndef __API_UPGRADE_EV_GRACEFUL_HPP__
#define __API_UPGRADE_EV_GRACEFUL_HPP__

namespace api {

/// \brief upgrade event handlers for the registered thread
/// functions for upgrading process/thread A to B
/// A is the currently running process and B is the new process
/// each process/thread should implement this and act on these events.
typedef struct upg_ev_graceful_s {
    /// registering thread name
    /// used for debug traces. no other significance.
    char thread_name[SDK_MAX_NAME_LEN];

    /// compat checks should be done here (on A)
    upg_ev_hdlr_t compat_check_hdlr;

    /// start of a new upgrade, mount check the existance of B should be
    /// done here (on A).
    upg_ev_hdlr_t start_hdlr;

    /// software states should be saved here (on A).
    upg_ev_hdlr_t backup_hdlr;

    /// linkdown (on A).
    upg_ev_hdlr_t linkdown_hdlr;

    /// host device reset
    upg_ev_hdlr_t hostdev_reset_hdlr;

    /// threads should be paused here to a safe point for switchover (on A)
    /// also pipeline pause if required
    upg_ev_hdlr_t quiesce_hdlr;

    /// switching to B
    upg_ev_hdlr_t switchover_hdlr;

    /// making sure B bringup is successful
    upg_ev_hdlr_t ready_hdlr;

    /// repeal an upgrade (on A / B)
    /// an repeal (on A)
    ///   after backup, requires the thread to cleanup its saved states
    ///   after quiesce, and ready for re-spawn
    /// an repeal (on B)
    ///   needs to restore old state (rollback), and ready to re-spawn previous
    ///   version
    upg_ev_hdlr_t repeal_hdlr;

    /// completed the upgrade (on A / B)
    /// threads should shutdown by receiving this event
    upg_ev_hdlr_t exit_hdlr;

} upg_ev_graceful_t;

/// register the thread to the main upgrade event handler
void upg_ev_thread_hdlr_register(upg_ev_graceful_t &ev);

}   // namespace api

using api::upg_ev_graceful_t;

/// @}

#endif   // __API_UPGRADE_EV_GRACEFUL_HPP__
