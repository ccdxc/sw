//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nicmgr functionality
///
//----------------------------------------------------------------------------

#include <pthread.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/utils/port_utils.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/internal/upgrade_ev.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/upgrade.hpp"

/// \defgroup PDS_NICMGR
/// @{

DeviceManager *g_devmgr;
sdk::event_thread::prepare_t g_ev_prepare;

namespace nicmgr {

// ipc incoming msg pointer which should be saved and later
// used to respond to the caller
typedef struct upg_ev_info_s {
    sdk::ipc::ipc_msg_ptr msg_in;
} upg_ev_info_t;

using api::upg_ev_params_t;

static void
prepare_callback (sdk::event_thread::prepare_t *prepare, void *ctx)
{
    fflush(stdout);
    fflush(stderr);
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}

static void
nicmgr_upg_ev_response_hdlr (sdk_ret_t status, void *cookie)
{
    upg_ev_info_t *info = (upg_ev_info_t *)cookie;
    upg_ev_params_t *params = (upg_ev_params_t *)info->msg_in->data();
    upg_ev_params_t resp;

    PDS_TRACE_DEBUG("Upgrade nicmgr IPC response type %s status %u",
                    upg_msgid2str(params->id), status);

    if (status == SDK_RET_IN_PROGRESS) {
        return;
    }

    resp.id = params->id;
    resp.rsp_code = status;
    sdk::ipc::respond(info->msg_in, (const void *)&resp, sizeof(resp));
    delete info;
}

// callback handler from nicmgr library when the hostdev
// reset is completed. some pipelines may need this
static int
nicmgr_device_reset_status_hdlr (void)
{
    return 0;
}

static void
nicmgr_upg_ev_compat_check_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_info_s *info = new upg_ev_info_t();
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Upgrade nicmgr IPC request compat check");
    info->msg_in = msg;
    ret = nicmgr::lib::upg_compat_check_handler();
    return nicmgr_upg_ev_response_hdlr(ret, info);
}

static void
nicmgr_upg_ev_backup_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_info_s *info = new upg_ev_info_t();

    PDS_TRACE_DEBUG("Upgrade nicmgr IPC request backup");
    info->msg_in = msg;
    // TODO
    return nicmgr_upg_ev_response_hdlr(SDK_RET_OK, info);
}

static void
nicmgr_upg_ev_link_down_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_info_s *info = new upg_ev_info_t();
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Upgrade nicmgr IPC request linkdown");
    info->msg_in = msg;
    ret = nicmgr::lib::upg_link_down_handler(info);
    return nicmgr_upg_ev_response_hdlr(ret, info);
}

static void
nicmgr_upg_ev_hostdev_reset_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_info_s *info = new upg_ev_info_t();
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Upgrade nicmgr IPC request hostdev reset");
    info->msg_in = msg;
    ret = nicmgr::lib::upg_host_down_handler(info);
    return nicmgr_upg_ev_response_hdlr(ret, info);
}

static void
nicmgr_upg_ev_repeal_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_info_s *info = new upg_ev_info_t();
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Upgrade nicmgr IPC request repeal");
    info->msg_in = msg;
    ret = nicmgr::lib::upg_failed_handler(info);
    return nicmgr_upg_ev_response_hdlr(ret, info);
}

static void
upg_ipc_register (void)
{
    sdk::ipc::reg_request_handler(UPG_MSG_ID_COMPAT_CHECK,
                                  nicmgr_upg_ev_compat_check_hdlr, NULL);
    sdk::ipc::reg_request_handler(UPG_MSG_ID_BACKUP,
                                  nicmgr_upg_ev_backup_hdlr, NULL);
    sdk::ipc::reg_request_handler(UPG_MSG_ID_LINK_DOWN,
                                  nicmgr_upg_ev_link_down_hdlr, NULL);
    sdk::ipc::reg_request_handler(UPG_MSG_ID_HOSTDEV_RESET,
                                  nicmgr_upg_ev_hostdev_reset_hdlr, NULL);
    sdk::ipc::reg_request_handler(UPG_MSG_ID_REPEAL,
                                  nicmgr_upg_ev_repeal_hdlr, NULL);
}

