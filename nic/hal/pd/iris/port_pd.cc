// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/port_pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/port.hpp"

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
    hal::pd::port  *pd_p =
                    (hal::pd::port*)g_hal_state_pd->port_slab()->alloc();

    pd_p->set_port_type(args->port_type);
    pd_p->set_port_speed(args->port_speed);

    pd_p->set_mac_id(args->mac_id);
    pd_p->set_mac_ch(args->mac_ch);
    pd_p->set_num_lanes(args->num_lanes);

    pd_p->set_pi_p(pi_p);
    pi_p->pd_p = pd_p;

    // if admin up is set, enable the port, else disable the port
    if(args->admin_state == ::port::PORT_ADMIN_STATE_UP) {
        ret = hal::pd::port::port_enable(pd_p);
    } else {
        ret = hal::pd::port::port_disable(pd_p);
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

    if (args->admin_state == ::port::PORT_ADMIN_STATE_UP) {
        ret = hal::pd::port::port_enable(pd_p);
    } else if (args->admin_state == ::port::PORT_ADMIN_STATE_DOWN) {
        ret = hal::pd::port::port_disable(pd_p);
    }

    HAL_TRACE_DEBUG("{}: port update", __FUNCTION__);

    return ret;
}

// ----------------------------------------------------------------------------
// PD Port Delete
// 1. Disable the port
// 2. Delete the port pd instance
// ----------------------------------------------------------------------------
hal_ret_t
pd_port_delete (pd_port_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    hal::port_t    *pi_p = args->pi_p;
    hal::pd::port  *pd_p = (hal::pd::port*)pi_p->pd_p;

    ret = hal::pd::port::port_disable(pd_p);

    g_hal_state_pd->port_slab()->free(pd_p);

    HAL_TRACE_DEBUG("{}: port delete", __FUNCTION__);

    return ret;
}

// ----------------------------------------------------------------------------
// PD Port get
// ----------------------------------------------------------------------------
hal_ret_t
pd_port_get (pd_port_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("{}: port get", __FUNCTION__);

    hal::pd::port  *pd_p = (hal::pd::port*) args->pi_p->pd_p;

    args->port_type   = pd_p->port_type();
    args->port_speed  = pd_p->port_speed();
    args->admin_state = pd_p->admin_state();
    args->mac_id      = pd_p->mac_id();
    args->mac_ch      = pd_p->mac_ch();
    args->num_lanes   = pd_p->num_lanes();
    args->oper_status = pd_p->oper_status();

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
    hal::pd::port  *pd_clone_p =
                    (hal::pd::port*)g_hal_state_pd->port_slab()->alloc();

    // populate cloned pd instance from existing pd instance
    pd_clone_p->set_oper_status(pd_p->oper_status());
    pd_clone_p->set_port_speed(pd_p->port_speed());
    pd_clone_p->set_port_type(pd_p->port_type());
    pd_clone_p->set_admin_state(pd_p->admin_state());

    pd_clone_p->set_port_link_sm(pd_p->port_link_sm());
    pd_clone_p->set_link_bring_up_timer(pd_p->link_bring_up_timer());

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

    g_hal_state_pd->port_slab()->free(pd_p);

    return ret;
}

bool
pd_port_has_speed_changed(pd_port_args_t *args)
{
    hal::pd::port *pd_p = (hal::pd::port*)args->pi_p->pd_p;

    return (args->port_speed != pd_p->port_speed());
}

bool
pd_port_has_admin_state_changed(pd_port_args_t *args)
{
    hal::pd::port *pd_p = (hal::pd::port*)args->pi_p->pd_p;

    return (args->admin_state != pd_p->admin_state());
}

}    // namespace pd
}    // namespace hal
