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
#include "include/sdk/base.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "fsm.hpp"
#include "ipc.hpp"
#include "stage.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "upgrade/include/ev.hpp"
#include "upgrade/include/upgrade.hpp"

namespace sdk {
namespace upg {

static bool stage_in_progress = false;

static bool
invoke_hooks (upg_stage_t stage_id, hook_execution_t hook_type,
              svc_rsp_code_t status=SVC_RSP_MAX)
{
    upg_stage stage;
    upg_scripts hooks;
    std::string name;
    bool result = true;
    name = upg_stage2str(stage_id);
    stage = fsm_stages[stage_id];

    if (hook_type == PRE_STAGE) {
        hooks = stage.pre_hook_scripts();
    } else {
        hooks = stage.post_hook_scripts();
    }

    for (const auto& x : hooks) {
        if (!is_valid_script(fsm_states.init_params()->tools_dir, x.path())) {
            UPG_TRACE_ERR("Not a valid Script %s", x.path().c_str());
            result = false;
            break;
        }
        if (!execute_hook(fsm_states.init_params()->tools_dir,
                          x.path(), name, fsm_states.init_params()->fw_pkgname,
                          hook_type, status)) {
            UPG_TRACE_ERR("Failed to execute Script %s", x.path().c_str());
            result = false;
            break;
        }
    }
    return result;
}

static bool
execute_pre_hooks (upg_stage_t stage_id)
{
    return invoke_hooks(stage_id, PRE_STAGE);
}

static bool
execute_post_hooks (upg_stage_t stage_id, svc_rsp_code_t status)
{
    SDK_ASSERT(status != SVC_RSP_MAX);
    return invoke_hooks(stage_id, POST_STAGE, status);
}


static void
dispatch_event (ipc_svc_dom_id_t dom, upg_stage_t id, upg_svc svc)
{
    std::string stage_name(upg_stage2str(id));
    UPG_TRACE_INFO("Sending event %s to service %s IPC ID %u",
                      stage_name.c_str(), svc.name().c_str(), svc.ipc_id());
    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());

    if (svc.has_valid_ipc_id()) {
        svc.dispatch_event(dom, id, fsm_states.init_params()->upg_mode);
    }
}

static void
dispatch_event (ipc_svc_dom_id_t dom, upg_stage_t id)
{
    std::string stage_name(upg_stage2str(id));
    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());
    UPG_TRACE_INFO("Sending parallel event %s", stage_name.c_str());

    for (auto& name : fsm_states.svc_sequence()) {
        SDK_ASSERT(fsm_services.find(name) != fsm_services.end());
        upg_svc svc = fsm_services[name];
        if (svc.has_valid_ipc_id()) {
            dispatch_event(dom, id, svc);
        } else {
            fsm_states.skip_svc();
            UPG_TRACE_WARN("Service %s does't have a valid IPC ID",
                           name.c_str());
        }
    }
}

static void
send_discovery_event (ipc_svc_dom_id_t dom, upg_stage_t id)
{
    std::string stage_name(upg_stage2str(id));
    SDK_ASSERT(fsm_stages.find(id) != fsm_stages.end());
    UPG_TRACE("Sending discovery event %s", stage_name.c_str());

    upg_send_broadcast_request(dom, id, fsm_states.init_params()->upg_mode,
                               fsm_services.size(), fsm_states.timeout());
}

static void
send_ipc_to_next_service (void)
{
    ipc_svc_dom_id_t domain = fsm_states.domain();

    UPG_TRACE("Sending serial request");
    if (!fsm_states.has_next_svc()) {
        // Stage must have pending svc
        // if it is not over yet
        SDK_ASSERT(0);
    } else {
        upg_stage_t id = fsm_states.current_stage();
        std::string name(upg_stage2str(fsm_states.current_stage()));

        while (fsm_states.has_next_svc()) {
            std::string svc_name = fsm_states.next_svc();
            SDK_ASSERT(fsm_services.find(svc_name) != fsm_services.end());
            upg_svc svc = fsm_services[svc_name];
            if (svc.has_valid_ipc_id()) {
                fsm_states.timer_stop();
                dispatch_event(domain, id, svc);
                fsm_states.timer_start();
                UPG_TRACE_INFO("Sending %s serial event to %s, IPC ID %u",
                               name.c_str(), svc_name.c_str(), svc.ipc_id());
                break;
            } else {
                UPG_TRACE_WARN("Not sending %s serial event to %s, IPC ID %u."
                               " Not a valid IPC ID", name.c_str(),
                               svc_name.c_str(), svc.ipc_id());
                fsm_states.skip_svc();
            }
        }
    }
}

sdk_ret_t
upg_interactive_stage_exec (upg_stage_t stage)
{
    if (stage_in_progress) {
        UPG_TRACE_ERR("Upgrade stage is in progress");
        return SDK_RET_ERR;
    }
    stage_in_progress = true;
    // todo:
    fsm_states.set_current_stage(stage);
    upg_stage_t id = fsm_states.current_stage();
    std::string name(upg_stage2str(fsm_states.current_stage()));
    ipc_svc_dom_id_t dom = fsm_states.domain();

    fsm_states.timer_stop();
    fsm_states.timer_start();
    if (!execute_pre_hooks(id)) {
        UPG_TRACE_ERR("Failed to execute pre hooks in stage %s",
                      upg_stage2str(id));
        fsm_states.update_stage_progress(SVC_RSP_FAIL);
    }

    if (fsm_states.is_discovery()) {
        UPG_TRACE_INFO("Sending discovery event %s", name.c_str());
        dump(fsm_states);
        send_discovery_event (dom, id);
    } else if (fsm_states.is_serial_event_sequence() &&
               fsm_states.has_next_svc()) {
        UPG_TRACE_INFO("Sending serial interactive event %s", name.c_str());
        dump(fsm_states);
        send_ipc_to_next_service();
    } else if (fsm_states.is_parallel_event_sequence()) {
        UPG_TRACE_INFO("Sending parallel interactive event %s", name.c_str());
        dump(fsm_states);
        dispatch_event(dom, id);
    } else {
        SDK_ASSERT(0);
    }
    return SDK_RET_IN_PROGRESS;
}

