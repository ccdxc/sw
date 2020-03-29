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

#ifndef __API_UPGRADE_EV_HITLESS_HPP__
#define __API_UPGRADE_EV_HITLESS_HPP__

using upg::upg_ev_hdlr_t;
using upg::upg_ev_params_t;

namespace api {

/// \brief upgrade event handlers to the registered thread
/// functions for upgrading process/thread A to B
/// A is the currently running process and B is the new process
/// each process/thread should implement this and act on these events.
typedef struct upg_ev_hitless_s {
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

    /// upgrade specific bringup should be done here (on B)
    /// the bringup shouldn't touch the shared hw resources with A
    /// [Ex: polling LIFs, ports, etc )
    /// below functions should check the bringup status of A
    upg_ev_hdlr_t ready;

    /// oper state syncing, config replay should be done here (on B)
    upg_ev_hdlr_t sync;

    /// threads should be paused here to a safe point for switchover (on A)
    upg_ev_hdlr_t quiesce;

    /// pipeline switch should be done here (on B)
    /// if it is success, it can start shared resource access
    upg_ev_hdlr_t switchover;

    /// abort an upgrade (on A / B)
    /// an abort (on A)
    ///   after backup, requires the thread to cleanup its saved states
    ///   after sw_quiesce, requires the thread to resume and goes to ready state
    /// an abort (on B)
    ///   after restore, requires the thread to cleanup its state and go to
    ///   sofware quiesce state.
    ///   after switchover, requires the threads to rollback and go to software
    ///   quiesce state.
    upg_ev_hdlr_t repeal;

    /// completed the upgrade (on A / B)
    /// threads should shutdown by receiving this event
    upg_ev_hdlr_t exit;

} upg_ev_hitless_t;

/// register the thread to the main upgrade event handler
void upg_ev_thread_hdlr_register(upg_ev_hitless_t &hdlr);

}   // namespace api

using api::upg_ev_hitless_t;

/// @}

#endif   // __API_UPGRADE_EV_HITLESS_HPP__
