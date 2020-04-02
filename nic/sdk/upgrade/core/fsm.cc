// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation for core upgrade data structures, methods, and APIs
///
//----------------------------------------------------------------------------

#include <iostream>
#include <cassert>
#include <exception>
#include <sstream>
#include <string>
#include <ev.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "include/sdk/base.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "fsm.hpp"
#include "ipc.hpp"
#include "stage.hpp"
#include "logger.hpp"
#include "utils.hpp"

#define GRACEFUL_UPGRADE_GEN "graceful_upgrade_gen.json"
#define HITLESS_UPGRADE_GEN  "hitless_upgrade_gen.json"

namespace sdk {
namespace upg {

namespace pt = boost::property_tree;

static struct ev_loop *loop;
static ev_timer timeout_watcher;
static fsm fsm_states;
static upg_stages_map fsm_stages;
static upg_svc_map fsm_services;
static svc_sequence_list default_svc_names;
static stage_map fsm_lookup_tbl;
event_sequence_t event_sequence;
upg_stage_t entry_stage;

static void
dispatch_event (ipc_svc_dom_id_t dom, upg_stage_t id, upg_svc svc)
{
    // TODO: domain
    std::string stage_name(upg_stage2str(id));
    UPG_TRACE_VERBOSE("Sending event %s to svc: %s ipc_id: %d\n",
                      stage_name.c_str(), svc.name().c_str(), svc.ipc_id());
    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());
    svc.dispatch_event(dom, id);
}

static void
dispatch_event (ipc_svc_dom_id_t dom, upg_stage_t id)
{
    std::string stage_name(upg_stage2str(id));
    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());
    UPG_TRACE_VERBOSE("Broadcasting event %s\n", stage_name.c_str());

    for (auto& name : fsm_states.svc_sequence()) {
        SDK_ASSERT(fsm_services.find(name) != fsm_services.end());
        upg_svc svc = fsm_services[name];
        dispatch_event(dom, id, svc);
    }
}

static void
update_ipc_id (std::string name, uint32_t ipc_id)
{
    SDK_ASSERT(fsm_services.find(name) != fsm_services.end());

    upg_svc service = fsm_services[name];
    service.set_ipc_id(ipc_id);
    fsm_services[name] = service;
    UPG_TRACE_VERBOSE("Updating IPC id of service: %s,"
                      " ipc id: %d\n ",
                      name.c_str(), ipc_id);
}

static void
move_to_nextstage (void)
{
    // TODO: domain
    ipc_svc_dom_id_t domain = IPC_SVC_DOM_ID_A;

    std::string name(upg_stage2str(fsm_states.current_stage()));
    if (fsm_states.is_serial_event_sequence()) {
        UPG_TRACE_VERBOSE("Moving to next stage (serial): %s\n", name.c_str());
        dump(fsm_states);
        if (fsm_states.has_next_svc()) {
            upg_stage_t id = fsm_states.current_stage();
            std::string svc_name = fsm_states.next_svc();
            SDK_ASSERT(fsm_services.find(svc_name) != fsm_services.end());
            upg_svc svc = fsm_services[svc_name];
            fsm_states.timer_stop();
            fsm_states.timer_start();
            dispatch_event(domain, id, svc);
        } else {
            UPG_TRACE_VERBOSE("Oops! no service to send request.");
            SDK_ASSERT(0);
        }
    } else {
        UPG_TRACE_VERBOSE("Moving to next stage (parallel): %s\n",
                          name.c_str());
        dump(fsm_states);
        upg_stage_t id = fsm_states.current_stage();
        fsm_states.timer_stop();
        fsm_states.timer_start();
        dispatch_event(domain, id);
    }
}

static void
send_ipc_to_next_service (void)
{
    ipc_svc_dom_id_t domain = IPC_SVC_DOM_ID_A;

    UPG_TRACE_VERBOSE("Send serial request.");
    if (!fsm_states.has_next_svc()) {
        // Stage must have pending svc
        // if it is not over yet
        SDK_ASSERT(0);
    } else {
        upg_stage_t id = fsm_states.current_stage();
        std::string svc_name = fsm_states.next_svc();
        SDK_ASSERT(fsm_services.find(svc_name) != fsm_services.end());
        upg_svc svc = fsm_services[svc_name];
        dispatch_event(domain, id, svc);
    }
}

