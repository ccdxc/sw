// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "asic/asic.hpp"
#include "platform/drivers/xcvr.hpp"
#include "port_serdes.hpp"
#include "lib/pal/pal.hpp"
#include "platform/pal/include/pal.h"
#include "lib/thread/thread.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "lib/ipc/ipc.hpp"
#include "linkmgr.hpp"
#include "linkmgr_state.hpp"
#include "linkmgr_internal.hpp"
#include "port.hpp"
#include "include/sdk/port_utils.hpp"

using namespace sdk::event_thread;
using namespace sdk::ipc;

namespace sdk {
namespace linkmgr {

static int aacs_server_port_num = -1;
bool hal_cfg = false;

// global log buffer
char log_buf[MAX_LOG_SIZE];

// global sdk-linkmgr state
linkmgr_state *g_linkmgr_state;

// global sdk-linkmgr config
linkmgr_cfg_t g_linkmgr_cfg;

// sdk-linkmgr threads
sdk::lib::thread *g_linkmgr_threads[LINKMGR_THREAD_ID_MAX];

// xcvr poll timer handle
sdk::event_thread::timer_t xcvr_poll_timer_handle;

// link down poll list
sdk::event_thread::timer_t port_link_poll_timer_handle;
port *link_poll_timer_list[MAX_LOGICAL_PORTS];

// Global setting for link status poll. Default is enabled.
bool port_link_poll_en = true;

bool
port_link_poll_enabled (void)
{
    return port_link_poll_en;
}

void
linkmgr_set_link_poll_enable (bool enable)
{
    port_link_poll_en = enable;
}

uint32_t
glbl_mode_mgmt (mac_mode_t mac_mode)
{
    return g_linkmgr_cfg.catalog->glbl_mode_mgmt(mac_mode);
}

uint32_t
ch_mode_mgmt (mac_mode_t mac_mode, uint32_t ch)
{
    return g_linkmgr_cfg.catalog->ch_mode_mgmt(mac_mode, ch);
}

uint32_t
glbl_mode (mac_mode_t mac_mode)
{
    return g_linkmgr_cfg.catalog->glbl_mode(mac_mode);
}

uint32_t
ch_mode (mac_mode_t mac_mode, uint32_t ch)
{
    return g_linkmgr_cfg.catalog->ch_mode(mac_mode, ch);
}

uint32_t
num_asic_ports(uint32_t asic)
{
    return g_linkmgr_cfg.catalog->num_asic_ports(asic);
}

uint32_t
sbus_addr_asic_port(uint32_t asic, uint32_t asic_port)
{
    return g_linkmgr_cfg.catalog->sbus_addr_asic_port(asic, asic_port);
}

uint32_t
jtag_id(void)
{
    return g_linkmgr_cfg.catalog->jtag_id();
}

uint8_t
sbm_clk_div(void)
{
    return g_linkmgr_cfg.catalog->sbm_clk_div();
}

uint8_t
num_sbus_rings(void)
{
    return g_linkmgr_cfg.catalog->num_sbus_rings();
}

uint32_t
logical_port_to_tm_port (uint32_t logical_port)
{
    return g_linkmgr_cfg.catalog->logical_port_to_tm_port(logical_port);
}

bool
aacs_server_en(void)
{
    uint8_t en = g_linkmgr_cfg.catalog->aacs_server_en();
    if (en == 1) {
        return true;
    }

    return false;
}

bool
aacs_connect(void)
{
    uint8_t connect = g_linkmgr_cfg.catalog->aacs_connect();
    if (connect == 1) {
        return true;
    }

    return false;
}

uint32_t
aacs_server_port(void)
{
    return g_linkmgr_cfg.catalog->aacs_server_port();
}

std::string
aacs_server_ip(void)
{
    return g_linkmgr_cfg.catalog->aacs_server_ip();
}

serdes_info_t*
serdes_info_get(uint32_t sbus_addr,
                uint32_t port_speed,
                uint32_t cable_type)
{
    return g_linkmgr_cfg.catalog->serdes_info_get(sbus_addr,
                                               port_speed,
                                               cable_type);
}

int
serdes_build_id (void)
{
    return g_linkmgr_cfg.catalog->serdes_build_id();
}

int
serdes_rev_id (void)
{
    return g_linkmgr_cfg.catalog->serdes_rev_id();
}

std::string
serdes_fw_file (void)
{
    return g_linkmgr_cfg.catalog->serdes_fw_file();
}

sdk_ret_t
port_link_poll_timer_add(port *port)
{
    link_poll_timer_list[port->port_num() - 1] = port;
    return SDK_RET_OK;
}

sdk_ret_t
port_link_poll_timer_delete(port *port)
{
    link_poll_timer_list[port->port_num() - 1] = NULL;
    return SDK_RET_OK;
}

sdk::lib::thread *
current_thread (void)
{
    return sdk::lib::thread::current_thread() ?
                       sdk::lib::thread::current_thread() :
                       g_linkmgr_threads[LINKMGR_THREAD_ID_CFG];
}

bool
is_linkmgr_ctrl_thread (void)
{
    bool is_ctrl_thread;

    sdk::lib::thread *curr_thread = current_thread();
    sdk::lib::thread *ctrl_thread = g_linkmgr_threads[LINKMGR_THREAD_ID_CTRL];

    // TODO assert once the thread store is fixed
    if (curr_thread == NULL) {
        is_ctrl_thread = false;
        goto end;
    }

    // if curr_thread/ctrl_thread is NULL, then init has failed or not invoked
    if (ctrl_thread == NULL) {
        SDK_ASSERT_GOTO(0, end);
    }

    if (curr_thread->thread_id() == ctrl_thread->thread_id()) {
        is_ctrl_thread = true;
        goto end;
    }

    is_ctrl_thread = false;
end:
    // SDK_TRACE_DEBUG("ctrl thread %s", is_ctrl_thread == true? "true" : "false");
    return is_ctrl_thread;
}

bool
is_linkmgr_ctrl_thread_ready (void)
{
    return (g_linkmgr_threads[LINKMGR_THREAD_ID_CTRL] &&
            g_linkmgr_threads[LINKMGR_THREAD_ID_CTRL]->ready());
}

void
xcvr_poll_timer_cb (sdk::event_thread::timer_t *timer)
{
    sdk::platform::xcvr_poll_timer();
}

void
port_link_poll_timer_cb (sdk::event_thread::timer_t *timer)
{
    if (port_link_poll_enabled() == true) {
        for (int i = 0; i < MAX_LOGICAL_PORTS; ++i) {
            port *port_p = link_poll_timer_list[i];

            if (port_p != NULL) {
                if(port_p->port_link_status() == false) {
                    SDK_TRACE_DEBUG("%d: Link DOWN", port_p->port_num());
                    port_p->port_link_dn_handler();
                }
            }
        }
    }
}

static sdk_ret_t
port_event_enable (linkmgr_entry_data_t *data)
{
    port *port_p = (port *)data->ctxt;
    return port_p->port_enable(true);
}

static sdk_ret_t
port_event_disable (linkmgr_entry_data_t *data)
{
    port *port_p = (port *)data->ctxt;
    return port_p->port_disable();
}

static void
port_enable_req_handler (ipc_msg_ptr msg, const void *ctx)
{
    SDK_ATOMIC_STORE_BOOL(&hal_cfg, false);
    port_event_enable((linkmgr_entry_data_t *)msg->data());

    // send response back to blocked caller
    respond(msg, NULL, 0);
}

static void
port_rsp_handler (ipc_msg_ptr msg, const void *request_cookie)
{
}

static void
port_disable_req_handler (ipc_msg_ptr msg, const void *ctx)
{
    SDK_ATOMIC_STORE_BOOL(&hal_cfg, false);
    port_event_disable((linkmgr_entry_data_t *)msg->data());

    // send response back to blocked caller
    respond(msg, NULL, 0);
}

void
port_bringup_timer_cb (sdk::event_thread::timer_t *timer)
{
    uint32_t  retries = 0;
    port      *port_p = (port *)timer->ctx;

    // stop the repeated timer since timer_again is being used
    // to schedule timer
    timer_stop(timer);

    // if the bringup timer has expired, reset and try again
    if (port_p->bringup_timer_expired() == true) {
        retries = port_p->num_retries();

        // port disable resets num_retries
        port_p->port_disable();

        // Enable port only if max retries is not reached
        if (1 || retries < MAX_PORT_LINKUP_RETRIES) {
            port_p->set_num_retries(retries + 1);
            port_p->port_enable();
        } else {
            // TODO notify upper layers?
            SDK_TRACE_DEBUG("Max retries: %d reached for port: %d",
                            MAX_PORT_LINKUP_RETRIES, port_p->port_num());
        }
    } else {
        port_p->port_link_sm_process();
    }
}

void
port_debounce_timer_cb (sdk::event_thread::timer_t *timer)
{
    port *port_p = (port *)timer->ctx;

    // stop the repeated timer since timer_again is being used
    // to schedule timer
    timer_stop(timer);

    port_p->port_debounce_timer_cb();
}

//------------------------------------------------------------------------------
// linkmgr thread notification by other threads
//------------------------------------------------------------------------------
sdk_ret_t
linkmgr_notify (uint8_t operation, linkmgr_entry_data_t *data,
                q_notify_mode_t mode)
{
    // notify the ctrl thread about the config pending
    switch (operation) {
        case LINKMGR_OPERATION_PORT_ENABLE:
        case LINKMGR_OPERATION_PORT_DISABLE:
            SDK_ATOMIC_STORE_BOOL(&hal_cfg, true);
            break;

        default:
            break;
    }

    request(LINKMGR_THREAD_ID_CTRL, operation, data,
            sizeof(linkmgr_entry_data_t), port_rsp_handler, NULL);
    return SDK_RET_OK;
}

static sdk_ret_t
linkmgr_timers_init (void)
{
    // init the transceiver poll timer
    timer_init(&xcvr_poll_timer_handle, xcvr_poll_timer_cb,
               XCVR_POLL_TIME, XCVR_POLL_TIME);
    timer_start(&xcvr_poll_timer_handle);

    // init the link poll timer
    timer_init(&port_link_poll_timer_handle, port_link_poll_timer_cb,
               LINKMGR_LINK_POLL_TIME, LINKMGR_LINK_POLL_TIME);
    timer_start(&port_link_poll_timer_handle);

    SDK_TRACE_DEBUG("starting poll timers");

    return SDK_RET_OK;
}

static sdk_ret_t
xcvr_poll_init (linkmgr_cfg_t *cfg)
{
    sdk::platform::xcvr_init(cfg->xcvr_event_cb);
    return SDK_RET_OK;
}

static void
linkmgr_event_thread_init (void *ctxt)
{
    int         exp_build_id = serdes_build_id();
    int         exp_rev_id   = serdes_rev_id();
    std::string cfg_file     = "fw/" + serdes_fw_file();

    cfg_file = std::string(g_linkmgr_cfg.cfg_path) + "/" + cfg_file;

    pal_wr_lock(SBUSLOCK);

    // TODO move back to serdes_fn_init
    serdes_get_ip_info(1);

    serdes_sbm_set_sbus_clock_divider(sbm_clk_div());

    for (uint32_t asic_port = 0; asic_port < num_asic_ports(0); ++asic_port) {
        uint32_t sbus_addr = sbus_addr_asic_port(0, asic_port);

        if (sbus_addr == 0) {
            continue;
        }

        sdk::linkmgr::serdes_fns.serdes_spico_upload(sbus_addr, cfg_file.c_str());

        int build_id = sdk::linkmgr::serdes_fns.serdes_get_build_id(sbus_addr);
        int rev_id   = sdk::linkmgr::serdes_fns.serdes_get_rev(sbus_addr);

        if (build_id != exp_build_id || rev_id != exp_rev_id) {
            SDK_TRACE_DEBUG("sbus_addr: 0x%x,"
                            " build_id: 0x%x, exp_build_id: 0x%x,"
                            " rev_id: 0x%x, exp_rev_id: 0x%x",
                            sbus_addr, build_id, exp_build_id,
                            rev_id, exp_rev_id);
            // TODO fail if no match
        }

        sdk::linkmgr::serdes_fns.serdes_spico_status(sbus_addr);

        SDK_TRACE_DEBUG("sbus_addr: 0x%x, spico_crc: %d",
                        sbus_addr,
                        sdk::linkmgr::serdes_fns.serdes_spico_crc(sbus_addr));
    }

    pal_wr_unlock(SBUSLOCK);

    srand(time(NULL));

    // initialize xcvr polling
    xcvr_poll_init(&g_linkmgr_cfg);

    // init the linkmgr timers
    linkmgr_timers_init();

    reg_request_handler(LINKMGR_OPERATION_PORT_ENABLE,
                        port_enable_req_handler, NULL);
    reg_request_handler(LINKMGR_OPERATION_PORT_DISABLE,
                        port_disable_req_handler, NULL);
}

static void
linkmgr_event_thread_exit (void *ctxt)
{
}

static void
linkmgr_event_handler (void *msg, void *ctxt)
{
}

static sdk_ret_t
event_thread_init (linkmgr_cfg_t *cfg)
{
    return SDK_RET_OK;
}

static sdk_ret_t
thread_init (linkmgr_cfg_t *cfg)
{
    int thread_id = 0;
    sdk::event_thread::event_thread *new_thread;

    event_thread_init(cfg);

    // init the control thread
    thread_id = LINKMGR_THREAD_ID_CTRL;
    new_thread =
        sdk::event_thread::event_thread::factory(
            "linkmgr-ctrl", thread_id,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            linkmgr_event_thread_init,
            linkmgr_event_thread_exit,
            linkmgr_event_handler,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);
    SDK_ASSERT_TRACE_RETURN((new_thread != NULL), SDK_RET_ERR,
                            "linkmgr-ctrl thread create failure");
    g_linkmgr_threads[thread_id] = new_thread;

    new_thread->start(new_thread);
    return SDK_RET_OK;
}

void
linkmgr_threads_stop (void)
{
    int thread_id;

    for (thread_id = 0; thread_id < LINKMGR_THREAD_ID_MAX; thread_id++) {
        if (g_linkmgr_threads[thread_id] != NULL) {
            // stop the thread
            SDK_TRACE_DEBUG("Stopping thread %s", g_linkmgr_threads[thread_id]->name());
            g_linkmgr_threads[thread_id]->stop();
        }
    }
}

void
linkmgr_threads_wait (void)
{
    int thread_id;

    for (thread_id = 0; thread_id < LINKMGR_THREAD_ID_MAX; thread_id++) {
        if (g_linkmgr_threads[thread_id] != NULL) {
            SDK_TRACE_DEBUG("Waiting thread %s to exit", g_linkmgr_threads[thread_id]->name());
            g_linkmgr_threads[thread_id]->wait();
            // free the allocated thread
            sdk::lib::thread::destroy(g_linkmgr_threads[thread_id]);
            g_linkmgr_threads[thread_id] = NULL;
        }
    }
}

sdk_ret_t
linkmgr_init (linkmgr_cfg_t *cfg)
{
    sdk_ret_t    ret = SDK_RET_OK;

    g_linkmgr_cfg = *cfg;

    g_linkmgr_state = linkmgr_state::factory();
    if (NULL == g_linkmgr_state) {
        SDK_TRACE_ERR("linkmgr init failed");
        return SDK_RET_ERR;
    }

    // initialize the port mac and serdes functions
    port::port_init(cfg);

    if ((ret = thread_init(cfg)) != SDK_RET_OK) {
        SDK_TRACE_ERR("linkmgr thread init failed");
        return ret;
    }

    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// Validate speed against number of lanes
//-----------------------------------------------------------------------------
static bool
validate_speed_lanes (port_speed_t speed, uint32_t num_lanes)
{
    bool valid = true;

    // speed = NONE implies no update
    switch (num_lanes) {
    case 4:
        switch (speed) {
            case port_speed_t::PORT_SPEED_100G:
            case port_speed_t::PORT_SPEED_40G:
            case port_speed_t::PORT_SPEED_NONE:
                break;

            default:
                valid = false;
                break;
        }
        break;

    case 2:
        switch (speed) {
            case port_speed_t::PORT_SPEED_50G:
            case port_speed_t::PORT_SPEED_NONE:
                break;

            default:
                valid = false;
                break;
        }
        break;

    case 1:
        switch (speed) {
            case port_speed_t::PORT_SPEED_25G:
            case port_speed_t::PORT_SPEED_10G:
            case port_speed_t::PORT_SPEED_1G:
            case port_speed_t::PORT_SPEED_NONE:
                break;

            default:
                valid = false;
                break;
        }
        break;

    default:
        valid = false;
        break;
    }

    if (valid == false) {
        SDK_TRACE_ERR("Invalid speed and lanes config."
                      " num_lanes: %d, speed: %d",
                      num_lanes, speed);
    }

    return valid;
}

void
port_set_leds (uint32_t port_num, port_event_t event)
{
    int phy_port = sdk::lib::catalog::logical_port_to_phy_port(port_num);

    if (phy_port == -1) {
        return;
    }

    switch (event) {
    case port_event_t::PORT_EVENT_LINK_UP:
        sdk::lib::pal_qsfp_set_led(phy_port,
                                   pal_led_color_t::LED_COLOR_GREEN);
        break;

    case port_event_t::PORT_EVENT_LINK_DOWN:
        sdk::lib::pal_qsfp_set_led(phy_port,
                                   pal_led_color_t::LED_COLOR_NONE);
        break;

    default:
        break;
    }
}

sdk_ret_t
port_update_xcvr_event (void *pd_p, xcvr_event_info_t *xcvr_event_info)
{
    sdk_ret_t   sdk_ret   = SDK_RET_OK;
    port_args_t port_args = { 0 };

    sdk::linkmgr::port_args_init(&port_args);

    sdk_ret = port_get(pd_p, &port_args);
    if (sdk_ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to get pd for port, err: %u", sdk_ret);
        return sdk_ret;
    }

    int phy_port =
        sdk::lib::catalog::logical_port_to_phy_port(port_args.port_num);

    SDK_TRACE_DEBUG("port: %u, phy_port: %u, xcvr_event_port: %u, "
                    "xcvr_state: %u, user_admin: %u, admin: %u, "
                    "AN_cfg: %u, AN_enable: %u, num_lanes_cfg: %u",
                    port_args.port_num, phy_port, xcvr_event_info->phy_port,
                    static_cast<uint32_t>(xcvr_event_info->state),
                    static_cast<uint32_t>(port_args.user_admin_state),
                    static_cast<uint32_t>(port_args.admin_state),
                    port_args.auto_neg_cfg,
                    port_args.auto_neg_enable,
                    port_args.num_lanes_cfg);

    if (phy_port == -1 || phy_port != (int)xcvr_event_info->phy_port) {
        return SDK_RET_OK;
    }

    if (xcvr_event_info->state == xcvr_state_t::XCVR_SPROM_READ) {
        // update cable type
        port_args.cable_type   = xcvr_event_info->cable_type;
        port_args.port_an_args = xcvr_event_info->port_an_args;

        SDK_TRACE_DEBUG("port: %u, phy_port: %u, "
                        "user_cap: %u, fec_ability: %u, fec_request: %u",
                        port_args.port_num,
                        phy_port,
                        port_args.port_an_args->user_cap,
                        port_args.port_an_args->fec_ability,
                        port_args.port_an_args->fec_request);

        // set admin_state based on user configured value
        if (port_args.user_admin_state ==
                port_admin_state_t::PORT_ADMIN_STATE_UP) {
            port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
        }

        // set AN based on user configured value
        port_args.auto_neg_enable = port_args.auto_neg_cfg;

        // set num_lanes based on user configured value
        port_args.num_lanes = port_args.num_lanes_cfg;

        // update port_args based on the xcvr state
        port_args_set_by_xcvr_state(&port_args);

        sdk_ret = port_update(pd_p, &port_args);

        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Failed to update for port: %u, err: %u",
                          port_args.port_num, sdk_ret);
        }
    } else if (xcvr_event_info->state == xcvr_state_t::XCVR_REMOVED) {
        if (port_args.user_admin_state ==
                port_admin_state_t::PORT_ADMIN_STATE_UP) {
            port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_DOWN;

            sdk_ret = port_update(pd_p, &port_args);

            if (sdk_ret != SDK_RET_OK) {
                SDK_TRACE_ERR("Faileu to upuate for port: %u, err: %u",
                              port_args.port_num, sdk_ret);
            }
        }
    }

