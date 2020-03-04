//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines event callbacks during process upgrade
///
//----------------------------------------------------------------------------

/// \defgroup UPGRADE Upgrade event callbacks
/// \ingroup  UPG_EVENTCB
/// @{
///

#ifndef __UPGRADE_EVENT_CB_HPP__
#define __UPGRADE_EVENT_CB_HPP__

namespace upg {

/// \brief upgrade event context
typedef struct upg_event_ctxt_s {
    // empty now
} upg_event_ctxt_t;

/// \brief upgrade event callback function
typedef sdk_ret_t (*upg_event_cb_t)(upg_event_ctxt_t *upg_ev_ctxt);

/// thread id from which upgrade event callbacks are registerd
typedef uint32_t upg_thread_id_t;

/// \brief upgrade event callbacks
/// callback functions for upgrading process/thread A to B
/// A is the currently running process and B is the new process
/// each process/thread should implement this and act on these events.
typedef struct upg_event_s {
    /// registering thread id
    /// used for debug traces. no other significance.
    upg_thread_id_t  thread_id;

    /// compat checks should be done here (on A)
    upg_event_cb_t compat_check_cb;

    /// software states should be saved here (on A).
    upg_event_cb_t backup_cb;

    /// upgrade specific bringup should be done here (on B)
    /// the bringup shouldn't touch the shared hw resources with A
    /// [Ex: polling LIFs, ports, etc )
    upg_event_cb_t init_cb;

    /// software state restore, oper table copy should be done here (on B)
    upg_event_cb_t restore_cb;

    /// threads should be paused here to a safe point for switchover (on A)
    upg_event_cb_t sw_quiesce_cb;

    /// pipeline switch should be done here (on B)
    /// if it is success, it can start shared resource access
    upg_event_cb_t switchover_cb;

    /// abort an upgrade (on A / B)
    /// an abort (on A)
    ///   after backup, requires the thread to cleanup its saved states
    ///   after sw_quiesce, requires the thread to resume and goes to ready state
    /// an abort (on B)
    ///   after restore, requires the thread to cleanup its state and go to
    ///   sofware quiesce state.
    ///   after switchover, requires the threads to rollback and go to software
    ///   quiesce state.
    upg_event_cb_t abort_cb;

    /// completed the upgrade (on A / B)
    /// threads should shutdown by receiving this event
    upg_event_cb_t exit_cb;
} upg_event_t;

/// register the thread to the main upgrade event handler
void upg_event_cb_register(upg_event_t &upg_ev);

}    // namespace upg

using upg::upg_event_ctxt_t;

/// @}

#endif     // __UPGRADE_EVENT_CB_HPP__