static void
update_ipc_id (std::string name, uint32_t ipc_id)
{
    SDK_ASSERT(fsm_services.find(name) != fsm_services.end());

    UPG_TRACE_INFO("Updating IPC ID of service %s, IPC ID %u ",
                      name.c_str(), ipc_id);
    upg_svc service = fsm_services[name];
    service.set_ipc_id(ipc_id);
    fsm_services[name] = service;
}

void
upg_event_interactive_handler (upg_event_msg_t *event)
{
    UPG_TRACE_INFO("IPC response event interactive handler called");

    upg_stage_t id = fsm_states.current_stage();
    std::string svc_name = event->rsp_svc_name;

    UPG_TRACE_INFO("Received UPG interactive IPC event response %s,"
                   " status %s, service %s, ipc id %u",
                   upg_stage2str(event->stage),
                   upg_status2str(event->rsp_status), event->rsp_svc_name,
                   event->rsp_svc_ipc_id);

    if (event->stage == id && fsm_states.is_valid_service(svc_name)) {
        if (fsm_states.is_discovery()) {
            update_ipc_id(svc_name, event->rsp_svc_ipc_id);
        }

        fsm_states.update_stage_progress_interactive(svc_rsp_code
                                                     (event->rsp_status));
        fsm_states.timer_stop();
        if (fsm_states.is_current_stage_over(event->stage)) {
            if (!execute_post_hooks(id, svc_rsp_code(event->rsp_status))) {
                UPG_TRACE_ERR("Failed to execute post hooks in stage %s",
                              upg_stage2str(id));
                UPG_TRACE_ERR("Setting stage progress failure for stage %s",
                              upg_stage2str(id));
                //  todo: response ipc with failure
            } else {
                if (svc_rsp_code(event->rsp_status) != SVC_RSP_OK) {
                    // todo: response ipc with failure
                } else {
                    // todo: response ipc with success
                }
            }
            stage_in_progress = false;
        } else if (fsm_states.is_serial_event_sequence() &&
                   fsm_states.has_next_svc()) {
            send_ipc_to_next_service();
        } else {
            UPG_TRACE_INFO("Interactive stage progress is updated");
        }
    } else {
        UPG_TRACE_WARN("Dropping event response, not expecting it !");
    }

    UPG_TRACE_INFO("Done with IPC response interactive event handler");

}

static upg_status_t
get_exit_status (const svc_rsp_code_t rsp)
{
    return rsp == SVC_RSP_OK ? UPG_STATUS_OK : rsp == SVC_RSP_CRIT ?
        UPG_STATUS_CRITICAL : UPG_STATUS_FAIL;
}

void
fsm::update_stage_progress_interactive(const svc_rsp_code_t rsp) {
    if (rsp != SVC_RSP_OK) {
        switch (rsp) {
        case SVC_RSP_FAIL:
            UPG_TRACE_ERR("Got failure response in stage %s",
                          upg_stage2str(current_stage_));
            break;
        case SVC_RSP_CRIT:
            UPG_TRACE_ERR("Got critical service response in stage %s",
                          upg_stage2str(current_stage_));
            break;
        case SVC_RSP_NONE:
            UPG_TRACE_ERR("Timer expired, no service response so far"
                          " in stage %s",upg_stage2str(current_stage_));
            break;
        default:
            break;
        }
        if (!execute_post_hooks(current_stage_, rsp)) {
            UPG_TRACE_ERR("Failed to execute post hooks in stage %s",
                          upg_stage2str(current_stage_));
        }

        fsm_states.timer_stop();
        fsm_states.init_params()->fsm_completion_cb(get_exit_status(rsp));
        stage_in_progress = false;
    } else {
        std::string str = "Event is successfully handled.";
        pending_response_--;
        str += " Pending response " + std::to_string(pending_response_);

        SDK_ASSERT(pending_response_ >= 0);

        if (pending_response_ == 0) {
            str += ". Finish current stage";
            UPG_TRACE_INFO("%s", str.c_str());
            fsm_states.init_params()->fsm_completion_cb(UPG_STATUS_OK);
            stage_in_progress = false;
        } else {
            UPG_TRACE_INFO("%s", str.c_str());
        }
    }
}

static void
timeout_cb (EV_P_ ev_timer *w, int revents)
{
    fsm_states.timer_stop();
    fsm_states.update_stage_progress_interactive(SVC_RSP_NONE);
}

sdk_ret_t
init_interactive (fsm_init_params_t *params)
{
    UPG_TRACE_INFO("Initializing upgrade manager with interactive mode");
    SDK_ASSERT(params != NULL);
    SDK_ASSERT(UPG_STAGE_NONE == params->entry_stage);

    upg_ipc_init(upg_event_interactive_handler);
    loop = params->ev_loop;
    // todo: fsm_states.timer_init(params->ev_loop);
    ev_timer_init(&timeout_watcher, timeout_cb, fsm_states.timeout(), 0.0);
    fsm_states.set_init_params(params);
    return SDK_RET_OK;
}

}
}