void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg)
{
    UPG_TRACE_VERBOSE("IPC response event handler called.");

    upg_stage_t id = fsm_states.current_stage();
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();
    std::string thread_name = event->rsp_thread_name;

    UPG_TRACE_INFO("Received UPG IPC event:\n\t stageid %s, \n\tstatus %s,"
                   "\n\t thread %s,\n\t thread_id %u\n",
                   upg_stage2str(event->stage),
                   upg_status2str(event->rsp_status), event->rsp_thread_name,
                   event->rsp_thread_id);

    if (event->stage == id && fsm_states.is_valid_service(thread_name)) {

        if (event->stage == fsm_states.start_stage()) {
            update_ipc_id(thread_name, event->rsp_thread_id);
        }

        fsm_states.update_stage_progress(svc_rsp_code(event->rsp_status));
        if (fsm_states.is_current_stage_over()) {
            upg_stage_t id = fsm_states.current_stage();
            fsm_states.set_current_stage(id);
            if (fsm_states.current_stage() == fsm_states.end_stage()) {
                // TODO:
                UPG_TRACE_VERBOSE("Upgrade is successful.");
                exit(0);
            } else {
                move_to_nextstage();
            }
        } else if (fsm_states.is_serial_event_sequence() &&
                   (fsm_states.current_stage() != fsm_states.start_stage())) {
            send_ipc_to_next_service();
        } else {
            UPG_TRACE_VERBOSE("Stage progress is updated.");
        }
    } else {
        UPG_TRACE_VERBOSE("Dropping event response, not expecting it !");
    }

    UPG_TRACE_VERBOSE("Done with IPC response event handler.");
}

static void
timeout_cb (EV_P_ ev_timer *w, int revents)
{

    UPG_TRACE_VERBOSE("Timer expired !");
    fsm_states.timer_stop();
    fsm_states.update_stage_progress(SVC_RSP_NONE);

    if (fsm_states.current_stage() != fsm_states.end_stage()) {
        move_to_nextstage();
    } else {
        UPG_TRACE_VERBOSE("Upgrade Failed !");
        exit(1);
    }
}

static upg_stage_t
lookup_stage_transition (upg_stage_t cur, svc_rsp_code_t rsp)
{
    upg_stage_t next_stage = UPG_STAGE_EXIT;
    next_stage = fsm_lookup_tbl[std::to_string(cur) + std::to_string(rsp)];
    return next_stage;
}

// Note: please do not call this in ctor
void
fsm::set_current_stage(const upg_stage_t stage_id) {
    current_stage_ = stage_id;

    SDK_ASSERT(fsm_stages.find(start_stage_) != fsm_stages.end());

    upg_stage stage = fsm_stages[current_stage_];
    svc_sequence_ = stage.svc_sequence();
    size_ = 0;
    for (auto x : svc_sequence_) {
        size_++;
    }
    pending_response_ = size_;
    timeout_ = stage.svc_rsp_timeout();
    timeout_ = double(timeout_ * 1.0) / 1000;

    UPG_TRACE_VERBOSE("\n stage :%s , pending response : %d, svc sequence: %s,"
                      " timeout: %f \n ",
                      upg_stage2str(current_stage_), pending_response_,
                      svc_sequence_to_str(svc_sequence_).c_str(), timeout_);
}

void
fsm::timer_stop(void) {
    UPG_TRACE_VERBOSE("Stopping the timer !");
    ev_timer_stop(loop, &timeout_watcher);
}

void
fsm::timer_init(const void *ctxt) {
    UPG_TRACE_VERBOSE("Initializing the timer with timeout: %f", timeout_);

    sdk::event_thread::event_thread *curr_thread;
    curr_thread = (sdk::event_thread::event_thread *)ctxt;
    loop = curr_thread->ev_loop();

    ev_timer_init(&timeout_watcher, timeout_cb, timeout_, 0.0);
}

