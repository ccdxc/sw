// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "port_utils.hpp"
#include "nic/linkmgr/linkmgr_src.hpp"

using linkmgr::g_linkmgr_state;

int port_create(uint32_t port_id)
{
    hal_ret_t     ret        = HAL_RET_OK;
    port_args_t   args       = {0};
    hal_handle_t  hal_handle = 0;

    sdk::linkmgr::port_args_init(&args);

    args.port_num = port_id;

    // speed
    args.port_speed = port_speed_t::PORT_SPEED_25G;

    // num_lanes
    args.num_lanes = 1;

    // Port type
    args.port_type = port_type_t::PORT_TYPE_ETH;

    // FEC type
    args.fec_type = port_fec_type_t::PORT_FEC_TYPE_FC;

    // mac_id and mac_ch
    args.mac_id = 0;
    args.mac_ch = 0;

    // admin status
    args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_create(&args, &hal_handle);

    linkmgr::g_linkmgr_state->cfg_db_close();

    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }

    return 0;
}

int port_update(uint32_t            port_id,
                hal_ret_t           ret_exp,
                port_speed_t        speed,
                port_admin_state_t  admin_state,
                port_fec_type_t     fec_type,
                uint32_t            debounce_time,
                bool                auto_neg_enable,
                uint32_t            num_lanes)
{
    hal_ret_t    ret  = HAL_RET_OK;
    port_args_t  args = {0};

    sdk::linkmgr::port_args_init(&args);

    args.port_num        = port_id;
    args.port_speed      = speed;
    args.admin_state     = admin_state;
    args.fec_type        = fec_type;
    args.debounce_time   = debounce_time;
    args.auto_neg_enable = auto_neg_enable;

    if (num_lanes != 0) {
        args.num_lanes = num_lanes;
    }

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_update(&args);

    linkmgr::g_linkmgr_state->cfg_db_close();

    if (ret != ret_exp) {
        std::cout << __func__ << ": failed. ret: " << ret << std::endl;
        return -1;
    }

    return 0;
}

int port_get(uint32_t             port_id,
             hal_ret_t            ret_exp,
             bool                 compare,
             port_oper_status_t   port_oper_status,
             port_type_t          port_type,
             port_speed_t         port_speed,
             port_admin_state_t   port_admin_state,
             port_fec_type_t      fec_type,
             uint32_t             debounce_time,
             bool                 auto_neg_enable)
{
    hal_ret_t    ret  = HAL_RET_OK;
    port_args_t  args = {0};

    sdk::linkmgr::port_args_init(&args);

    args.port_num = port_id;

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_READ);

    ret = linkmgr::port_get(&args);

    linkmgr::g_linkmgr_state->cfg_db_close();

    if (ret != ret_exp) {
        std::cout << __func__ << ": failed. ret: " << ret << std::endl;
        return -1;
    }

    std::cout << "Port Get succeeded for port "
              << port_id          << std::endl
              << " Port oper status: "
              << static_cast<uint32_t>(args.oper_status) << std::endl
              << " Port type: "
              << static_cast<uint32_t>(args.port_type)   << std::endl
              << " Admin state: "
              << static_cast<uint32_t>(args.admin_state) << std::endl
              << " Port speed: "
              << static_cast<uint32_t>(args.port_speed)  << std::endl
              << " MAC ID: "
              << args.mac_id      << std::endl
              << " MAC channel: "
              << args.mac_ch      << std::endl
              << " Num lanes: "
              << args.num_lanes   << std::endl;

    if (compare == true) {
        if (port_oper_status != port_oper_status_t::PORT_OPER_STATUS_NONE) {
            if (args.oper_status != port_oper_status) {
                std::cout << __func__
                          << ": oper_status get does not match."
                          << " Expected: "
                          << static_cast<uint32_t>(port_oper_status)
                          << ", Got: "
                          << static_cast<uint32_t>(args.oper_status)
                          << std::endl;
                return -1;
            }
        }

        if (port_type != port_type_t::PORT_TYPE_NONE) {
            if (args.port_type != port_type) {
                std::cout << __func__
                          << ": port_type get response does not match."
                          << " Expected: "
                          << static_cast<uint32_t>(port_type)
                          << ", Got: "
                          << static_cast<uint32_t>(args.port_type)
                          << std::endl;
                return -1;
            }
        }

        if (port_speed != port_speed_t::PORT_SPEED_NONE) {
            if (args.port_speed != port_speed) {
                std::cout << __func__
                          << ": port_speed get response does not match."
                          << " Expected: "
                          << static_cast<uint32_t>(port_speed)
                          << ", Got: "
                          << static_cast<uint32_t>(args.port_speed)
                          << std::endl;
                return -1;
            }
        }

        if (port_admin_state != port_admin_state_t::PORT_ADMIN_STATE_NONE) {
            if (args.admin_state != port_admin_state) {
                std::cout << __func__
                          << ": admin_state get response does not match."
                          << " Expected: "
                          << static_cast<uint32_t>(port_admin_state)
                          << ", Got: "
                          << static_cast<uint32_t>(args.admin_state)
                          << std::endl;
                return -1;
            }
        }

        if (args.fec_type != fec_type) {
            std::cout << __func__
                      << ": fec_type get response does not match."
                      << " Expected: "
                      << static_cast<uint32_t>(fec_type)
                      << ", Got: "
                      << static_cast<uint32_t>(args.fec_type)
                      << std::endl;
            return -1;
        }

        if (args.debounce_time != debounce_time) {
            std::cout << __func__
                      << ": debounce_time get response does not match."
                      << " Expected: "
                      << debounce_time
                      << ", Got: "
                      << args.debounce_time
                      << std::endl;
            return -1;
        }

        if (args.auto_neg_enable != auto_neg_enable) {
            std::cout << __func__
                      << ": AutoNeg get response does not match."
                      << " Expected: "
                      << auto_neg_enable
                      << ", Got: "
                      << args.auto_neg_enable
                      << std::endl;
            return -1;
        }
    }

    return 0;
}

int port_delete(uint32_t port_id)
{
    hal_ret_t    ret  = HAL_RET_OK;
    port_args_t  args = {0};

    sdk::linkmgr::port_args_init(&args);

    args.port_num = port_id;

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_delete(&args);

    linkmgr::g_linkmgr_state->cfg_db_close();

    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed. ret: " << ret << std::endl;
        return -1;
    }

    return 0;
}
