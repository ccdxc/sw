// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "port_utils.hpp"
#include "nic/linkmgr/linkmgr_src.hpp"

using sdk::SDK_RET_OK;
using linkmgr::g_linkmgr_state;

int port_create(uint32_t vrf_id,
                uint32_t port_id,
                types::ApiStatus api_status)
{
    hal_ret_t     ret;
    PortSpec      spec;
    PortResponse  rsp;

    spec.Clear();
    spec.mutable_key_or_handle()->set_port_id(port_id);
    spec.mutable_meta()->set_vrf_id(vrf_id);
    spec.set_port_speed(::port::PORT_SPEED_25G);
    spec.set_num_lanes(1);
    spec.set_port_type(::port::PORT_TYPE_ETH);
    spec.set_admin_state(::port::PORT_ADMIN_STATE_UP);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_create(spec, &rsp);

    g_linkmgr_state->cfg_db_close();

    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }

    if (rsp.api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }

    spec.Clear();
    return 0;
}

int port_update(uint32_t             vrf_id,
                uint32_t             port_id,
                port::PortSpeed      speed,
                port::PortAdminState admin_state,
                port::PortFecType    fec_type,
                uint32_t             debounce_time,
                bool                 auto_neg_enable,
                types::ApiStatus     api_status)
{
    hal_ret_t     ret;
    PortSpec      spec;
    PortResponse  rsp;

    spec.Clear();
    spec.mutable_key_or_handle()->set_port_id(port_id);
    spec.mutable_meta()->set_vrf_id(vrf_id);
    spec.set_port_speed(speed);
    spec.set_admin_state(admin_state);
    spec.set_fec_type(fec_type);
    spec.set_debounce_time(debounce_time);
    spec.set_auto_neg_enable(auto_neg_enable);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);
    ret = linkmgr::port_update(spec, &rsp);
    g_linkmgr_state->cfg_db_close();
    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }
    if (rsp.api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }
    spec.Clear();
    return 0;
}

int port_get(
        uint32_t             vrf_id,
        uint32_t             port_id,
        types::ApiStatus     api_status,
        hal_ret_t            ret_exp,
        bool                 compare,
        port::PortOperStatus port_oper_status,
        port::PortType       port_type,
        port::PortSpeed      port_speed,
        port::PortAdminState port_admin_state,
        port::PortFecType    fec_type,
        uint32_t             debounce_time,
        bool                 auto_neg_enable)
{
    hal_ret_t           ret = HAL_RET_OK;
    PortGetRequest      req;
    PortGetResponseMsg  rsp;
    PortGetResponse     response;

    req.Clear();
    req.mutable_key_or_handle()->set_port_id(port_id);
    req.mutable_meta()->set_vrf_id(vrf_id);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_READ);
    ret = linkmgr::port_get(req, &rsp);
    g_linkmgr_state->cfg_db_close();
    if (ret != ret_exp) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }

    response = rsp.response(0);

    if (response.api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }
    req.Clear();

    std::cout << "Port Get succeeded for port "
              << port_id << std::endl
              << " Port oper status: "
              << response.status().oper_status() << std::endl
              << " Port type: "
              << response.spec().port_type() << std::endl
              << " Admin state: "
              << response.spec().admin_state() << std::endl
              << " Port speed: "
              << response.spec().port_speed() << std::endl
              << " MAC ID: "
              << response.spec().mac_id() << std::endl
              << " MAC channel: "
              << response.spec().mac_ch() << std::endl
              << " Num lanes: "
              << response.spec().num_lanes() << std::endl;

    if (compare == true) {
        if (port_oper_status != port::PORT_OPER_STATUS_NONE) {
            if (response.status().oper_status() != port_oper_status) {
                std::cout << __func__
                          << ": oper_status get does not match."
                          << " Expected: "
                          << port_oper_status
                          << ", Got: "
                          << response.status().oper_status()
                          << std::endl;
                return -1;
            }
        }

        if (port_type != port::PORT_TYPE_NONE) {
            if (response.spec().port_type() != port_type) {
                std::cout << __func__
                          << ": port_type get response does not match."
                          << " Expected: "
                          << port_type
                          << ", Got: "
                          << response.spec().port_type()
                          << std::endl;
                return -1;
            }
        }

        if (port_speed != port::PORT_SPEED_NONE) {
            if (response.spec().port_speed() != port_speed) {
                std::cout << __func__
                          << ": port_speed get response does not match"
                          << " Expected: "
                          << port_speed
                          << ", Got: "
                          << response.spec().port_speed()
                          << std::endl;
                return -1;
            }
        }

        if (port_admin_state != port::PORT_ADMIN_STATE_NONE) {
            if (response.spec().admin_state() != port_admin_state) {
                std::cout << __func__
                          << ": admin_state get response does not match"
                          << " Expected: "
                          << port_admin_state
                          << ", Got: "
                          << response.spec().admin_state()
                          << std::endl;
                return -1;
            }
        }

        if (response.spec().fec_type() != fec_type) {
            std::cout << __func__
                      << ": fec_type get response does not match"
                      << " Expected: "
                      << fec_type
                      << ", Got: "
                      << response.spec().fec_type()
                      << std::endl;
            return -1;
        }

        if (response.spec().debounce_time() != debounce_time) {
            std::cout << __func__
                      << ": debounce_time get response does not match"
                      << " Expected: "
                      << debounce_time
                      << ", Got: "
                      << response.spec().debounce_time()
                      << std::endl;
            return -1;
        }

        if (response.spec().auto_neg_enable() != auto_neg_enable) {
            std::cout << __func__
                      << ": AutoNeg get response does not match"
                      << " Expected: "
                      << auto_neg_enable
                      << ", Got: "
                      << response.spec().auto_neg_enable()
                      << std::endl;
            return -1;
        }
    }
    return 0;
}

int port_delete(uint32_t vrf_id, uint32_t port_id, types::ApiStatus api_status) {
    hal_ret_t           ret;
    PortDeleteRequest   req;
    PortDeleteResponseMsg  rsp;

    req.Clear();
    req.mutable_key_or_handle()->set_port_id(port_id);
    req.mutable_meta()->set_vrf_id(vrf_id);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);
    ret = linkmgr::port_delete(req, &rsp);
    g_linkmgr_state->cfg_db_close();
    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }
    if (rsp.response(0).api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }
    req.Clear();
    return 0;
}
