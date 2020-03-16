// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation for core upgrade data structures, methods, and APIs
///
//----------------------------------------------------------------------------


#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"
#include "nic/apollo/upgrade/core/utils.hpp"
#include "nic/apollo/upgrade/core/fsm.hpp"
#include "gen/proto/upg_fsm.hpp"
#include "nic/apollo/upgrade/core/ipc/notify_endpoint.hpp"

namespace upg {

evutil_timer api_rsp_timer;

static void
dispatch_event (stage_id_t id)
{
    std::string stage_name;
    stage_name = id_to_stage_name(id);

    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());

    upg_event_msg_t    next_event;
    next_event.stage = upg_stage_id(id);
    broadcast_notification(next_event);

}

static void
dispatch_event (stage_id_t id, svc_t& svc)
{
    std::string stage_name;
    stage_name = id_to_stage_name(id);

    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());

    svc.dispatch_event(upg_stage_id(id));

}

static void
update_thread_id(std::string name, uint32_t thread_id)
{
    svc_t service = fsm_services.svc_by_name(name);
    service.set_thread_id(thread_id);
    service.set_thread_name(name);

    for (auto& stage: fsm_stages) {
        for (auto& svc_instance : stage.second.svc_sequence()) {
            if (name.compare(svc_instance.name()) == 0) {
                svc_instance.set_thread_id(thread_id);
                svc_instance.set_thread_name(name);
            }
        }
    }
}

static void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *req_cookie,
                   const void *ctxt)
{
    UPG_TRACE_PRINT("RSP Event handler called.");

    stage_id_t id           = fsm_states.current_stage();
    upg_event_msg_t *event  = (upg_event_msg_t *)msg->data();
    std::string thread_name = event->rsp_thread_name;

    UPG_TRACE_INFO("Received UPG IPC event:\n\t stageid %s, \n\tstatus %s,"
           "\n\t thread %s,\n\t thread_id %u\n",
           upg_stage2str(event->stage),
           upg_status2str(event->rsp_status),
           event->rsp_thread_name,
           event->rsp_thread_id);
    dump(fsm_states);


    if (event->stage == upg_stage_id(id) &&
        fsm_states.is_valid_service(thread_name)) {

        if (event->stage == upg_stage_id(fsm_states.start_stage())) {
            update_thread_id(thread_name, event->rsp_thread_id );
        }

        fsm_states.update_stage_progress(svc_rsp_code(event->rsp_status));

        if ( fsm_states.is_current_stage_over()) {
            if (fsm_states.current_stage() == fsm_states.end_stage()) {
                // TODO:
                UPG_TRACE_PRINT("Upgrade is over Exiting..");
                exit(0);

            } else {
                if (fsm_states.is_serial_event_sequence()) {
                    UPG_TRACE_PRINT("Moving to Next stage in serial order..");
                    if (fsm_states.has_next_svc()) {
                        stage_id_t id = fsm_states.current_stage();
                        svc_t svc = fsm_states.next_svc();
                        dispatch_event(id, svc);
                    } else {
                        // No svc ? but not end stage?
                        // Alert: something went wrong
                        SDK_ASSERT(0);
                    }
                } else {
                    UPG_TRACE_PRINT("Moving to Next stage in parallel order..");
                    stage_id_t id = fsm_states.current_stage();
                    fsm_states.set_current_stage(id);
                    dispatch_event(id);
                }
            }
        } else if (fsm_states.is_serial_event_sequence() &&
                   (fsm_states.current_stage() != fsm_states.start_stage()) ) {
            UPG_TRACE_PRINT("Send serial request..");
            if (!fsm_states.has_next_svc()) {
                // Stage must have pending svc
                // if it is not over yet
                SDK_ASSERT(0);
            } else {
                stage_id_t id = fsm_states.current_stage();
                svc_t svc = fsm_states.next_svc();
                dispatch_event(id, svc);
            }
        } else {
            UPG_TRACE_PRINT("Its a parallel stage (or start"
                            " stage) and event already broadcasted.");
        }
    } else {
        // TODO: Not workign
        UPG_TRACE_PRINT("Dropping response from a previous stage..");
    }

    UPG_TRACE_PRINT("RSP Event handler Done..");
    dump(fsm_states);
}

    void
timer_callback (void*)
{
    std::cout<<"Timer called ... \n";
    fsm_states.stop_timer();
    // Check if pending is still non zero
    // need to fail and move to next stape
    // else start timer
    fsm_states.start_timer();
    return;
}


    static stage_id_t
lookup_stage_transition (stage_id_t cur, svc_rsp_code_t rsp)
{
    stage_id_t next_stage = STAGE_ID_EXIT ;
    next_stage = fsm_lookup_tbl[std::to_string(cur) +std::to_string(rsp)];
    return next_stage;
}

