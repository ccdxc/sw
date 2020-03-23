// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// Port service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/include/hal.hpp"
#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "linkmgr_svc.hpp"
#include "linkmgr_src.hpp"
#include "linkmgr_utils.hpp"
#include "linkmgr_internal.hpp"
#include "delphi/linkmgr_delphi.hpp"

using hal::hal_cfg_db_open;
using hal::hal_cfg_db_close;
using hal::CFG_OP_WRITE;
using hal::CFG_OP_READ;

static ::types::ApiStatus
hal_ret_to_api_status (hal_ret_t hal_ret)
{
    switch (hal_ret) {
    case HAL_RET_OK:
        return types::API_STATUS_OK;

    case HAL_RET_PORT_NOT_FOUND:
        return types::API_STATUS_NOT_FOUND;

    case HAL_RET_ENTRY_EXISTS:
        return types::API_STATUS_EXISTS_ALREADY;

    case HAL_RET_OOM:
        return types::API_STATUS_OUT_OF_MEM;

    case HAL_RET_HANDLE_INVALID:
        return types::API_STATUS_HANDLE_INVALID;

    default:
        return types::API_STATUS_ERR;
    }

    return types::API_STATUS_OK;
}

static uint32_t
sbus_addr (uint32_t asic, uint32_t asic_port, uint32_t lane)
{
    return linkmgr::catalog()->sbus_addr(asic, asic_port, lane);
}

static uint32_t
breakout_modes (uint32_t port)
{
    return linkmgr::catalog()->breakout_modes(port);
}

static uint32_t
num_fp_ports (void)
{
    return linkmgr::catalog()->num_fp_ports();
}

static uint32_t
num_lanes_fp (uint32_t port)
{
    return linkmgr::catalog()->num_lanes_fp(port);
}

static port_type_t
port_type_fp (uint32_t port)
{
    return linkmgr::catalog()->port_type_fp(port);
}

static uint32_t
asic_port_to_mac_id (uint32_t asic, uint32_t asic_port) {
    return linkmgr::catalog()->asic_port_to_mac_id(asic, asic_port);
}

static uint32_t
asic_port_to_mac_ch (uint32_t asic, uint32_t asic_port) {
    return linkmgr::catalog()->asic_port_to_mac_ch(asic, asic_port);
}

static uint32_t
port_num_to_asic_num (uint32_t port_num)
{
    return 0;
}

static uint32_t
port_num_to_asic_port (uint32_t port_num)
{
    return port_num - 1;
}

void
populate_port_create_args (PortSpec& spec, port_args_t *args)
{
    uint32_t asic      = 0;
    uint32_t asic_port = 0;

    sdk::linkmgr::port_args_init(args);

    args->port_num        = spec.key_or_handle().port_id();
    asic                  = port_num_to_asic_num(args->port_num);
    asic_port             = port_num_to_asic_port(args->port_num);

    args->mac_id          = asic_port_to_mac_id(asic, asic_port);
    args->mac_ch          = asic_port_to_mac_ch(asic, asic_port);
    args->num_lanes       = spec.num_lanes();
    args->auto_neg_enable = spec.auto_neg_enable();
    args->debounce_time   = spec.debounce_time();
    args->mtu             = spec.mtu();
    args->tx_pause_enable = spec.tx_pause_enable();
    args->rx_pause_enable = spec.rx_pause_enable();

    args->pause           =
        linkmgr::port_pause_type_spec_to_sdk_port_pause_type(spec.pause());
    args->port_type       =
        linkmgr::port_type_spec_to_sdk_port_type (spec.port_type());
    args->admin_state     =
        linkmgr::port_admin_st_spec_to_sdk_port_admin_st(spec.admin_state());
    args->port_speed      =
        linkmgr::port_speed_spec_to_sdk_port_speed(spec.port_speed());
    args->fec_type        =
        linkmgr::port_fec_type_spec_to_sdk_port_fec_type(spec.fec_type());
    args->loopback_mode   =
        linkmgr::port_lb_mode_spec_to_sdk_port_lb_mode(spec.loopback_mode());

    for (uint32_t i = 0; i < spec.num_lanes(); ++i) {
        args->sbus_addr[i] = sbus_addr(asic, asic_port, i);
    }
}