void
fsm::timer_set(void) {
    UPG_TRACE_VERBOSE("Setting the timer : %f", timeout_);
    ev_timer_set(&timeout_watcher, timeout_, 0.0);
}

void
fsm::timer_start(void) {
    UPG_TRACE_VERBOSE("Starting the timer.");
    ev_timer_start(loop, &timeout_watcher);
}

void
fsm::update_stage_progress(const svc_rsp_code_t rsp) {
    if (rsp != SVC_RSP_OK) {
        switch (rsp) {
        case UPG_STATUS_FAIL:
            UPG_TRACE_VERBOSE("Got failure svc response");
            break;
        case UPG_STATUS_CRITICAL:
            // TODO: if ( critical ??)
            UPG_TRACE_VERBOSE("Got critical svc response");
            break;
        case SVC_RSP_NONE:
            UPG_TRACE_VERBOSE("Timer expired, no svc response so far.");
            break;
        default:
            break;
        }
        // TODO: if current stage is ( exit/rollback ??)
        current_stage_ = lookup_stage_transition(current_stage_, rsp);
        SDK_ASSERT(pending_response_ >= 0);
        pending_response_ = 0;
        size_ = 0;
    } else {
        std::string str =
            "Got OK svc response 1 of " + std::to_string(pending_response_);

        pending_response_--;
        str += ". Pending rsp: " + std::to_string(pending_response_);

        SDK_ASSERT(pending_response_ >= 0);

        if (pending_response_ == 0) {
            str += " . Finish current stage.";
            // TOO: if current state is exit ??
            SDK_ASSERT(pending_response_ >= 0);
            current_stage_ = lookup_stage_transition(current_stage_, rsp);
            size_ = 0;
            // TODO: if ( critical ??)
        }
        UPG_TRACE_VERBOSE("%s", str.c_str());
    }
}

bool
fsm::is_valid_service(const std::string svc) const {
    for (auto x : svc_sequence_) {
        if (svc.compare(x) == 0) {
            return true;
        }
    }
    UPG_TRACE_VERBOSE("\n %s is not a valid service.\n", svc.c_str());
    return false;
}

std::string
fsm::next_svc(void) const {
    std::string svc;
    SDK_ASSERT(pending_response_ > 0);
    svc = svc_sequence_[size_ - pending_response_];
    return svc;
}

bool
fsm::is_current_stage_over(void) {
    SDK_ASSERT(pending_response_ >= 0);

    if (pending_response_ == 0) {

        SDK_ASSERT((current_stage_ == UPG_STAGE_EXIT) ||
                   (fsm_stages.find(current_stage_) != fsm_stages.end()));

        if (current_stage_ != end_stage_) {
            upg_stage stage = fsm_stages[current_stage_];
            timeout_ = stage.svc_rsp_timeout();
            svc_sequence_ = stage.svc_sequence();
            size_ = 0;
            for (auto x : svc_sequence_) {
                size_++;
            }
            pending_response_ = size_;
        } else {
            timeout_ = 0;
            pending_response_ = 0;
            size_ = 0;
        }

        SDK_ASSERT((pending_response_ >= 0) && (pending_response_ <= size_));

        return true;
    }
    return false;
}

bool
fsm::is_serial_event_sequence(void) const {
    SDK_ASSERT(fsm_stages.find(current_stage_) != fsm_stages.end());
    upg_stage stage = fsm_stages[current_stage_];
    return stage.event_sequence() == SERIAL;
}

static svc_sequence_list
str_to_svc_sequence (std::string& svc_seq)
{
    svc_sequence_list svcs_sequence;
    if (svc_seq.empty()) {
        svcs_sequence = default_svc_names;
    } else {
        std::vector<std::string> svcs;
        token_parse(svc_seq, svcs);
        for (std::string& name : svcs) {
            svcs_sequence.push_back(name);
        }
    }
    return svcs_sequence;
};

