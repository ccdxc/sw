/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   This file handles port operations
 */
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/hal/apollo/core/trace.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"

namespace api {

void *g_port_store[8];

/**
 * @brief        create a port with the given configuration information
 * @param[in]    port_args    port parameters filled by this API
 * @return       SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
create_port (port_args_t *port_args)
{
    void    *port_handle;

    port_args->user_admin_state = port_args->admin_state;
    sdk::linkmgr::port_args_set_by_xcvr_state(port_args);
    port_handle = sdk::linkmgr::port_create(port_args);
    g_port_store[port_args->port_num] = port_handle;
    if (port_handle == NULL) {
        OCI_TRACE_ERR("port %u create failed", port_args->port_num);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

/**
 * @brief        populate port information based on the catalog
 * @param[in]    fp_port      front panel port number
 * @param[in]    port_args    port parameters filled by this API
 * @return       SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
populate_port_info (uint32_t fp_port, port_args_t *port_args)
{
    port_args->port_num = fp_port;
    port_args->port_type = g_oci_state.catalogue()->port_type_fp(fp_port);
    port_args->port_speed = g_oci_state.catalogue()->port_speed(fp_port);
    port_args->admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
    port_args->num_lanes = g_oci_state.catalogue()->num_lanes_fp(fp_port);
    port_args->breakout_modes =
        g_oci_state.catalogue()->breakout_modes(fp_port);

    return SDK_RET_OK;
}

/**
 * @brief     create all ports based on the catalog information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
create_ports (void)
{
    uint32_t       num_fp_ports;
    port_args_t    port_args;

    num_fp_ports = g_oci_state.catalogue()->num_fp_ports();
    for (uint32_t fp_port = 1; fp_port < num_fp_ports; fp_port++) {
        memset(&port_args, 0, sizeof(port_args));
        populate_port_info(fp_port, &port_args);
        create_port(&port_args);
    }
    return SDK_RET_OK;
}

}    // namespace api
