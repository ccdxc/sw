/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   This file handles port operations
 */

#include "nic/sdk/linkmgr/port.hpp"
#include "nic/sdk/platform/drivers/xcvr.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**< port handles indexed by the front panel port number (starting from 1) */
void *g_port_store[PDS_MAX_PORT + 1];

/**
 * @brief        Handle link UP/Down events
 * @param[in]    port_num    port number of the port
 * @param[in]    event       link UP/Down event
 * @param[in]    port_speed  speed of the port
 */
void
port_event_cb (uint32_t port_num, port_event_t event, port_speed_t port_speed)
{
    sdk::linkmgr::port_set_leds(port_num, event);
}

/**
 * @brief        Handle transceiver insert/remove events
 * @param[in]    xcvr_event_info    transceiver info filled by linkmgr
 */
void
xcvr_event_cb (xcvr_event_info_t *xcvr_event_info)
{
    int phy_port = 0;

    /**< ignore xcvr events if xcvr valid check is disabled */
    if (!sdk::platform::xcvr_valid_check_enabled()) {
        return;
    }

    /**
     * if xcvr is removed, bring link down
     * if xcvr sprom read is successful, bring linkup if user admin enabled.
     * ignore all other xcvr states.
     */
    if (xcvr_event_info->state != xcvr_state_t::XCVR_REMOVED &&
        xcvr_event_info->state != xcvr_state_t::XCVR_SPROM_READ) {
        return;
    }

    for (int port = 0; port < PDS_MAX_PORT+1; ++port) {
        if (g_port_store[port] != NULL) {
            phy_port = sdk::lib::catalog::logical_port_to_phy_port(port);
            if (phy_port == -1 ||
                phy_port != (int)xcvr_event_info->phy_port) {
                continue;
            }
            sdk::linkmgr::port_update_xcvr_event(g_port_store[port],
                                                 xcvr_event_info);
            // TODO: notify interested parties
        }
    }
}

/**
 * @brief        create a port with the given configuration information
 * @param[in]    port_args    port parameters filled by this API
 * @return       SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
create_port (port_args_t *port_args)
{
    void    *port_handle;

    PDS_TRACE_DEBUG("Creating port %u", port_args->port_num);

    /**
     * store user configured admin_state in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->user_admin_state = port_args->admin_state;

    /**
     * store user configured AN in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->auto_neg_cfg     = port_args->auto_neg_enable;

    /**
     * store user configured num_lanes in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->num_lanes_cfg    = port_args->num_lanes;

    sdk::linkmgr::port_args_set_by_xcvr_state(port_args);
    port_handle = sdk::linkmgr::port_create(port_args);
    g_port_store[port_args->port_num] = port_handle;
    if (port_handle == NULL) {
        PDS_TRACE_ERR("port %u create failed", port_args->port_num);
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
    port_args->port_type = g_pds_state.catalogue()->port_type_fp(fp_port);
    if (port_args->port_type == port_type_t::PORT_TYPE_ETH) {
        port_args->port_speed = port_speed_t::PORT_SPEED_100G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
    } else if (port_args->port_type == port_type_t::PORT_TYPE_MGMT) {
        port_args->port_speed = port_speed_t::PORT_SPEED_1G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
    }
    port_args->admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
    port_args->num_lanes = g_pds_state.catalogue()->num_lanes_fp(fp_port);
    asic = sdk::linkmgr::port::port_num_to_asic_num(port_num);
    asic_port = sdk::linkmgr::port::port_num_to_asic_port(port_num);
    port_args->mac_id =
        g_pds_state.catalogue()->asic_port_to_mac_id(asic, asic_port);
    port_args->mac_ch =
        g_pds_state.catalogue()->asic_port_to_mac_ch(asic, asic_port);
    if (port_args->port_type != port_type_t::PORT_TYPE_MGMT) {
        port_args->auto_neg_enable = true;
    }
    port_args->debounce_time = 0;
    port_args->mtu = 0;    /**< default will be set to max mtu */
    port_args->pause = port_pause_type_t::PORT_PAUSE_TYPE_LINK;
    port_args->loopback_mode = port_loopback_mode_t::PORT_LOOPBACK_MODE_NONE;

    for (uint32_t i = 0; i < port_args->num_lanes; i++) {
        port_args->sbus_addr[i] =
            g_pds_state.catalogue()->sbus_addr(asic, asic_port, i);
    }
    port_args->breakout_modes =
        g_pds_state.catalogue()->breakout_modes(fp_port);

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

    num_fp_ports = g_pds_state.catalogue()->num_fp_ports();
    for (uint32_t fp_port = 1, port_num = 1;
         fp_port <= num_fp_ports; fp_port++) {
        memset(&port_args, 0, sizeof(port_args));
        populate_port_info(fp_port, port_num, &port_args);
        create_port(&port_args);
        port_num += g_pds_state.catalogue()->num_lanes_fp(fp_port);
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
        for (uint32_t fp_port = 1, port_num = 1;
             fp_port <= g_pds_state.catalogue()->num_fp_ports(); fp_port++) {
            if (g_port_store[port_num] == NULL)  {
                PDS_TRACE_ERR("Port %u not created, skipping", port_num);
                continue;
            }
            memset(&port_info, 0, sizeof(port_info));
            port_info.stats_data = stats_data;
            ret = sdk::linkmgr::port_get(g_port_store[port_num], &port_info);
            if (ret != sdk::SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to get port %u info", port_num);
                continue;
            }
            port_info.port_num =
                g_pds_state.catalogue()->logical_port_to_fp_port(port_info.port_num);
            /** call the per port callback for this port */
            port_get_cb(&port_info, ctxt);
            port_num += g_pds_state.catalogue()->num_lanes_fp(fp_port);
        }
    } else {
        uint32_t port_num = g_pds_state.catalogue()->fp_port_to_logical_port(fp_port);

        if (port_num > PDS_MAX_PORT) {
            PDS_TRACE_ERR("Invalid port number %u", fp_port);
            return SDK_RET_INVALID_ARG;
        }

        if (g_port_store[port_num] == NULL)  {
            PDS_TRACE_ERR("Port %u not created", fp_port);
            return SDK_RET_INVALID_OP;
        }

        memset(&port_info, 0, sizeof(port_info));
        port_info.stats_data = stats_data;
        ret = sdk::linkmgr::port_get(g_port_store[port_num], &port_info);
        if (ret != sdk::SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to get port %u info", fp_port);
            return ret;
        }
        port_info.port_num =
            g_pds_state.catalogue()->logical_port_to_fp_port(port_info.port_num);
        /** call the per port callback for this port */
        port_get_cb(&port_info, ctxt);
    }
    return SDK_RET_OK;
}

}    // namespace api