static event_sequence_t
str_to_event_sequence (std::string& evt_seq)
{
    event_sequence_t seq = PARALLEL;
    if (evt_seq.empty()) {
        seq = event_sequence;
    } else if (boost::iequals(evt_seq, "parallel")) {
        seq = PARALLEL;
    } else if (boost::iequals(evt_seq, "serial")) {
        seq = SERIAL;
    } else {
        SDK_ASSERT(0);
    }
    return seq;
};

static svc_sequence_list
get_upg_services (pt::ptree& tree)
{
    svc_sequence_list svc_list;

    BOOST_FOREACH (pt::ptree::value_type const &svc,
                   tree.get_child("upg_svc")) {
        svc_list.push_back(svc.second.get_value<std::string>());
    }
    return svc_list;
}

static sdk_ret_t
init_svc (pt::ptree& tree)
{
    sdk_ret_t ret = SDK_RET_ERR;
    try
    {
        svc_sequence_list svc_list;

        svc_list = get_upg_services(tree);
        for (auto const& name : svc_list) {
            upg_svc service = upg_svc(name);
            fsm_services[service.name()] = service;
            default_svc_names.push_back(service.name());
        }
        return SDK_RET_OK;
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Error reading upgrade spec:\n %s\n", ex.what());
        UPG_TRACE_ERR("Error reading upgrade spec:\n %s\n", ex.what());
    }
    return ret;
}

static sdk_ret_t
init_stage_transitions (pt::ptree& tree)
{
    upg_stage_t curr;
    upg_stage_t next;
    svc_rsp_code_t rsp;

    std::string key;
    std::vector<std::string> stage_transition;

    sdk_ret_t ret = SDK_RET_ERR;
    try
    {
        BOOST_FOREACH (pt::ptree::value_type &row,
                       tree.get_child("upg_stage_transitions"))
        {
            stage_transition.clear();
            BOOST_FOREACH (pt::ptree::value_type& transition, row.second)
            {
                std::string str = transition.second.get_value<std::string>();
                stage_transition.push_back(str);
            }
            curr = name_to_stage_id(stage_transition[0]);
            next = name_to_stage_id(stage_transition[2]);
            rsp = svc_rsp_code_name_to_id(stage_transition[1]);
            key = std::to_string(curr) + std::to_string(rsp);

            fsm_lookup_tbl[key] = next;
        }
        return SDK_RET_OK;
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Error reading upgrade spec:\n %s\n", ex.what());
        UPG_TRACE_ERR("Error reading upgrade spec:\n %s\n", ex.what());
    }
    return ret;
}

static sdk_ret_t
init_fsm_stages (pt::ptree& tree)
{
    upg_stage_t stage_id;
    ev_tstamp svc_rsp_timeout;
    svc_sequence_list svcs_seq;
    event_sequence_t event_seq;
    upg_scripts prehook;
    upg_scripts posthook;

    sdk_ret_t ret = SDK_RET_OK;
    try
    {
        BOOST_FOREACH (pt::ptree::value_type const&v,
                       tree.get_child("upg_stages")) {
            const std::string & key = v.first;
            const pt::ptree & subtree = v.second;
            std::string timeout_str;
            std::string svc_seq_str;
            std::string evt_seq_str;
            std::string pre_hook_str;
            std::string post_hook_str;
            if(subtree.empty()) {
                UPG_TRACE_ERR(" Parsing Error !");
                return SDK_RET_ERR;
            } else {
                timeout_str = subtree.get<std::string>("rsp_timeout", "5000");
                svc_seq_str = subtree.get<std::string>("svc_sequence", "");
                evt_seq_str =
                    subtree.get<std::string>("event_sequence", "parallel");
                pre_hook_str = subtree.get<std::string>("pre_hook", "");
                post_hook_str = subtree.get<std::string>("post_hook", "");

                stage_id = name_to_stage_id(key);
                svcs_seq = str_to_svc_sequence(svc_seq_str);
                event_seq = str_to_event_sequence(evt_seq_str);
                prehook = str_to_scripts(pre_hook_str);
                posthook  = str_to_scripts(post_hook_str);
                svc_rsp_timeout = str_to_timeout(timeout_str);

                fsm_stages[stage_id] = upg_stage(svc_rsp_timeout, svcs_seq,
                                                 event_seq, prehook,posthook);
            }
        }
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Error reading upgrade spec:\n %s\n", ex.what());
        UPG_TRACE_ERR("Error reading upgrade spec:\n %s\n", ex.what());
    }
    return ret;
}