fsm::fsm(stage_id_t start, stage_id_t end) {
    current_stage_    = start;
    start_stage_      = start;
    end_stage_        = end;
    pending_response_ = 0;
    size_             = 0;
    this->loop = EV_DEFAULT;
}

fsm::~fsm(void) { }

stage_id_t fsm::current_stage(void) {
    return current_stage_;
}

// Note: please do not call this in ctor
void fsm::set_current_stage(stage_id_t stage_id) {
    current_stage_ = stage_id;

    SDK_ASSERT(fsm_stages.find(start_stage_) != fsm_stages.end());

    stage_t stage     = fsm_stages[start_stage_];
    svc_sequence_     = stage.svc_sequence();
    size_             = 0;
    for (auto x: svc_sequence_) {
        UPG_TRACE_PRINT("name : " + x.name());
        size_++;
    }
    pending_response_ = size_;
    timeout_          = stage.svc_rsp_timeout();

    std::string str   = __PRETTY_FUNCTION__   ;
    str += " pending_response :" + std::to_string(pending_response_);
    str += " : " + svc_sequence_to_str(svc_sequence_);
    UPG_TRACE_PRINT(str);
}


stage_id_t fsm::start_stage(void) {
    return start_stage_;
}

stage_id_t fsm::end_stage(void) {
    return end_stage_;
}

uint32_t fsm::pending_response(void) {
    return pending_response_;
}

void fsm::set_pending_response(uint32_t count) {
    pending_response_ = count;
}

void fsm::set_timeout(ev_tstamp timeout) {
    timeout_ = timeout;
}

ev_tstamp fsm::timeout(void) {
    return timeout_;
}

void fsm::update_stage_progress(svc_rsp_code_t rsp) {
    if (rsp != SVC_RSP_OK) {

        UPG_TRACE_PRINT("Got failure svc response");
        SDK_ASSERT(pending_response_ >= 0);

        current_stage_ = lookup_stage_transition(current_stage_, rsp);
        // TODO: if ( critical ??)

        // TODO: if ( exit ??)

        pending_response_ = 0;
        size_             = 0;
    } else {
        std::string str = "Got OK svc response 1 of " +
            std::to_string(pending_response_);

        pending_response_--;
        str += ". Pending rsp: " + std::to_string(pending_response_);

        SDK_ASSERT(pending_response_ >= 0);

        if (pending_response_ == 0) {
            str += " . Finish current stage.";
            // TODO: if current state is exit ??
            SDK_ASSERT(pending_response_ >= 0);
            current_stage_ = lookup_stage_transition(current_stage_, rsp);
            size_          = 0;
            // TODO: if ( critical ??)
        }
        UPG_TRACE_PRINT(str);
    }
}

svc_sequence_t fsm::svc_sequence(void) {
       return svc_sequence_;
}

bool fsm::has_next_svc(void) {
    return pending_response_ > 0;
}

bool fsm::is_valid_service(std::string svc) {
     for (auto x: svc_sequence_) {
         if (svc.compare(x.name()) == 0) {
             return true;
         }
     }
    std::string str = svc;
    str += " : is not a valid service.";
    UPG_TRACE_PRINT(str);
    return false;
}

svc_t fsm::next_svc(void) {
    svc_t svc;
    SDK_ASSERT(pending_response_ > 0);
    svc = svc_sequence_[size_ - pending_response_] ;
    return svc;
}

bool fsm::is_current_stage_over(void) {
    SDK_ASSERT(pending_response_ >= 0);

    if (pending_response_ == 0 ) {
        std::string str = " Pending rsp: " + std::to_string(pending_response_);
        UPG_TRACE_PRINT(str);

        SDK_ASSERT(fsm_stages.find(current_stage_) != fsm_stages.end());

        if (current_stage_ != end_stage_) {
            stage_t stage     = fsm_stages[current_stage_];
            timeout_          = stage.svc_rsp_timeout();
            svc_sequence_     = stage.svc_sequence();
            size_             = 0;
            for (auto x: svc_sequence_) {
                size_++;
            }
            pending_response_ = size_;
        } else {
            timeout_          = 0;
            pending_response_ = 0;
            size_             = 0;
        }

        SDK_ASSERT((pending_response_>= 0) &&
                   (pending_response_ <= size_));

        return true;
    }
    return false;
}

bool fsm::is_serial_event_sequence(void) {
    SDK_ASSERT(fsm_stages.find(current_stage_) != fsm_stages.end());
    stage_t stage     = fsm_stages[current_stage_];
    return stage.event_sequence() == SERIAL;
}

