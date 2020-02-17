//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __UPGRADE_EVENT_CB_HPP__
#define __UPGRADE_EVENT_CB_HPP__

namespace upg {

typedef upg_event_s {
    // empty now
} upg_event_t;

typedef sdk_ret_t (*upg_event_cb_t)(upg_event_t upg_ev);

// callback functions for upgrading process/thread A to B
// each process/thread should implement this and act on these events.
struct event_cb_s {
    // compat checks should be done here (on A)
    upg_event_cb_t compat_check_cb;

    // sw states should be saved here (on A).
    upg_event_cb_t backup_cb;

    // upgrade specific bringup should be done here (on B)
    // the bringup shouldn't touch the shared hw resources with A
    // [Ex: polling LIFs, ports, etc )
    upg_event_cb_t init_cb;

    // sw state restore, oper table copy should be done here (on B)
    upg_event_cb_t restore_cb;

    // threads should be paused here to a safe point for switchover (on A)
    upg_event_cb_t sw_quiesce_cb;

    // pipeline switch should be done here (on B)
    // if it is success, it can start shared resource access
    upg_event_cb_t switchover_cb;

    // abort an upgrade (on A / B)
    // an abort (on A)
    //   after backup, requires the thread to cleanup its saved states
    //   after sw_quiesce, requires the thread to resume and goes to ready state
    // an abort (on B)
    //   after restore, requires the thread to cleanup its state and go to
    //   sw quiesce state.
    //   after switchover, requires the threads to rollback and go to sw
    //   quiesce state.
    upg_event_cb_t abort_cb;

    // completed the upgrade (on A / B)
    // threads should shutdown by receiving this event
    upg_event_cb_t exit_cb;
};

typedef uint32_t upg_tid_t;

// register for upgrade event callbacks
// register the thread to the main upgrade event handler
void upg_event_cb_register(upg_tid_t tid, upg_event_cb_t *cb);

}    // namespace upg

#endif     // __UPGRADE_EVENT_CB_HPP__