static sdk_ret_t
set_entry_stage (pt::ptree& tree)
{
    std::string stage;
    try
    {
        stage = tree.get<std::string>("entry_stage", "compatcheck");
        entry_stage = name_to_stage_id(stage);
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Error reading upgrade spec:\n %s\n", ex.what());
        UPG_TRACE_ERR("Error reading upgrade spec:\n %s\n", ex.what());
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static sdk_ret_t
set_event_sequence (pt::ptree& tree)
{
    try
    {
        std::string seq;
        seq = tree.get<std::string>("event_sequence", "parallel");
        event_sequence = str_to_event_sequence(seq);
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Error reading upgrade spec:\n %s\n", ex.what());
        UPG_TRACE_ERR("Error reading upgrade spec:\n %s\n", ex.what());
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static void
init_fsm (void *ctxt)
{
    fsm_states.set_start_stage(entry_stage);

    upg_stage_t start_stage = fsm_states.start_stage();

    SDK_ASSERT(fsm_stages.find(start_stage) != fsm_stages.end());
    fsm_states.timer_init(ctxt);
    fsm_states.set_current_stage(start_stage);

    fsm_states.timer_start();
    upg_ipc_init(upg_event_handler);
    upg_send_broadcast_request(IPC_SVC_DOM_ID_A, fsm_states.start_stage());
    return;
}

static sdk_ret_t
load_pipeline_json(pt::ptree& tree, bool is_graceful)
{
    sdk_ret_t ret = SDK_RET_ERR;
    try
    {   std::string upg_gen_json = std::string(std::getenv("CONFIG_PATH"));
        upg_gen_json += "/gen/";

        if (is_graceful) {
            upg_gen_json += GRACEFUL_UPGRADE_GEN;
        } else {
            upg_gen_json += HITLESS_UPGRADE_GEN;
        }

        if (access(upg_gen_json.c_str(), F_OK) != -1)
        {
            pt::read_json(upg_gen_json, tree);
            return SDK_RET_OK;
        } else {
            UPG_TRACE_VERBOSE("Error reading upgrade spec: No Access");
            UPG_TRACE_ERR("Error reading upgrade spec: No Access");
            return SDK_RET_ERR;
        }
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Error reading upgrade spec:\n %s\n", ex.what());
        UPG_TRACE_ERR("Error reading upgrade spec:\n %s\n", ex.what());
    }
    return ret;
}

void
init (void* ctxt)
{
    pt::ptree tree;

    if (SDK_RET_OK != load_pipeline_json(tree, true)){
        UPG_TRACE_ERR("Failed to load upgrade json !\n");
        exit(1);
    }

    if (SDK_RET_OK != init_svc(tree)){
        UPG_TRACE_ERR("Failed to init service objects !\n");
        exit(1);
    }

    if (SDK_RET_OK != init_stage_transitions(tree)){
        UPG_TRACE_ERR("Failed to init stage transition table !\n");
        exit(1);
    }

    if (SDK_RET_OK != set_entry_stage(tree)){
        UPG_TRACE_ERR("Failed to set entry stage !\n");
        exit(1);
    }

    if (SDK_RET_OK != set_event_sequence(tree)){
        UPG_TRACE_ERR("Failed to set default event sequence !\n");
        exit(1);
    }

    if (SDK_RET_OK != init_fsm_stages(tree)){
        UPG_TRACE_ERR("Failed to set init stages !\n");
        exit(1);
    }

    init_fsm(ctxt);
}

}    // namespace upg
}    // namespace sdk