//------------------------------------------------------------------------------
// validate an incoming port create request
//------------------------------------------------------------------------------
bool
validate_port_create (PortSpec& spec, PortResponse *response)
{
    // TODO meta check

    std::string str;

    // must have key-handle set
    str = "key_or_handle";
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{} not set in request",
                       str.c_str());
        response->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return false;
    }

    // port type
    str = "port type";
    if (spec.port_type() == ::port::PORT_TYPE_NONE) {
        HAL_TRACE_ERR("not set in request {}",
                       str.c_str());
        response->set_api_status(types::API_STATUS_PORT_TYPE_INVALID);
        return false;
    }

    // port speed
    str = "port speed";
    if (spec.port_speed() == ::port::PORT_SPEED_NONE) {
        HAL_TRACE_ERR("not set in request {}",
                       str.c_str());
        response->set_api_status(types::API_STATUS_PORT_SPEED_INVALID);
        return false;
    }

    // number of lanes for the port
    str = "number of lanes";
    if (spec.num_lanes() == 0) {
        HAL_TRACE_ERR("not set in request {}",
                       str.c_str());
        response->set_api_status(types::API_STATUS_PORT_NUM_LANES_INVALID);
        return false;
    }

    return true;
}

Status
PortServiceImpl::PortCreate(ServerContext *context,
                            const PortRequestMsg *req,
                            PortResponseMsg *rsp)
{
    uint32_t               i          = 0;
    uint32_t               nreqs      = req->request_size();
    PortResponse           *response  = NULL;
    port_args_t            port_args  = {0};
    hal_ret_t              hal_ret    = HAL_RET_OK;
    hal_handle_t           hal_handle = 0;
    dobj::PortStatusPtr    port_status;

    HAL_TRACE_DEBUG("Rcvd Port Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal_cfg_db_open(CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        if (validate_port_create(spec, response) == false) {
            HAL_TRACE_ERR("port create validation failed");
            continue;
        }
        // set the params in port_args
        populate_port_create_args(spec, &port_args);
        // create the port
        hal_ret = linkmgr::port_create(&port_args, &hal_handle);

        // ports are created internally in linkmgr.
        // When Agent tries to creates ports again, HAL_RET_ENTRY_EXISTS
        // is returned. Ignore until Agent is cleaned up
        if (hal_ret == HAL_RET_ENTRY_EXISTS) {
            hal_ret = HAL_RET_OK;
        }

        response->set_api_status(hal_ret_to_api_status(hal_ret));

        if (hal_ret == HAL_RET_OK) {
            port_status = std::make_shared<dobj::PortStatus>(response->status());
            port_status->mutable_key_or_handle()->set_port_id(spec.key_or_handle().port_id());
            linkmgr::delphi_sdk_get()->QueueUpdate(port_status);
        }
    }
    hal_cfg_db_close();

    return Status::OK;
}

//------------------------------------------------------------------------------
// validate port update request
//------------------------------------------------------------------------------
bool
validate_port_update (PortSpec& spec, PortResponse*rsp)
{
    // TODO no meta check?

    std::string str;

    // must have key-handle set
    str = "key_or_handle";
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{} not set in update request",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return false;
    }

    return true;
}

void
populate_port_update_args (PortSpec& spec, port_args_t *args)
{
    uint32_t ifindex;

    sdk::linkmgr::port_args_init(args);

    // port update is with ifindex
    // convert to logical port
    ifindex = spec.key_or_handle().port_id();
    args->port_num = sdk::lib::catalog::ifindex_to_logical_port(ifindex);
    args->mtu             = spec.mtu();
    args->auto_neg_enable = spec.auto_neg_enable();
    args->debounce_time   = spec.debounce_time();
    args->mac_stats_reset = spec.mac_stats_reset();
    args->num_lanes       = spec.num_lanes();
    args->tx_pause_enable = spec.tx_pause_enable();
    args->rx_pause_enable = spec.rx_pause_enable();

    args->port_type   =
        linkmgr::port_type_spec_to_sdk_port_type (spec.port_type());
    args->pause       =
        linkmgr::port_pause_type_spec_to_sdk_port_pause_type(spec.pause());
    args->admin_state =
        linkmgr::port_admin_st_spec_to_sdk_port_admin_st(spec.admin_state());
    args->port_speed  =
        linkmgr::port_speed_spec_to_sdk_port_speed(spec.port_speed());
    args->fec_type    =
        linkmgr::port_fec_type_spec_to_sdk_port_fec_type(spec.fec_type());
    args->loopback_mode   =
        linkmgr::port_lb_mode_spec_to_sdk_port_lb_mode(spec.loopback_mode());
}

