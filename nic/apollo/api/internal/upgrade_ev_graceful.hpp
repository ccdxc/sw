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

using upg::upg_ev_hdlr_t;
using upg::upg_ev_params_t;

namespace api {

/// \brief upgrade event handlers for the registered thread
/// functions for upgrading process/thread A to B
/// A is the currently running process and B is the new process
/// each process/thread should implement this and act on these events.
typedef struct upg_ev_graceful_s {
    /// registering thread id
    /// used for debug traces. no other significance.
    upg_thread_id_t  thread_id;

    /// compat checks should be done here (on A)
    upg_ev_hdlr_t compat_check;

    /// start of a new upgrade, mount check the existance of B should be
    /// done here (on A).
    upg_ev_hdlr_t start;

    /// software states should be saved here (on A).
    upg_ev_hdlr_t backup;

    /// linkdown (on A).
    upg_ev_hdlr_t linkdown;

    /// host device reset
    upg_ev_hdlr_t hostdev_reset;

    /// threads should be paused here to a safe point for switchover (on A)
    /// also pipeline pause if required
    upg_ev_hdlr_t quiesce;

    /// switching to B
    upg_ev_hdlr_t switchover;

    /// making sure B bringup is successful
    upg_ev_hdlr_t ready;

    /// repeal an upgrade (on A / B)
    /// an repeal (on A)
    ///   after backup, requires the thread to cleanup its saved states
    ///   after quiesce, and ready for re-spawn
    /// an repeal (on B)
    ///   needs to restore old state (rollback), and ready to re-spawn previous
    ///   version
    upg_ev_hdlr_t repeal;

    /// completed the upgrade (on A / B)
    /// threads should shutdown by receiving this event
    upg_ev_hdlr_t exit;

} upg_ev_graceful_t;

/// register the thread to the main upgrade event handler
void upg_ev_thread_hdlr_register(upg_ev_graceful_t &ev);

}   // namespace api

using api::upg_ev_graceful_t;

/// @}

#endif   // __API_UPGRADE_EV_GRACEFUL_HPP__
