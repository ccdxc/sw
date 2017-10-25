#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/port_pd.hpp"
#include "nic/include/port.hpp"
#include "nic/include/hal_control.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// PD If Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_port_create (pd_port_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    hal::port_t    *pi_p = args->pi_p;
    hal::pd::port  *pd_p = new hal::pd::port();

    pd_p->set_port_type(args->port_type);
    pd_p->set_port_speed(args->port_speed);

    pd_p->set_mac_id(args->mac_id);
    pd_p->set_mac_ch(args->mac_ch);
    pd_p->set_num_lanes(args->num_lanes);

    pd_p->set_pi_p(pi_p);
    pi_p->pd_p = pd_p;

    if(args->admin_state == ::port::PORT_ADMIN_STATE_UP) {
        // wake up the hal control thread to process port event
        ret = hal_control_notify (HAL_CONTROL_OPERATION_PORT_ENABLE, pd_p);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error notifying control-thread for port enable");
        }
    }

    HAL_TRACE_DEBUG("{}: port create ", __FUNCTION__);

    return ret;
}

// ----------------------------------------------------------------------------
// PD Port Update
// ----------------------------------------------------------------------------
hal_ret_t
pd_port_update (pd_port_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    hal::port_t    *pi_p = args->pi_p;
    hal::pd::port  *pd_p = (hal::pd::port*)pi_p->pd_p;

    if (args->port_speed != ::port::PORT_SPEED_NONE) {
        pd_p->set_port_speed(args->port_speed);
    }

    // wake up the hal control thread to process port event
    if(args->admin_state == ::port::PORT_ADMIN_STATE_UP) {
        ret = hal_control_notify (HAL_CONTROL_OPERATION_PORT_ENABLE, pd_p);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error notifying control-thread for enable");
        }
    } else if (args->admin_state == ::port::PORT_ADMIN_STATE_DOWN) {
        ret = hal_control_notify (HAL_CONTROL_OPERATION_PORT_DISABLE, pd_p);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error notifying control-thread for disable");
        }
    }

    HAL_TRACE_DEBUG("{}: port update", __FUNCTION__);

    return ret;
}

// ----------------------------------------------------------------------------
// PD Port Delete
// ----------------------------------------------------------------------------
hal_ret_t
pd_port_delete (pd_port_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("{}: port delete", __FUNCTION__);

    return ret;
}

hal_ret_t
pd_port_make_clone (port_t *pi_p, port_t *pi_clone_p)
{
    hal_ret_t       ret = HAL_RET_OK;
    HAL_TRACE_DEBUG("{}: port clone", __FUNCTION__);

    // existing pd instance
    hal::pd::port  *pd_p = (hal::pd::port*) pi_p->pd_p;

    // cloned pd instance
    hal::pd::port  *pd_clone_p = new hal::pd::port();

    // populate cloned pd instance from existing pd instance
    pd_clone_p->set_port_type(pd_p->port_type());
    pd_clone_p->set_port_speed(pd_p->port_speed());

    pd_clone_p->set_mac_id(pd_p->mac_id());
    pd_clone_p->set_mac_ch(pd_p->mac_ch());
    pd_clone_p->set_num_lanes(pd_p->num_lanes());

    pd_clone_p->set_pi_p(pi_clone_p);
    pi_clone_p->pd_p = pd_clone_p;

    return ret;
}

// ----------------------------------------------------------------------------
// PD Port mem free
// ----------------------------------------------------------------------------
hal_ret_t
pd_port_mem_free (pd_port_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("{}: port mem_free", __FUNCTION__);

    hal::pd::port  *pd_p = (hal::pd::port*) args->pi_p->pd_p;

    delete pd_p;

    return ret;
}

bool
pd_port_has_speed_changed(pd_port_args_t *args)
{
    hal::pd::port *pd_p = (hal::pd::port*)args->pi_p->pd_p;

    return (args->port_speed != pd_p->port_speed());
}

}    // namespace pd
}    // namespace hal