Status
PortServiceImpl::PortUpdate(ServerContext *context,
                            const PortRequestMsg *req,
                            PortResponseMsg *rsp)
{
    uint32_t      i         = 0;
    uint32_t      nreqs     = req->request_size();
    hal_ret_t     hal_ret   = HAL_RET_OK;
    port_args_t   port_args = {0};
    PortResponse  *response = NULL;

    HAL_TRACE_DEBUG("Rcvd Port Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal_cfg_db_open(CFG_OP_WRITE);

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);

        // validate the request message
        if(validate_port_update (spec, response) == false) {
            HAL_TRACE_ERR("port update validation failed");
            continue;
        }

        // set the params to be updated in port_args
        populate_port_update_args (spec, &port_args);

        // update the port params
        hal_ret = linkmgr::port_update(&port_args);

        response->set_api_status(hal_ret_to_api_status(hal_ret));
    }

    hal_cfg_db_close();

    return Status::OK;
}

//------------------------------------------------------------------------------
// validate port delete request
//------------------------------------------------------------------------------
bool
validate_port_delete_req (PortDeleteRequest& req, PortDeleteResponse *rsp)
{
    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        return false;
    }

    return true;
}

Status
PortServiceImpl::PortDelete(ServerContext *context,
                            const PortDeleteRequestMsg *req,
                            PortDeleteResponseMsg *rsp)
{
    uint32_t     i          = 0;
    uint32_t     nreqs      = req->request_size();
    hal_ret_t    hal_ret    = HAL_RET_OK;
    port_args_t  port_args  = { 0 };
    PortDeleteResponse *response = NULL;
    dobj::PortStatusPtr    port_status;

    HAL_TRACE_DEBUG("Rcvd Port Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal_cfg_db_open(CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);

        // validate the request message
        if (validate_port_delete_req(spec, response) == false) {
            HAL_TRACE_ERR("port delete request validation failed");
            continue;
        }
        sdk::linkmgr::port_args_init(&port_args);
        // set port number in port_args
        port_args.port_num = spec.key_or_handle().port_id();
        // delete the port
        hal_ret = linkmgr::port_delete(&port_args);
        response->set_api_status(hal_ret_to_api_status(hal_ret));
        if (hal_ret == HAL_RET_OK) {
            port_status = std::make_shared<dobj::PortStatus>();
            port_status->mutable_key_or_handle()->set_port_id(spec.key_or_handle().port_id());
            linkmgr::delphi_sdk_get()->DeleteObject(port_status);
        }
    }
    hal_cfg_db_close();

    return Status::OK;
}

//------------------------------------------------------------------------------
// process a port get request
//------------------------------------------------------------------------------

