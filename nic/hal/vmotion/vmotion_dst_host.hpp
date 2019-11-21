//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __VMOTION_DST_HOST_HPP__
#define __VMOTION_DST_HOST_HPP__

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

enum vmotion_dst_host_fsm_state_t {
    STATE_DST_HOST_INIT,
    STATE_DST_HOST_SYNC_REQ,
    STATE_DST_HOST_SYNCING,
    STATE_DST_HOST_SYNCED,
    STATE_DST_HOST_TERM_SYNCING,
    STATE_DST_HOST_TERM_SYNCED,
    STATE_DST_HOST_EP_MOVED,
    STATE_DST_HOST_END
};

#define DST_HOST_EVENT_ENTRIES(ENTRY)                 \
    ENTRY(EVT_START_SYNC,     0, "EVT_START_SYNC")    \
    ENTRY(EVT_SYNC,           1, "EVT_SYNC")          \
    ENTRY(EVT_SYNC_END,       2, "EVT_SYNC_END")      \
    ENTRY(EVT_TERM_SYNC,      3, "EVT_TERM_SYNC")     \
    ENTRY(EVT_TERM_SYNC_END,  4, "EVT_TERM_SYNC_END") \
    ENTRY(EVT_DST_EP_MOVED,   5, "EVT_DST_EP_MOVED")

DEFINE_ENUM(vmotion_dst_host_fsm_event_t, DST_HOST_EVENT_ENTRIES)

class vmotion_dst_host_fsm_def {
public:
    static vmotion_dst_host_fsm_def* factory(void);
    vmotion_dst_host_fsm_def(void);
    void set_state_machine(fsm_state_machine_def_t def) { dst_host_sm_def_ = def; }
    fsm_state_machine_def_t* get_state_machine(void) { return &dst_host_sm_def_; }
private:
    // Actions
    bool process_start_sync(fsm_state_ctx ctx, fsm_event_data data);
    bool process_sync(fsm_state_ctx ctx, fsm_event_data data);
    bool process_sync_end(fsm_state_ctx ctx, fsm_event_data data);
    bool process_term_sync(fsm_state_ctx ctx, fsm_event_data data);
    bool process_term_sync_end(fsm_state_ctx ctx, fsm_event_data data);
    bool process_ep_moved(fsm_state_ctx ctx, fsm_event_data data);
    void state_dst_host_end(fsm_state_ctx ctx);

    fsm_state_machine_def_t dst_host_sm_def_;
};

} // namespace hal

#endif    // __VMOTION_DST_HOST_HPP__