    return SDK_RET_OK;
}

/// \brief      derive port_speed, fec_type and num_lanes based on transceiver
//  \param[in]  phy_port port representing transceiver
//  \param[out] port_speed derived based on transceiver
//  \param[out] fec_type   derived based on port_speed
//  \param[out] num_lanes  derived based on port_speed
//  \param[out] toggle_fec toggle FEC modes during fixed speed bringup
//  \return     #SDK_RET_OK on success, failure status code on error
static sdk_ret_t
port_args_set_by_xcvr_state_ (int phy_port, port_speed_t *port_speed,
                              port_fec_type_t *fec_type, uint32_t *num_lanes,
                              bool *toggle_fec_mode)
{
    *port_speed = sdk::platform::cable_speed(phy_port-1);

    // TODO FEC assumed based on cable speed
    switch (*port_speed) {
        case port_speed_t::PORT_SPEED_100G:
            *fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
            *num_lanes = 4;
            break;

        case port_speed_t::PORT_SPEED_40G:
            *fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
            *num_lanes = 4;
            break;

        case port_speed_t::PORT_SPEED_25G:
            *toggle_fec_mode = true;
            *fec_type = sdk::platform::xcvr_fec_type(phy_port-1);
            *num_lanes = 1;
            break;

        case port_speed_t::PORT_SPEED_10G:
            *fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
            *num_lanes = 1;
            break;

        default:
            *fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
            SDK_TRACE_ERR("Invalid port speed %u for phy_port %u",
                          *port_speed, phy_port);
            return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// set port args given transceiver state
//-----------------------------------------------------------------------------
sdk_ret_t
port_args_set_by_xcvr_state (port_args_t *port_args)
{
    // if xcvr is inserted:
    //     set port params based on cable
    // else:
    //     set admin_state as ADMIN_DOWN
    //     (only admin_state is down. user_admin_state as per request msg)

    int phy_port =
        sdk::lib::catalog::logical_port_to_phy_port(port_args->port_num);

    port::phy_port_mac_addr(phy_port, port_args->mac_addr);

    // default cable type is CU
    port_args->cable_type = cable_type_t::CABLE_TYPE_CU;

    if (phy_port != -1) {
        if (sdk::platform::xcvr_valid(phy_port-1) == true) {
            port_args->cable_type = sdk::platform::cable_type(phy_port-1);

            // for older boards, cable type returned is NONE.
            // Set it to CU
            if (port_args->cable_type == cable_type_t::CABLE_TYPE_NONE) {
                port_args->cable_type = cable_type_t::CABLE_TYPE_CU;
            }
            port_args->port_an_args =
                            sdk::platform::xcvr_get_an_args(phy_port - 1);
           SDK_TRACE_DEBUG("port : %u, phy_port : %u, user_cap : %u "
                           "fec ability : %u, fec request : %u, "
                           "cable_type : %u", port_args->port_num,
                           phy_port, port_args->port_an_args->user_cap,
                           port_args->port_an_args->fec_ability,
                           port_args->port_an_args->fec_request,
                           port_args->cable_type);

            port_args->toggle_neg_mode = false;
            port_args->toggle_fec_mode = false;

            // if AN=true and
            // (1) fiber cable is inserted
            //     auto_neg_enable = false
            //     port_speed based on transceiver
            //     derived_fec_type and num_lanes based on port_speed
            // (2) SFP is inserted
            //     port_speed based on transceiver
            //     derived_fec_type and num_lanes based on port_speed
            //     (2.1) CU SFP is inserted
            //           Enable toggle mode - b/w AN and Fixed
            // TODO: to extend to QSFP also after initial validations on SFPs
            if (port_args->auto_neg_enable == true) {
                if (port_args->cable_type == cable_type_t::CABLE_TYPE_FIBER) {
                    port_args->auto_neg_enable = false;
                    port_args_set_by_xcvr_state_(
                                          phy_port, &port_args->port_speed,
                                          &port_args->derived_fec_type,
                                          &port_args->num_lanes,
                                          &port_args->toggle_fec_mode);
                }
                if (sdk::platform::xcvr_type(phy_port-1) ==
                                                  xcvr_type_t::XCVR_TYPE_SFP) {
                    port_args_set_by_xcvr_state_(
                                          phy_port, &port_args->port_speed,
                                          &port_args->derived_fec_type,
                                          &port_args->num_lanes,
                                          &port_args->toggle_fec_mode);
                    if (port_args->cable_type == cable_type_t::CABLE_TYPE_CU) {
                        // for these types, toggle between force/an till link-up
                        // TOR/peer could have do either forced/an
                        port_args->toggle_neg_mode = true;
                        SDK_TRACE_DEBUG("port %u CU-SFP toggle_neg_mode "
                                        "enabled", port_args->port_num);
                    }
                }
            }
            SDK_TRACE_DEBUG("port %u auto_neg_enable %u toggle_an %u "
                            "derived_fec_type %u, toggle_fec %u",
                            port_args->port_num, port_args->auto_neg_enable,
                            port_args->toggle_neg_mode,
                            port_args->derived_fec_type,
                            port_args->toggle_fec_mode);

        } else {
            port_args->admin_state = port_admin_state_t::PORT_ADMIN_STATE_DOWN;
        }
    }
    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// Validate port create config
//-----------------------------------------------------------------------------
static bool
validate_port_create (port_args_t *args)
{
    if (args->port_type == port_type_t::PORT_TYPE_NONE) {
        SDK_TRACE_ERR("Invalid port type for port: %d", args->port_num);
        return false;
    }

    if (args->port_speed == port_speed_t::PORT_SPEED_NONE) {
        SDK_TRACE_ERR("Invalid port speed for port: %d", args->port_num);
        return false;
    }

    // validate speed against num_lanes only if AN=false
    if (args->auto_neg_enable == false) {
        return validate_speed_lanes(args->port_speed, args->num_lanes);
    }

    return true;
}

static void
port_init_num_lanes (port_args_t *args)
{
    uint32_t asic_inst = 0;

    // capri is NRZ serdes. Force num_lanes based on speed
    // to avoid upper layers setting it
    if (g_linkmgr_cfg.catalog->asic_type(asic_inst) ==
                               asic_type_t::SDK_ASIC_TYPE_CAPRI) {
        switch (args->port_speed) {
        case port_speed_t::PORT_SPEED_100G:
        case port_speed_t::PORT_SPEED_40G:
            args->num_lanes = 4;
            break;

        case port_speed_t::PORT_SPEED_50G:
            args->num_lanes = 2;
            break;

        case port_speed_t::PORT_SPEED_25G:
        case port_speed_t::PORT_SPEED_10G:
        case port_speed_t::PORT_SPEED_1G:
            args->num_lanes = 1;
            break;

        default:
            break;
        }
    }
}

static void
port_init_defaults (port_args_t *args)
{
    if (args->mtu == 0) {
        args->mtu = 9216;   // TODO define?
    }
}

//-----------------------------------------------------------------------------
// PD If Create
//-----------------------------------------------------------------------------
void *
port_create (port_args_t *args)
{
    sdk_ret_t    ret = SDK_RET_OK;
    void         *mem = NULL;
    port         *port_p = NULL;

    port_init_num_lanes(args);
    if (validate_port_create (args) == false) {
        // TODO return codes
        return NULL;
    }

    port_init_defaults(args);
    mem = g_linkmgr_state->port_slab()->alloc();
    port_p = new (mem) port();

    // init the bringup and debounce timers
    port_p->timers_init();

    port_p->set_port_num(args->port_num);
    port_p->set_port_type(args->port_type);
    port_p->set_port_speed(args->port_speed);
    port_p->set_mac_id(args->mac_id);
    port_p->set_mac_ch(args->mac_ch);
    port_p->set_num_lanes(args->num_lanes);
    port_p->set_debounce_time(args->debounce_time);
    port_p->set_auto_neg_enable(args->auto_neg_enable);
    port_p->set_toggle_neg_mode(args->toggle_neg_mode);
    port_p->set_toggle_fec_mode(args->toggle_fec_mode);
    port_p->set_mtu(args->mtu);
    port_p->set_pause(args->pause);
    port_p->set_tx_pause_enable(args->tx_pause_enable);
    port_p->set_rx_pause_enable(args->rx_pause_enable);
    port_p->set_cable_type(args->cable_type);
    port_p->set_loopback_mode(args->loopback_mode);

    // store the user configured admin state
    port_p->set_user_admin_state(args->user_admin_state);

    // store the user configured AN
    port_p->set_auto_neg_cfg(args->auto_neg_cfg);

    // store the user configured num_lanes
    port_p->set_num_lanes_cfg(args->num_lanes_cfg);

    // store the user configure fec type
    port_p->set_user_fec_type(args->user_fec_type);

    port_p->set_derived_fec_type(args->derived_fec_type);
    port_p->set_mac_fns(&mac_fns);
    port_p->set_serdes_fns(&serdes_fns);

    if(args->port_type == sdk::types::port_type_t::PORT_TYPE_MGMT) {
        port_p->set_mac_fns(&mac_mgmt_fns);
    }

    // use the configured num_lanes for setting sbus_addr
    for (uint32_t i = 0; i < args->num_lanes_cfg; ++i) {
        port_p->sbus_addr_set(i, args->sbus_addr[i]);
    }

    // set the source mac addr for pause frames
    port_p->port_mac_set_pause_src_addr(args->mac_addr);

    if (args->port_an_args != NULL) {
        port_p->set_user_cap(args->port_an_args->user_cap);
        port_p->set_fec_ability(args->port_an_args->fec_ability);
        port_p->set_fec_request(args->port_an_args->fec_request);
    }

    ret = port::port_mac_stats_init(port_p);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("port %u mac stats init failed", args->port_num);
    }

    // disable a port to invoke soft reset
    ret = port::port_disable(port_p);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("port %u disable failed", args->port_num);
    }

    // if admin up is set, enable the port
    if (args->admin_state == port_admin_state_t::PORT_ADMIN_STATE_UP) {
        ret = port::port_enable(port_p);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("port %u enable failed", args->port_num);
        }
    }

    return port_p;
}

//-----------------------------------------------------------------------------
// Validate port update config
//-----------------------------------------------------------------------------
static bool
validate_port_update (port *port_p, port_args_t *args)
{
    if (args->port_type != port_p->port_type()) {
        SDK_TRACE_ERR("port_type update not supported for port: %d",
                        args->port_num);
        return false;
    }

    // TODO skip num_lanes check to support auto speed set for QSA
#if 0
    if (args->num_lanes != port_p->num_lanes()) {
        SDK_TRACE_ERR("num_lanes update not supported for port: %d",
                        args->port_num);
        return false;
    }
#endif

    // validate speed against num_lanes only if AN=false
    if (args->auto_neg_enable == false) {
        if (args->num_lanes == port_p->num_lanes()) {
            return validate_speed_lanes(args->port_speed, args->num_lanes);
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
// update given port
//-----------------------------------------------------------------------------
sdk_ret_t
port_update (void *pd_p, port_args_t *args)
{
    sdk_ret_t          ret           = SDK_RET_OK;
    bool               configured    = false;
    port               *port_p       = (port *)pd_p;
    port_admin_state_t prev_admin_st = port_p->admin_state();

    port_init_num_lanes(args);
    if (validate_port_update (port_p, args) == false) {
        // TODO return codes
        return SDK_RET_ERR;
    }

    // check if any properties have changed

    if (args->port_speed != port_speed_t::PORT_SPEED_NONE &&
        args->port_speed != port_p->port_speed()) {
        SDK_TRACE_DEBUG("speed updated. new: %d, old: %d",
                        args->port_speed, port_p->port_speed());

        port_p->port_deinit();
        port_p->set_port_speed(args->port_speed);
        configured = true;
    }

    if (args->admin_state != port_admin_state_t::PORT_ADMIN_STATE_NONE &&
        args->admin_state != port_p->admin_state()) {
        SDK_TRACE_DEBUG("admin_state updated. new: %d, old: %d",
                        args->admin_state, port_p->admin_state());

        // Do not update admin_state here
        // port_disable following configured=true will update it
        configured = true;
    }

    if (args->user_admin_state != port_admin_state_t::PORT_ADMIN_STATE_NONE &&
        args->user_admin_state != port_p->user_admin_state()) {
        SDK_TRACE_DEBUG("user_admin_state updated. new: %d, old: %d",
                        args->user_admin_state, port_p->user_admin_state());
        port_p->set_user_admin_state(args->user_admin_state);
    }

    if (args->cable_type != cable_type_t::CABLE_TYPE_NONE &&
        args->cable_type != port_p->cable_type()) {
        SDK_TRACE_DEBUG("cable_type updated. new: %d, old: %d",
                        args->cable_type, port_p->cable_type());
        port_p->set_cable_type(args->cable_type);
        configured = true;
    }

    // FEC_TYPE_NONE is valid
    if (args->user_fec_type != port_p->user_fec_type()) {
        SDK_TRACE_DEBUG("fec updated. new: %d, old: %d",
                        args->user_fec_type, port_p->user_fec_type());
        port_p->set_user_fec_type(args->user_fec_type);
        configured = true;
    }
    port_p->set_derived_fec_type(args->derived_fec_type);
    port_p->set_toggle_fec_mode(args->toggle_fec_mode);

    // MTU 0 is invalid
    if (args->mtu != 0 &&
        args->mtu != port_p->mtu()) {
        SDK_TRACE_DEBUG("mtu updated. new: %d, old: %d",
                        args->mtu, port_p->mtu());
        port_p->set_mtu(args->mtu);
        configured = true;
    }

    if (args->debounce_time != port_p->debounce_time()) {
        SDK_TRACE_DEBUG("Debounce updated. new: %d, old: %d",
                        args->debounce_time, port_p->debounce_time());
        port_p->set_debounce_time(args->debounce_time);
        configured = true;
    }

    // toggle_neg_mode overrides auto_neg_enable
    // check auto_neg_enable only if toggle_neg_mode is false
    if (args->toggle_neg_mode == false) {
        if (args->auto_neg_enable != port_p->auto_neg_enable()) {
            SDK_TRACE_DEBUG("AN updated. new: %d, old: %d",
                            args->auto_neg_enable, port_p->auto_neg_enable());
            port_p->set_auto_neg_enable(args->auto_neg_enable);
            configured = true;
        }
    }
    port_p->set_toggle_neg_mode(args->toggle_neg_mode);

    // TODO required?
    // if (args->toggle_neg_mode != port_p->toggle_neg_mode()) {
    //     SDK_TRACE_DEBUG("port %u toggle neg mode updated. new: %d, old: %d",
    //                     args->port_num, args->toggle_neg_mode, port_p->toggle_neg_mode());
    //     port_p->set_toggle_neg_mode(args->toggle_neg_mode);
    //     configured = true;
    // }

    if (args->auto_neg_cfg != port_p->auto_neg_cfg()) {
        SDK_TRACE_DEBUG("AN cfg updated. new: %d, old: %d",
                        args->auto_neg_cfg, port_p->auto_neg_cfg());
        port_p->set_auto_neg_cfg(args->auto_neg_cfg);
        configured = true;
    }

    // TODO update num_lanes to support auto speed set for QSA
    if (args->num_lanes != port_p->num_lanes()) {
        SDK_TRACE_DEBUG("num_lanes updated. new: %d, old: %d",
                        args->num_lanes, port_p->num_lanes());
        port_p->set_num_lanes(args->num_lanes);
        configured = true;
    }

    if (args->num_lanes_cfg != port_p->num_lanes_cfg()) {
        SDK_TRACE_DEBUG("num_lanes_cfg updated. new: %d, old: %d",
                        args->num_lanes_cfg, port_p->num_lanes_cfg());
        port_p->set_num_lanes_cfg(args->num_lanes_cfg);
        configured = true;
    }

    if (args->loopback_mode != port_p->loopback_mode()) {
        SDK_TRACE_DEBUG(
            "loopback updated. new: %s, old: %s",
            args->loopback_mode ==
                port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC? "mac" :
            args->loopback_mode ==
                port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY? "phy" : "none",
            port_p->loopback_mode() ==
                port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC? "mac" :
            port_p->loopback_mode() ==
                port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY? "phy" : "none");

        port_p->set_loopback_mode(args->loopback_mode);
        configured = true;
    }

    if (args->pause != port_p->pause()) {
        SDK_TRACE_DEBUG("Pause updated. new: %d, old: %d",
                        args->pause, port_p->pause());
        port_p->set_pause(args->pause);
        configured = true;
    }

    if (args->tx_pause_enable != port_p->tx_pause_enable()) {
        SDK_TRACE_DEBUG("Tx Pause updated. new: %d, old: %d",
                        args->tx_pause_enable, port_p->tx_pause_enable());
        port_p->set_tx_pause_enable(args->tx_pause_enable);
        configured = true;
    }

    if (args->rx_pause_enable != port_p->rx_pause_enable()) {
        SDK_TRACE_DEBUG("Rx Pause updated. new: %d, old: %d",
                        args->rx_pause_enable, port_p->rx_pause_enable());
        port_p->set_rx_pause_enable(args->rx_pause_enable);
        configured = true;
    }

    // TODO need check?
    if (args->mac_stats_reset == true) {
        SDK_TRACE_DEBUG("Resetting MAC stats");
        port_p->port_mac_stats_reset(args->mac_stats_reset);
        port_p->port_mac_stats_reset(false);
    }

    if (args->port_an_args != NULL) {
        port_p->set_user_cap(args->port_an_args->user_cap);
        port_p->set_fec_ability(args->port_an_args->fec_ability);
        port_p->set_fec_request(args->port_an_args->fec_request);
    }

    // Disable the port if any config has changed
    if (configured == true) {
        ret = port::port_disable(port_p);
    }

    // Enable the port if -
    //      admin-up state is set in request msg OR
    //      admin state is not set in request msg, but port was admin up
    switch(args->admin_state) {
    case port_admin_state_t::PORT_ADMIN_STATE_NONE:
        if (prev_admin_st == port_admin_state_t::PORT_ADMIN_STATE_UP) {
            ret = port::port_enable(port_p);
        }
        break;

    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        ret = port::port_disable(port_p);
        break;

    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        ret = port::port_enable(port_p);
        break;

    default:
        break;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// delete given port by disabling the port and then deleting the port instance
//-----------------------------------------------------------------------------
sdk_ret_t
port_delete (void *pd_p)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = (port *)pd_p;

    SDK_TRACE_DEBUG("port delete");

    port_p->port_deinit();

    ret = port::port_disable(port_p);
    port_p->~port();
    g_linkmgr_state->port_slab()->free(port_p);

    return ret;
}

//-----------------------------------------------------------------------------
// PD Port get
//-----------------------------------------------------------------------------
sdk_ret_t
port_get (void *pd_p, port_args_t *args)
{
    port      *port_p = (port *)pd_p;

    args->port_num    = port_p->port_num();
    args->port_type   = port_p->port_type();
    args->port_speed  = port_p->port_speed();
    args->admin_state = port_p->admin_state();
    args->mac_id      = port_p->mac_id();
    args->mac_ch      = port_p->mac_ch();
    args->num_lanes   = port_p->num_lanes();
    args->fec_type    = port_p->fec_type();
    args->user_fec_type = port_p->user_fec_type();
    args->derived_fec_type = port_p->derived_fec_type();
    args->mtu         = port_p->mtu();
    args->debounce_time    = port_p->debounce_time();
    args->auto_neg_enable  = port_p->auto_neg_enable();
    args->auto_neg_cfg     = port_p->auto_neg_cfg();
    args->user_admin_state = port_p->user_admin_state();
    args->num_lanes_cfg    = port_p->num_lanes_cfg();
    args->pause       = port_p->pause();
    args->tx_pause_enable = port_p->tx_pause_enable();
    args->rx_pause_enable = port_p->rx_pause_enable();
    args->link_sm     = port_p->port_link_sm();
    args->loopback_mode = port_p->loopback_mode();
    args->num_link_down = port_p->num_link_down();
    args->bringup_duration.tv_sec = port_p->bringup_duration_sec();
    args->bringup_duration.tv_nsec = port_p->bringup_duration_nsec();
    args->sm_logger = port_p->sm_logger();
    strncpy(args->last_down_timestamp, port_p->last_down_timestamp(), TIME_STR_SIZE);

    if (args->link_sm == port_link_sm_t::PORT_LINK_SM_AN_CFG &&
        port_p->auto_neg_enable() == true) {
        args->link_sm = port_p->port_link_an_sm();
    } else if (args->link_sm == port_link_sm_t::PORT_LINK_SM_DFE_TUNING) {
        args->link_sm = port_p->port_link_dfe_sm();
    }

    // TODO send live link status until poll timer is reduced
    // args->oper_status = port_p->oper_status();
    if (port_p->port_link_status() == true) {
        args->oper_status = port_oper_status_t::PORT_OPER_STATUS_UP;
    } else {
        args->oper_status = port_oper_status_t::PORT_OPER_STATUS_DOWN;
    }

    if (NULL != args->stats_data) {
        port_p->port_mac_stats_get (args->stats_data);
    }

    return SDK_RET_OK;
}

#if 0
void *
port_make_clone (void *pd_orig_p)
{
    port  *port_p;
    port  *pd_new_clone_p;

    SDK_TRACE_DEBUG("port clone");
    port_p = (port *)pd_orig_p;
    pd_new_clone_p =
        (port *)g_linkmgr_state->port_slab()->alloc();
    // populate cloned pd instance from existing pd instance
    pd_new_clone_p->set_oper_status(port_p->oper_status());
    pd_new_clone_p->set_port_speed(port_p->port_speed());
    pd_new_clone_p->set_port_type(port_p->port_type());
    pd_new_clone_p->set_admin_state(port_p->admin_state());

    pd_new_clone_p->set_port_link_sm(port_p->port_link_sm());
    pd_new_clone_p->set_link_bring_up_timer(port_p->link_bring_up_timer());

    pd_new_clone_p->set_mac_id(port_p->mac_id());
    pd_new_clone_p->set_mac_ch(port_p->mac_ch());
    pd_new_clone_p->set_num_lanes(port_p->num_lanes());

    return pd_new_clone_p;
}
#endif

//-----------------------------------------------------------------------------
// PD Port mem free
//-----------------------------------------------------------------------------
sdk_ret_t
port_mem_free (port_args_t *args)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = (port *)args->port_p;

    g_linkmgr_state->port_slab()->free(port_p);

    return ret;
}

bool
port_has_speed_changed (port_args_t *args)
{
    port *port_p = (port *)args->port_p;
    return (args->port_speed != port_p->port_speed());
}

bool
port_has_admin_state_changed (port_args_t *args)
{
    port *port_p = (port *)args->port_p;
    return (args->admin_state != port_p->admin_state());
}

//-----------------------------------------
// AACS server
//-----------------------------------------
static void*
linkmgr_aacs_start (void* ctxt)
{
    if (aacs_server_port_num != -1) {
        sdk::linkmgr::serdes_fns.serdes_aacs_start(aacs_server_port_num);
    }

    return NULL;
}

sdk_ret_t
start_aacs_server (int port)
{
    int thread_prio  = 0;
    int thread_id    = 0;
    int sched_policy = SCHED_OTHER;

    if (aacs_server_port_num != -1) {
        SDK_TRACE_DEBUG("AACS server already started on port: %d",
                        aacs_server_port_num);
        return SDK_RET_OK;
    }

    thread_prio = sched_get_priority_max(sched_policy);
    if (thread_prio < 0) {
        return SDK_RET_ERR;
    }

    thread_id = LINKMGR_THREAD_ID_AACS_SERVER;
    sdk::lib::thread *thread =
        sdk::lib::thread::factory(
                        std::string("aacs-server").c_str(),
                        thread_id,
                        sdk::lib::THREAD_ROLE_CONTROL,
                        0x0 /* use all control cores */,
                        linkmgr_aacs_start,
                        thread_prio,
                        sched_policy,
                        true);
    if (thread == NULL) {
        SDK_TRACE_ERR("Failed to create linkmgr aacs server thread");
        return SDK_RET_ERR;
    }

    g_linkmgr_threads[thread_id] = thread;
    aacs_server_port_num = port;
    thread->start(NULL);

    return SDK_RET_OK;
}

void
stop_aacs_server (void)
{
    int thread_id = LINKMGR_THREAD_ID_AACS_SERVER;

    if (g_linkmgr_threads[thread_id] != NULL) {
        // stop the thread
        g_linkmgr_threads[thread_id]->stop();
        // free the allocated thread
        sdk::lib::thread::destroy(g_linkmgr_threads[thread_id]);
        g_linkmgr_threads[thread_id] = NULL;
    }

    // reset the server port
    aacs_server_port_num = -1;
}

port_admin_state_t
port_default_admin_state (void)
{
    return g_linkmgr_cfg.admin_state;
}

mem_addr_t
port_stats_addr (uint32_t ifindex)
{
    sdk::types::mem_addr_t port_stats_base;

    if (g_linkmgr_cfg.mempartition == NULL) {
        SDK_TRACE_ERR("port %s stats_init NULL mempartition port stats not supported",
                      eth_ifindex_to_str(ifindex).c_str());
        return INVALID_MEM_ADDRESS;
    }
    port_stats_base = g_linkmgr_cfg.mempartition->start_addr(ASIC_HBM_REG_PORT_STATS);

    if ((port_stats_base == 0) || (port_stats_base == INVALID_MEM_ADDRESS)) {
        SDK_TRACE_ERR("port %s stats_init port_stats_base 0x%llx port stats not supported",
                      eth_ifindex_to_str(ifindex).c_str(), port_stats_base);
        return INVALID_MEM_ADDRESS;
    }

    return port_stats_base + port_stats_addr_offset(ifindex);
}

/// \brief     helper method to store the user configured values into separate
///            variables in port_args
/// \param[in] port_args port arguments to be programmed
void
port_store_user_config (port_args_t *port_args)
{
    // store user configured admin_state in another variable to be used
    // during xcvr insert/remove events
    port_args->user_admin_state = port_args->admin_state;

    // store user configured AN in another variable to be used
    // during xcvr insert/remove events
    port_args->auto_neg_cfg = port_args->auto_neg_enable;

    // store user configured num_lanes in another variable to be used
    // during xcvr insert/remove events
    port_args->num_lanes_cfg = port_args->num_lanes;

    // store user configured fec type
    port_args->user_fec_type = port_args->derived_fec_type =
                                                  port_args->fec_type;
}

}    // namespace linkmgr
}    // namespace sdk