static void
populate_port_get_response_spec (port_args_t *port_args,
                                 void        *ctxt,
                                 hal_ret_t   hal_ret)
{
    PortSpec  *spec  = NULL;
    PortStats *stats = NULL;
    uint32_t ifindex;

    PortGetResponseMsg *rsp      = (PortGetResponseMsg *) ctxt;
    PortGetResponse    *response = rsp->add_response();

    if (hal_ret == HAL_RET_OK) {
        // populate spec
        spec = response->mutable_spec();
        ifindex = sdk::lib::catalog::logical_port_to_ifindex(port_args->port_num);
        spec->mutable_key_or_handle()->set_port_id(ifindex);
        spec->set_port_type(
                linkmgr::sdk_port_type_to_port_type_spec(port_args->port_type));
        spec->set_port_speed(
                linkmgr::sdk_port_speed_to_port_speed_spec(
                                          port_args->port_speed));
        // user configured admin_state
        spec->set_admin_state(
                linkmgr::sdk_port_admin_st_to_port_admin_st_spec
                                          (port_args->user_admin_state));
        spec->set_fec_type(
                linkmgr::sdk_port_fec_type_to_port_fec_type_spec
                                          (port_args->user_fec_type));
        spec->set_pause(
                linkmgr::sdk_port_pause_type_to_port_pause_type_spec
                                          (port_args->pause));
        spec->set_tx_pause_enable(port_args->tx_pause_enable);
        spec->set_rx_pause_enable(port_args->rx_pause_enable);
        spec->set_loopback_mode(
                linkmgr::sdk_port_lp_mode_to_port_lp_mode_spec(
                                         port_args->loopback_mode));
        spec->set_mac_id(port_args->mac_id);
        spec->set_mac_ch(port_args->mac_ch);
        spec->set_num_lanes(port_args->num_lanes_cfg);
        spec->set_mtu(port_args->mtu);

        // user configured AN
        spec->set_auto_neg_enable(port_args->auto_neg_cfg);
        spec->set_debounce_time(port_args->debounce_time);

        // populate oper status
        auto status = response->mutable_status();
        auto xcvr_status = status->mutable_xcvr_status();
        auto link_status = status->mutable_link_status();

        status->mutable_key_or_handle()->set_port_id(port_args->port_num);
#if 0
        auto ifindex = ETH_IFINDEX(linkmgr::catalog()->slot(),
                                   port_args->port_num, ETH_IF_DEFAULT_CHILD_PORT);
#endif
        status->set_ifindex(ifindex);

        // link status
        link_status->set_oper_state(
                (linkmgr::sdk_port_oper_st_to_port_oper_st_spec
                                        (port_args->oper_status)));
        link_status->set_port_speed(
                linkmgr::sdk_port_speed_to_port_speed_spec(
                                          port_args->port_speed));
        link_status->set_auto_neg_enable(port_args->auto_neg_enable);
        link_status->set_num_lanes(port_args->num_lanes);
        link_status->set_fec_type(
                     linkmgr::sdk_port_fec_type_to_port_fec_type_spec
                                                (port_args->fec_type));
        // transceiver status
        xcvr_status->set_port(port_args->xcvr_event_info.phy_port);
        xcvr_status->set_state(port::PortXcvrState(
                                    port_args->xcvr_event_info.state));
        xcvr_status->set_pid(port::PortXcvrPid(
                                    port_args->xcvr_event_info.pid));
        xcvr_status->set_cable_type(port::CableType(
                                    port_args->xcvr_event_info.cable_type));
        linkmgr::port_populate_xcvr_status_sprom(
                        xcvr_status, &port_args->xcvr_event_info.xcvr_sprom);

        // set the internal link state machine
        response->set_linksm_state(
            linkmgr::sdk_port_link_sm_to_port_link_sm_spec(port_args->link_sm));

        // MAC stats
        stats = response->mutable_stats();
        stats->set_num_link_down(port_args->num_link_down);

        // Port bringup time
        auto linkinfo = stats->mutable_link_timing_info();
        linkinfo->set_last_down_timestamp(port_args->last_down_timestamp);

        auto duration = linkinfo->mutable_bringup_duration();

        duration->set_sec(port_args->bringup_duration.tv_sec);
        duration->set_nsec(port_args->bringup_duration.tv_nsec);

        if (port_args->port_type == port_type_t::PORT_TYPE_MGMT) {
            for (int i = 0; i < MAX_MGMT_MAC_STATS; ++i) {
                auto mgmt_mac_stats = stats->add_mgmt_mac_stats();
                mgmt_mac_stats->set_type(MgmtMacStatsType(i));
                mgmt_mac_stats->set_count(port_args->stats_data[i]);
            }
        } else {
            for (int i = 0; i < MAX_MAC_STATS; ++i) {
                auto mac_stats = stats->add_mac_stats();
                mac_stats->set_type(MacStatsType(i));
                mac_stats->set_count(port_args->stats_data[i]);
            }
        }
    }
    response->set_api_status(hal_ret_to_api_status(hal_ret));
}

static void
populate_port_get_response (PortGetRequest& req, PortGetResponseMsg *rsp)
{
    hal_ret_t   hal_ret   = HAL_RET_OK;
    port_args_t port_args = { 0 };
    uint32_t ifindex;

    uint64_t    stats_data[MAX_MAC_STATS];

    memset(stats_data, 0, sizeof(uint64_t) * MAX_MAC_STATS);

    sdk::linkmgr::port_args_init(&port_args);

    // port get is with ifindex
    // convert to logical port
    ifindex = req.key_or_handle().port_id();
    port_args.port_num = sdk::lib::catalog::ifindex_to_logical_port(ifindex);
    port_args.stats_data = stats_data;

    hal_ret = linkmgr::port_get(&port_args);

    populate_port_get_response_spec(&port_args, rsp, hal_ret);
}

