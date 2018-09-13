//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/hal_cfg.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include <google/protobuf/util/json_util.h>

namespace hal {

//------------------------------------------------------------------------------
// API open HAL cfg db in read/write mode
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_open (cfg_op_t op)
{
    HAL_TRACE_DEBUG("Opening cfg db with mode {}", op);
    return g_hal_state->cfg_db()->db_open(op);
}

//------------------------------------------------------------------------------
// API to close the HAL cfg db after performing commit/abort operation
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_close (void)
{
    HAL_TRACE_DEBUG("Closing cfg db, current mode {}", t_cfg_db_ctxt.cfg_op_);
    return g_hal_state->cfg_db()->db_close();
}

static hal_ret_t inline
hal_uplink_if_create (uint64_t if_id, uint32_t port_num)
{
    InterfaceSpec        spec;
    InterfaceResponse    response;
    hal_ret_t            ret;

    spec.mutable_key_or_handle()->set_interface_id(if_id);
    spec.set_type(::intf::IfType::IF_TYPE_UPLINK);
    spec.set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec.mutable_if_uplink_info()->set_port_num(port_num);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = interface_create(spec, &response);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Uplink interface {}/port {} create success, handle {}",
                        if_id, port_num, response.status().if_handle());
    } else {
        HAL_TRACE_ERR("Uplink interface {}/port {} create failed, err : {}",
                      if_id, port_num, ret);
    }
    hal::hal_cfg_db_close();

    return HAL_RET_OK;
}

hal_ret_t
hal_default_cfg_init (hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

}    // namespace hal
