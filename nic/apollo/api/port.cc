/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   This file handles port operations
 */

#include "nic/sdk/linkmgr/port.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/oci_state.hpp"

namespace api {

/**< port handles indexed by the front panel port number (starting from 1) */
void *g_port_store[OCI_MAX_PORT + 1];

/**
 * @brief        create a port with the given configuration information
 * @param[in]    port_args    port parameters filled by this API
 * @return       SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
create_port (port_args_t *port_args)
{
    void    *port_handle;

    OCI_TRACE_DEBUG("Creating port %u", port_args->port_num);
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
populate_port_info (uint32_t fp_port, uint32_t port_num, port_args_t *port_args)
{
    uint32_t    asic, asic_port;

    port_args->port_num = port_num;
    port_args->port_type = g_oci_state.catalogue()->port_type_fp(fp_port);
    if (port_args->port_type == port_type_t::PORT_TYPE_ETH) {
        port_args->port_speed = port_speed_t::PORT_SPEED_100G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
    } else if (port_args->port_type == port_type_t::PORT_TYPE_MGMT) {
        port_args->port_speed = port_speed_t::PORT_SPEED_1G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
    }
    port_args->admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
    port_args->num_lanes = g_oci_state.catalogue()->num_lanes_fp(fp_port);
    asic = sdk::linkmgr::port::port_num_to_asic_num(port_num);
    asic_port = sdk::linkmgr::port::port_num_to_asic_port(port_num);
    port_args->mac_id =
        g_oci_state.catalogue()->asic_port_to_mac_id(asic, asic_port);
    port_args->mac_ch =
        g_oci_state.catalogue()->asic_port_to_mac_ch(asic, asic_port);
    port_args->auto_neg_enable = false;
    port_args->debounce_time = 0;
    port_args->mtu = 0;    /**< default will be set to max mtu */
    port_args->pause = port_pause_type_t::PORT_PAUSE_TYPE_LINK;
    port_args->loopback_mode = port_loopback_mode_t::PORT_LOOPBACK_MODE_NONE;

    for (uint32_t i = 0; i < port_args->num_lanes; i++) {
        port_args->sbus_addr[i] =
            g_oci_state.catalogue()->sbus_addr(asic, asic_port, i);
    }
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
    for (uint32_t fp_port = 1, port_num = 1;
         fp_port <= num_fp_ports; fp_port++) {
        memset(&port_args, 0, sizeof(port_args));
        populate_port_info(fp_port, port_num, &port_args);
        create_port(&port_args);
        port_num += g_oci_state.catalogue()->num_lanes_fp(fp_port);
    }
    return SDK_RET_OK;
}

/**
 * @brief    get port information based on port number
 * @param[in]    fp_port     front panel port number or 0 for all ports
 * @param[in]    port_get_cb callback invoked per port
 * @param[in]    ctxt        opaque context passed back to the callback
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
port_get (uint32_t fp_port, port_get_cb_t port_get_cb, void *ctxt)
{
    sdk_ret_t      ret;
    uint64_t       stats_data[MAX_MAC_STATS];
    port_args_t    port_info;

    if (fp_port == 0) {
        /**< iterate over all ports */
        for (uint32_t fp_port = 1;
             fp_port <= g_oci_state.catalogue()->num_fp_ports(); fp_port++) {
            if (g_port_store[fp_port] == NULL)  {
                OCI_TRACE_ERR("Port %u not created, skipping", fp_port);
                continue;
            }
            memset(&port_info, 0, sizeof(port_info));
            port_info.stats_data = stats_data;
            ret = sdk::linkmgr::port_get(g_port_store[fp_port], &port_info);
            if (ret != sdk::SDK_RET_OK) {
                OCI_TRACE_ERR("Failed to get port %u info", fp_port);
                continue;
            }
            /** call the per port callback for this port */
            port_get_cb(&port_info, ctxt);
        }
    } else {
        if (fp_port > OCI_MAX_PORT) {
            OCI_TRACE_ERR("Invalid port number %u", fp_port);
            return SDK_RET_INVALID_ARG;
        }
        if (g_port_store[fp_port] == NULL)  {
            OCI_TRACE_ERR("Port %u not created", fp_port);
            return SDK_RET_INVALID_OP;
        }
        memset(&port_info, 0, sizeof(port_info));
        port_info.stats_data = stats_data;
        ret = sdk::linkmgr::port_get(g_port_store[fp_port], &port_info);
        if (ret != sdk::SDK_RET_OK) {
            OCI_TRACE_ERR("Failed to get port %u info", fp_port);
            return ret;
        }
        /** call the per port callback for this port */
        port_get_cb(&port_info, ctxt);
    }
    return SDK_RET_OK;
}

}    // namespace api