void fsm::start_timer(void) {
    std::cout<<"Timer Start .......... \n";
    evutil_timer_start(EV_A_ &api_rsp_timer, timer_callback, NULL, timeout_/1000,
                       0.1);
}

void fsm::stop_timer(void) {
    std::cout<<"Timer Stop .......... \n";
    evutil_timer_stop(EV_A_ &api_rsp_timer);
}

static svc_sequence_t
str_to_svc_sequence (std::string& svc_seq)
{

    if (svc_seq.empty()) {
        return fsm_services.svc_sequence();
    }
    std::vector<std::string> svcs;
    token_parse(svc_seq, svcs);
    svc_sequence_t svcs_sequence;
    for (std::string& name:svcs){
        svc_t svc;
        svc = fsm_services.svc_by_name(name);
        svcs_sequence.push_back(svc);
    }
    return svcs_sequence;
};

static event_sequence_t
str_to_event_sequence (std::string& evt_seq)
{
    event_sequence_t seq=PARALLEL;
    if (evt_seq.empty()) {
        seq = fsm_services.event_sequence();
    } else if (boost::iequals(evt_seq, "parallel")) {
        seq =  PARALLEL;
    } else if (boost::iequals(evt_seq, "serial")) {
        seq = SERIAL;
    } else {
        // erro
        SDK_ASSERT(0);
    }
    // TODO: more validation
    return seq;
};

static transition_t
to_transition (stage_id_t stage_id)
{
    transition_t transitions;
    for (stage_transition_t& stage: stage_transitions) {
        if (stage_id == stage.from()){
            transitions.push_back(stage);
        }
    }
    return transitions;
}

static void
init_svc (void)
{
    fsm_services.set_event_sequence(event_sequence);
    for (svc_t& service: svc) {
        fsm_services.add_svc(service);
    }
}

static void
init_lookup_table (void)
{
    for (stage_transition_t& stage: stage_transitions) {
        std::string key;
        key = std::to_string(stage.from()) +
            std::to_string(stage.svc_rsp_code());
        fsm_lookup_tbl[key] = stage.to();
    }
}

static void
init_fsm_stages (void)
{
    stage_id_t          stage_id;
    time_t              svc_rsp_timeout;
    svc_sequence_t      svcs_seq;
    event_sequence_t    event_seq;
    transition_t        transitions;
    scripts_t           prehook;
    scripts_t           posthook;

    // fsm_stages is ordered by stage_id_t
    for (idl_stage_t& stage : idl_stages_cfg) {
        stage_id    = name_to_stage_id(stage.stage());
        svc_rsp_timeout = str_to_timeout(stage.svc_rsp_timeout());
        svcs_seq    = str_to_svc_sequence(stage.svc_sequence());
        event_seq   = str_to_event_sequence(stage.event_sequence());
        transitions = to_transition(stage_id);
        prehook     = str_to_scripts(stage.pre_hook_scripts());
        posthook    = str_to_scripts(stage.post_hook_scripts());

        fsm_stages[stage_id] =  stage_t(svc_rsp_timeout, svcs_seq,
                                      event_seq, transitions,
                                      prehook,posthook);
    }
}

static void
init_fsm (void)
{
    upg_event_msg_t            event;
    stage_id_t start_stage     = fsm_states.start_stage();
    event.stage                = upg_stage_id(start_stage);

    SDK_ASSERT(fsm_stages.find(start_stage) != fsm_stages.end());
    fsm_states.set_current_stage(start_stage);

    sdk::ipc::reg_response_handler(PDS_IPC_MSG_ID_UPGRADE,
                                   upg_event_handler, NULL);
    fsm_states.start_timer();
    sdk::ipc::broadcast(PDS_IPC_MSG_ID_UPGRADE, &event, sizeof(event));
    return;
}

void
init (void)
{
    init_svc();
    init_lookup_table();
    init_fsm_stages();
    init_fsm();

#if 0
    dump(fsm_stages);
    dump(fsm_services);
    dump(fsm_lookup_tbl);
#endif
}

static sdk_ret_t
register_callback_inernal (stage_callback_t cb_type, stage_id_t cb_stage,
                   void *(*callback)(void *), void *arg)
{
    // TODO:
    //
    sdk_ret_t ret;

    ret = SDK_RET_OK;
    return ret;
}

sdk_ret_t
register_callback (stage_callback_t cb_type, stage_id_t cb_stage,
                   void *(*callback)(void *), void *arg)
{
    // TODO:
    //
    return register_callback_inernal(cb_type, cb_stage, callback, arg);
}

sdk_ret_t
do_switchover (void)
{
    return SDK_RET_OK;
}
}