Status
PortServiceImpl::PortGet(ServerContext *context,
                         const PortGetRequestMsg *req,
                         PortGetResponseMsg *rsp)
{
    uint32_t i     = 0;
    uint32_t nreqs = req->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal_cfg_db_open(CFG_OP_READ);

    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);

        // if empty request, return all objects
        if (!request.has_key_or_handle()) {
            // pass function to be invoked for each object
            linkmgr::port_get_all(populate_port_get_response_spec, rsp);
            break;
        }

        populate_port_get_response(request, rsp);
    }

    hal_cfg_db_close();

    return Status::OK;
}

//------------------------------------------------------------------------------
// process a port info get request
//------------------------------------------------------------------------------

static hal_ret_t
populate_port_info(uint32_t fp_port, PortInfoGetResponse *response)
{
    response->mutable_spec()->mutable_key_or_handle()->set_port_id(fp_port);
    response->mutable_spec()->set_port_type(
            linkmgr::sdk_port_type_to_port_type_spec(port_type_fp(fp_port)));
    response->mutable_spec()->set_num_lanes(num_lanes_fp(fp_port));

    // bring up the mgmt port by default
    if (port_type_fp(fp_port) == port_type_t::PORT_TYPE_MGMT) {
        response->mutable_spec()->set_admin_state(::port::PORT_ADMIN_STATE_UP);
    } else {
        response->mutable_spec()->set_admin_state(
                linkmgr::sdk_port_admin_st_to_port_admin_st_spec(
                                sdk::linkmgr::port_default_admin_state()));
    }

    uint32_t breakout_mask = breakout_modes(fp_port);

    while (breakout_mask != 0) {
        // find the position (0 based) of the first set bit
        uint32_t             bit  = ffs(breakout_mask) - 1;

        port_breakout_mode_t mode = static_cast<port_breakout_mode_t>(bit);

        response->mutable_spec()->add_breakout_modes(
            linkmgr::sdk_port_breakout_mode_to_port_breakout_mode_spec(mode));

        // clear the bit
        breakout_mask = breakout_mask & ~(1 << bit);
    }

    return HAL_RET_OK;
}

static hal_ret_t
populate_port_info_response(uint32_t fp_port,
                            PortInfoGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    PortInfoGetResponse *response = rsp->add_response();

    ret = populate_port_info(fp_port, response);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting port info {}", fp_port);
        response->set_api_status(types::API_STATUS_NOT_FOUND);
    } else {
        response->set_api_status(types::API_STATUS_OK);
    }

    return ret;
}

static hal_ret_t
port_info_get (PortInfoGetRequest& req, PortInfoGetResponseMsg *rsp)
{
    uint32_t  fp_port = 0;
    hal_ret_t ret         = HAL_RET_OK;

    if (!req.has_key_or_handle()) {
        for (fp_port = 1; fp_port <= num_fp_ports();
                              ++fp_port) {
            populate_port_info_response(fp_port, rsp);
        }
        return ret;
    }

    // TODO handle case?
    fp_port = req.key_or_handle().port_id();

    populate_port_info_response(fp_port, rsp);

    return ret;
}

Status
PortServiceImpl::PortInfoGet(ServerContext *context,
                             const PortInfoGetRequestMsg *req,
                             PortInfoGetResponseMsg *rsp)
{
    uint32_t i     = 0;
    uint32_t nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd Port Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal_cfg_db_open(CFG_OP_READ);

    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        port_info_get(request, rsp);
    }

    hal_cfg_db_close();

    return Status::OK;
}

Status
PortServiceImpl::StartAacsServer (ServerContext *context,
                                  const AacsRequestMsg *req,
                                  types::Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Port AACS server Request");

    hal_cfg_db_open(CFG_OP_WRITE);
    sdk::linkmgr::start_aacs_server(req->aacs_server_port());
    hal_cfg_db_close();

    return Status::OK;
}

Status
PortServiceImpl::StopAacsServer (ServerContext *context,
                                 const types::Empty *req,
                                 types::Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Port AACS server Request");

    hal_cfg_db_open(CFG_OP_WRITE);
    sdk::linkmgr::stop_aacs_server();
    hal_cfg_db_close();

    return Status::OK;
}
