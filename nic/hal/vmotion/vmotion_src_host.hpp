//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __VMOTION_SRC_HOST_HPP__
#define __VMOTION_SRC_HOST_HPP__

#include "nic/include/base.hpp"
#include "nic/utils/fsm/fsm.hpp"

using hal::utils::fsm_state_t;
using hal::utils::fsm_state_ctx;
using hal::utils::fsm_event_data;
using hal::utils::fsm_state_machine_t;
using hal::utils::fsm_state_machine_def_t;
using hal::utils::fsm_transition_t;
using hal::utils::fsm_transition_func;
using hal::utils::fsm_state_func;

namespace hal {

enum vmotion_src_host_fsm_state_t {
     STATE_SRC_HOST_INIT,
     STATE_SRC_HOST_SYNC,
     STATE_SRC_HOST_SYNCED,
     STATE_SRC_HOST_TERM_SYNC,
     STATE_SRC_HOST_TERM_SYNCED,
     STATE_SRC_HOST_EP_MOVED,
     STATE_SRC_HOST_END
};

#define SRC_HOST_EVENT_ENTRIES(ENTRY)                            \
    ENTRY(EVT_SYNC_BEGIN,             0, "EVT_SYNC_BEGIN")       \
    ENTRY(EVT_SYNC_DONE,              1, "EVT_SYNC_DONE")        \
    ENTRY(EVT_TERM_SYNC_REQ,          2, "EVT_TERM_SYNC_REQ")    \
    ENTRY(EVT_TERM_SYNC_DONE,         3, "EVT_TERM_SYNC_DONE")   \
    ENTRY(EVT_TERM_SYNCED_ACK,        4, "EVT_TERM_SYNCED_ACK")  \
    ENTRY(EVT_SRC_EP_MOVED_ACK,       5, "EVT_SRC_EP_MOVED_ACK") \
    ENTRY(EVT_EP_DELETE_RCVD,         6, "EVT_EP_DELETE_RCVD")

DEFINE_ENUM(vmotion_src_host_fsm_event_t, SRC_HOST_EVENT_ENTRIES)

class vmotion_src_host_fsm_def {
public:
    vmotion_src_host_fsm_def(void);
    static vmotion_src_host_fsm_def* factory(void);
    void set_state_machine(fsm_state_machine_def_t def) { src_host_sm_def_ = def; }
    fsm_state_machine_def_t* get_state_machine(void) { return &src_host_sm_def_; }
private:
    // Actions
    bool proc_sync_begin(fsm_state_ctx ctx, fsm_event_data data);
    bool proc_evt_sync_done(fsm_state_ctx ctx, fsm_event_data data);
    bool proc_term_sync_req(fsm_state_ctx ctx, fsm_event_data data);
    bool proc_term_sync_done(fsm_state_ctx ctx, fsm_event_data data);
    bool proc_term_synced_ack(fsm_state_ctx ctx, fsm_event_data data);
    bool proc_ep_moved_ack(fsm_state_ctx ctx, fsm_event_data data);
    bool proc_ep_delete(fsm_state_ctx ctx, fsm_event_data data);
    void state_src_host_end_entry(fsm_state_ctx ctx);

    fsm_state_machine_def_t src_host_sm_def_;
};

} // namespace hal

#endif    // __VMOTION_DST_HOST_HPP__
