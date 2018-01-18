// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "sdk/lock.hpp"
#include "sdk/pal.hpp"
#include "linkmgr_pd.hpp"
#include "linkmgr_state_pd.hpp"
//#include "linkmgr_src.hpp"
#include "port.hpp"
//#include "nic/model_sim/include/lib_model_client.h" ===> why do we need this ?
// TODO: should be in PAL/linkmgr code that resides in HAL

namespace sdk {
namespace linkmgr {
namespace pd {

linkmgr_state_pd *g_linkmgr_state_pd;

sdk_ret_t
linkmgr_init_pd (void)
{
    int  rc  = 0;

    g_linkmgr_state_pd = linkmgr_state_pd::factory();
    if (NULL == g_linkmgr_state_pd) {
        SDK_TRACE_ERR("{} pd init failed", __FUNCTION__);
        return SDK_RET_ERR;
    }

    // initialize the port mac and serdes functions
    port::port_init();

    if (platform_type() == sdk::lib::platform_type_t::PLATFORM_TYPE_SIM) {
        do {
            rc = lib_model_connect();
            if (rc == -1) {
                SDK_TRACE_ERR("Failed to connect to asic, retrying in 1 sec ...");
                sleep(1);
            }
        } while (rc == -1);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port_event_timer_pd (void *ctxt)
{
    port *pd_p = (port *)ctxt;
    return pd_p->port_link_sm_process();
}

sdk_ret_t
port_event_enable_pd (void *ctxt)
{
    port *pd_p = (port *)ctxt;
    return pd_p->port_enable();
}

sdk_ret_t
port_event_disable_pd (void *ctxt)
{
    port *pd_p = (port *)ctxt;
    return pd_p->port_disable();
}

//-----------------------------------------------------------------------------
// PD If Create
//-----------------------------------------------------------------------------
void *
port_create_pd (port_args_pd_t *args)
{
    sdk_ret_t            ret = SDK_RET_OK;
    linkmgr::pd::port    *pd_p;

    pd_p = (linkmgr::pd::port *)g_linkmgr_state_pd->port_slab()->alloc();
    pd_p->set_port_type(args->port_type);
    pd_p->set_port_speed(args->port_speed);
    pd_p->set_mac_id(args->mac_id);
    pd_p->set_mac_ch(args->mac_ch);
    pd_p->set_num_lanes(args->num_lanes);

    // if admin up is set, enable the port, else disable the port
    if (args->admin_state == ::port::PORT_ADMIN_STATE_UP) {
        ret = linkmgr::pd::port::port_enable(pd_p);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("{}: port enable failed", __FUNCTION__);
        }
    } else {
        ret = linkmgr::pd::port::port_disable(pd_p);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("{}: port disable failed", __FUNCTION__);
        }
    }

    return pd_p;
}

//-----------------------------------------------------------------------------
// PD Port Update
//-----------------------------------------------------------------------------
sdk_ret_t
port_update_pd (port_args_pd_t *args)
{
    sdk_ret_t      ret = SDK_RET_OK;
    linkmgr::pd::port  *pd_p = (linkmgr::pd::port*)args->pd_p;

    SDK_TRACE_DEBUG("{}: port update", __FUNCTION__);
    if (args->port_speed != ::port::PORT_SPEED_NONE) {
        pd_p->set_port_speed(args->port_speed);
    }

    if (args->admin_state == ::port::PORT_ADMIN_STATE_UP) {
        ret = linkmgr::pd::port::port_enable(pd_p);
    } else if (args->admin_state == ::port::PORT_ADMIN_STATE_DOWN) {
        ret = linkmgr::pd::port::port_disable(pd_p);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Port Delete
// 1. Disable the port
// 2. Delete the port pd instance
//-----------------------------------------------------------------------------
sdk_ret_t
port_delete_pd (port_args_pd_t *args)
{
    sdk_ret_t       ret = SDK_RET_OK;
    linkmgr::pd::port  *pd_p = (linkmgr::pd::port*)args->pd_p;

    SDK_TRACE_DEBUG("{}: port delete", __FUNCTION__);
    ret = linkmgr::pd::port::port_disable(pd_p);
    g_linkmgr_state_pd->port_slab()->free(pd_p);

    return ret;
}

//-----------------------------------------------------------------------------
// PD Port get
//-----------------------------------------------------------------------------
sdk_ret_t
port_get_pd(port_args_pd_t *args)
{
    sdk_ret_t            ret = SDK_RET_OK;
    linkmgr::pd::port    *pd_p = (linkmgr::pd::port *)args->pd_p;

    SDK_TRACE_DEBUG("{}: port get", __FUNCTION__);
    args->port_type   = pd_p->port_type();
    args->port_speed  = pd_p->port_speed();
    args->admin_state = pd_p->admin_state();
    args->mac_id      = pd_p->mac_id();
    args->mac_ch      = pd_p->mac_ch();
    args->num_lanes   = pd_p->num_lanes();
    args->oper_status = pd_p->oper_status();

    return ret;
}

void *
port_make_clone_pd (void *pd_orig_p)
{
    linkmgr::pd::port  *pd_p;
    linkmgr::pd::port  *pd_new_clone_p;

    SDK_TRACE_DEBUG("{}: port clone", __FUNCTION__);
    pd_p = (linkmgr::pd::port *)pd_orig_p;
    pd_new_clone_p =
        (linkmgr::pd::port *)g_linkmgr_state_pd->port_slab()->alloc();
    // populate cloned pd instance from existing pd instance
    pd_new_clone_p->set_oper_status(pd_p->oper_status());
    pd_new_clone_p->set_port_speed(pd_p->port_speed());
    pd_new_clone_p->set_port_type(pd_p->port_type());
    pd_new_clone_p->set_admin_state(pd_p->admin_state());

    pd_new_clone_p->set_port_link_sm(pd_p->port_link_sm());
    pd_new_clone_p->set_link_bring_up_timer(pd_p->link_bring_up_timer());

    pd_new_clone_p->set_mac_id(pd_p->mac_id());
    pd_new_clone_p->set_mac_ch(pd_p->mac_ch());
    pd_new_clone_p->set_num_lanes(pd_p->num_lanes());

    return pd_new_clone_p;
}

//-----------------------------------------------------------------------------
// PD Port mem free
//-----------------------------------------------------------------------------
sdk_ret_t
port_mem_free_pd (port_args_pd_t *args)
{
    sdk_ret_t            ret = SDK_RET_OK;
    linkmgr::pd::port    *pd_p = (linkmgr::pd::port*)args->pd_p;

    SDK_TRACE_DEBUG("{}: port mem_free", __FUNCTION__);
    g_linkmgr_state_pd->port_slab()->free(pd_p);

    return ret;
}

bool
port_has_speed_changed_pd (port_args_pd_t *args)
{
    linkmgr::pd::port *pd_p = (linkmgr::pd::port *)args->pd_p;
    return (args->port_speed != pd_p->port_speed());
}

bool
port_has_admin_state_changed_pd (port_args_pd_t *args)
{
    linkmgr::pd::port *pd_p = (linkmgr::pd::port *)args->pd_p;
    return (args->admin_state != pd_p->admin_state());
}

}    // namespace pd
}    // namespace linkmgr
}    // namespace sdk

// TODO
// WTH is this ??
uint32_t
read_reg_base (uint32_t chip, uint64_t addr)
{
    uint32_t data = 0x0;

    if (sdk::lib::pal_reg_read(addr, &data) != sdk::lib::PAL_RET_OK) {
        SDK_TRACE_ERR("{} read failed", __FUNCTION__);
    }

    return data;
}

void write_reg_base(uint32_t chip, uint64_t addr, uint32_t  data)
{
    if (sdk::lib::pal_reg_write(addr, data) != sdk::lib::PAL_RET_OK) {
        SDK_TRACE_ERR("{} write failed", __FUNCTION__);
    }

    uint32_t read_data = read_reg_base(chip, addr);
    SDK_TRACE_DEBUG("{0:s} read_data after write {1:x}", __FUNCTION__, read_data);
}