void
nicmgrapi::nicmgr_thread_init(void *ctxt) {
    pds_state *state;
    string device_cfg_file;
    sdk::event_thread::event_thread *curr_thread;
    devicemgr_cfg_t cfg;
    bool init_pci = sdk::platform::upgrade_mode_none(
        api::g_upg_state->upg_init_mode());

    // get pds state
    state = (pds_state *)sdk::lib::thread::current_thread()->data();
    curr_thread = (sdk::event_thread::event_thread *)ctxt;

    // compute the device profile json file to be used
    if (state->platform_type() == platform_type_t::PLATFORM_TYPE_SIM) {
        device_cfg_file =
            state->cfg_path() + "/" + state->pipeline() + "/device.json";
    } else {
        if (state->device_profile() == PDS_DEVICE_PROFILE_DEFAULT) {
            device_cfg_file = state->cfg_path() + "/device.json";
        } else {
            device_cfg_file =
                state->cfg_path() + "/device-" + state->device_profile_string() + ".json";
        }
    }

    // instantiate the logger
    utils::logger::init();

    // initialize device manager
    PDS_TRACE_INFO("Initializing device manager ...");
    cfg.platform_type = state->platform_type();
    cfg.cfg_path = state->cfg_path();
    cfg.device_conf_file = "";
    cfg.fwd_mode = sdk::lib::FORWARDING_MODE_NONE;
    cfg.micro_seg_en = false;
    cfg.shm_mgr = NULL;
    cfg.pipeline = state->pipeline();
    cfg.memory_profile = state->memory_profile_string();
    cfg.device_profile = state->device_profile_string();
    cfg.catalog = state->catalogue();
    cfg.EV_A = curr_thread->ev_loop();

    g_devmgr = new DeviceManager(&cfg);
    SDK_ASSERT(g_devmgr);
    g_devmgr->LoadProfile(device_cfg_file, init_pci);

    sdk::ipc::subscribe(EVENT_ID_PORT_STATUS, port_event_handler_, NULL);
    sdk::ipc::subscribe(EVENT_ID_XCVR_STATUS, xcvr_event_handler_, NULL);
    sdk::ipc::subscribe(EVENT_ID_PDS_HAL_UP, hal_up_event_handler_, NULL);
    sdk::event_thread::prepare_init(&g_ev_prepare, prepare_callback, NULL);
    sdk::event_thread::prepare_start(&g_ev_prepare);

    // register for upgrade events
    upg_ipc_register();

    // register the async handlers to nicmgr library
    nicmgr::lib::upg_ev_init(nicmgr_device_reset_status_hdlr,
                             nicmgr_upg_ev_response_hdlr);

    PDS_TRACE_INFO("Listening to events ...");
}

//------------------------------------------------------------------------------
// nicmgr thread cleanup
//------------------------------------------------------------------------------
void
nicmgrapi::nicmgr_thread_exit(void *ctxt) {
    delete g_devmgr;
    sdk::event_thread::prepare_stop(&g_ev_prepare);
}

void
nicmgrapi::nicmgr_event_handler(void *msg, void *ctxt) {
}

void
nicmgrapi::hal_up_event_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {
    // create mnets
    PDS_TRACE_INFO("Creating mnets ...");
    g_devmgr->HalEventHandler(true);
}

void
nicmgrapi::port_event_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {
    port_status_t st = { 0 };
    core::event_t *event = (core::event_t *)msg->data();

    st.id = event->port.ifindex;
    st.status =
        (event->port.event == port_event_t::PORT_EVENT_LINK_UP) ? 1 : 0;
    st.speed = sdk::lib::port_speed_enum_to_mbps(event->port.speed);
    st.fec_type = (uint8_t)event->port.fec_type;
    PDS_TRACE_DEBUG("Rcvd port event for ifidx 0x%x, speed %u, status %u "
                    " fec_type %u", st.id, st.speed, st.status, st.fec_type);
    g_devmgr->LinkEventHandler(&st);
}

void
nicmgrapi::xcvr_event_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {
    port_status_t st = { 0 };
    core::event_t *event = (core::event_t *)msg->data();

    st.id = event->port.ifindex;
    st.xcvr.state = event->xcvr.state;
    st.xcvr.pid = event->xcvr.pid;
    st.xcvr.phy = event->xcvr.cable_type;
    memcpy(st.xcvr.sprom, event->xcvr.sprom, XCVR_SPROM_SIZE);
    g_devmgr->XcvrEventHandler(&st);
    PDS_TRACE_DEBUG("Rcvd xcvr event for ifidx 0x%x, state %u, cable type %u"
                    "pid %u", st.id, st.xcvr.state, st.xcvr.phy, st.xcvr.pid);
    g_devmgr->LinkEventHandler(&st);
}

DeviceManager *
nicmgrapi::devmgr_if(void) {
    return g_devmgr;
}

}    // namespace nicmgr

/// \@}

