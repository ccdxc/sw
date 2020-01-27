/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   This file handles port operations
 */

#include "nic/sdk/linkmgr/port.hpp"
#include "nic/sdk/platform/drivers/xcvr.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/port.hpp"

namespace api {

/**
 * @brief        Handle link UP/Down events
 * @param[in]    port_event_info port event information
 */
void
port_event_cb (port_event_info_t *port_event_info)
{
    ::core::event_t event;
    port_event_t port_event = port_event_info->event;
    port_speed_t port_speed = port_event_info->speed;
    uint32_t logical_port = port_event_info->logical_port;

    sdk::linkmgr::port_set_leds(logical_port, port_event);

    memset(&event, 0, sizeof(event));
    event.event_id = EVENT_ID_PORT_STATUS;
    event.port.ifindex =
        sdk::lib::catalog::logical_port_to_ifindex(logical_port);
    event.port.event = port_event;
    event.port.speed = port_speed;
    sdk::ipc::broadcast(EVENT_ID_PORT_STATUS, &event, sizeof(event));
}

bool
xvcr_event_walk_cb (void *entry, void *ctxt)
{
    int phy_port;
    ::core::event_t event;
    uint32_t logical_port;
    pds_ifindex_t ifindex;
    if_entry *intf = (if_entry *)entry;
    xcvr_event_info_t *xcvr_event_info = (xcvr_event_info_t *)ctxt;

    ifindex = intf->ifindex();
    logical_port = sdk::lib::catalog::ifindex_to_logical_port(ifindex);
    phy_port = sdk::lib::catalog::logical_port_to_phy_port(logical_port);
    if ((phy_port == -1) ||
        (phy_port != (int)xcvr_event_info->phy_port)) {
        return false;
    }
    sdk::linkmgr::port_update_xcvr_event(intf->port_info(), xcvr_event_info);

    memset(&event, 0, sizeof(event));
    event.xcvr.ifindex = ifindex;
    event.xcvr.state = xcvr_event_info->state;
    event.xcvr.pid = xcvr_event_info->pid;
    event.xcvr.cable_type = xcvr_event_info->cable_type;
    memcpy(event.xcvr.sprom, xcvr_event_info->xcvr_sprom, XCVR_SPROM_SIZE);
    sdk::ipc::broadcast(EVENT_ID_XCVR_STATUS, &event, sizeof(event));
    return false;
}

/**
 * @brief        Handle transceiver insert/remove events
 * @param[in]    xcvr_event_info    transceiver info filled by linkmgr
 */
void
xcvr_event_cb (xcvr_event_info_t *xcvr_event_info)
{
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
    if_db()->walk(IF_TYPE_ETH, xvcr_event_walk_cb, xcvr_event_info);
}

/**
 * @brief        update a port with the given configuration information
 * @param[in]    key         key/uuid of the port
 * @param[in]    port_admin_state_t    port admin state
 * @return       SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
update_port (const pds_obj_key_t *key, port_args_t *api_port_info)
{
    sdk_ret_t ret;
    if_entry *intf;
    port_args_t port_info;

    intf = if_db()->find(key);
    if (intf == NULL) {
        PDS_TRACE_ERR("port %s update failed", key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memset(&port_info, 0, sizeof(port_info));
    ret = sdk::linkmgr::port_get(intf->port_info(), &port_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to get port %s info, err %u", key->str(), ret);
        return ret;
    }
    port_info.port_num = intf->ifindex();

    port_info.user_admin_state = api_port_info->user_admin_state;
    port_info.admin_state = api_port_info->admin_state;
    port_info.port_speed = api_port_info->port_speed;
    port_info.fec_type = api_port_info->fec_type;
    port_info.auto_neg_cfg = api_port_info->auto_neg_cfg;
    port_info.auto_neg_enable = api_port_info->auto_neg_enable;
    port_info.debounce_time = api_port_info->debounce_time;
    port_info.mtu = api_port_info->mtu;
    port_info.pause = api_port_info->pause;
    port_info.loopback_mode = api_port_info->loopback_mode;
    port_info.num_lanes_cfg = api_port_info->num_lanes_cfg;
    port_info.num_lanes = api_port_info->num_lanes;

    ret = sdk::linkmgr::port_update(intf->port_info(), &port_info);
    return ret;
}

/**
 * @brief        create a port with the given configuration information
 * @param[in]    ifindex      interface index
 * @param[in]    port_args    port parameters filled by this API
 * @return       SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
create_port (pds_ifindex_t ifindex, port_args_t *port_args)
{
    if_entry *intf;
    void *port_info;
    pds_obj_key_t key;

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
    port_args->auto_neg_cfg = port_args->auto_neg_enable;

    /**
     * store user configured num_lanes in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->num_lanes_cfg = port_args->num_lanes;

    sdk::linkmgr::port_args_set_by_xcvr_state(port_args);
    port_info = sdk::linkmgr::port_create(port_args);
    if (port_info == NULL) {
        PDS_TRACE_ERR("port %u create failed", port_args->port_num);
        return SDK_RET_ERR;
    }
    key = uuid_from_objid(ifindex);
    intf = if_entry::factory(key, ifindex);
    if (intf == NULL) {
        sdk::linkmgr::port_delete(port_info);
        return SDK_RET_ERR;
    }
    intf->set_port_info(port_info);
    if_db()->insert(intf);
    return SDK_RET_OK;
}

/**
 * @brief        populate port information based on the catalog
 * @param[in]    fp_port      front panel port number
 * @param[in]    port_args    port parameters filled by this API
 * @return       SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
populate_port_info (pds_ifindex_t ifindex, uint32_t phy_port,
                    port_args_t *port_args)
{
    uint32_t    logical_port;

    logical_port = port_args->port_num =
        sdk::lib::catalog::ifindex_to_logical_port(ifindex);
    port_args->port_type = g_pds_state.catalogue()->port_type_fp(phy_port);
    if (port_args->port_type == port_type_t::PORT_TYPE_ETH) {
        port_args->port_speed = port_speed_t::PORT_SPEED_100G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
    } else if (port_args->port_type == port_type_t::PORT_TYPE_MGMT) {
        port_args->port_speed = port_speed_t::PORT_SPEED_1G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
    }
    port_args->admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
    port_args->num_lanes = g_pds_state.catalogue()->num_lanes_fp(phy_port);
    port_args->mac_id = g_pds_state.catalogue()->mac_id(logical_port, 0);
    port_args->mac_ch = g_pds_state.catalogue()->mac_ch(logical_port, 0);
    if (port_args->port_type != port_type_t::PORT_TYPE_MGMT) {
        port_args->auto_neg_enable = true;
    }
    port_args->debounce_time = 0;
    port_args->mtu = 0;    /**< default will be set to max mtu */
    port_args->pause = port_pause_type_t::PORT_PAUSE_TYPE_NONE;
    port_args->loopback_mode = port_loopback_mode_t::PORT_LOOPBACK_MODE_NONE;

    for (uint32_t i = 0; i < port_args->num_lanes; i++) {
        port_args->sbus_addr[i] =
            g_pds_state.catalogue()->sbus_addr(logical_port, i);
    }
    port_args->breakout_modes =
        g_pds_state.catalogue()->breakout_modes(phy_port);

    return SDK_RET_OK;
}

/**
 * @brief     create all ports based on the catalog information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
create_ports (void)
{
    uint32_t       num_phy_ports;
    port_args_t    port_args;
    pds_ifindex_t ifindex;

    PDS_TRACE_DEBUG("Creating ports ...");
    num_phy_ports = g_pds_state.catalogue()->num_fp_ports();
    for (uint32_t phy_port = 1; phy_port <= num_phy_ports; phy_port++) {
        ifindex = ETH_IFINDEX(g_pds_state.catalogue()->slot(),
                              phy_port, ETH_IF_DEFAULT_CHILD_PORT);
        memset(&port_args, 0, sizeof(port_args));
        populate_port_info(ifindex, phy_port, &port_args);
        create_port(ifindex, &port_args);
    }
    return SDK_RET_OK;
}

typedef struct port_get_cb_ctxt_s {
    void *ctxt;
    port_get_cb_t port_get_cb;
} port_get_cb_ctxt_t;

bool
if_walk_port_get_cb (void *entry, void *ctxt)
{
    int phy_port;
    sdk_ret_t ret;
    pds_obj_key_t key;
    port_args_t port_info;
    if_entry *intf = (if_entry *)entry;
    uint64_t stats_data[MAX_MAC_STATS];
    port_get_cb_ctxt_t *cb_ctxt = (port_get_cb_ctxt_t *)ctxt;

    key = intf->key();
    memset(&port_info, 0, sizeof(port_info));
    port_info.stats_data = stats_data;
    ret = sdk::linkmgr::port_get(intf->port_info(), &port_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to get port 0x%x info, err %u", intf->key(), ret);
        return false;
    }
    port_info.port_num = intf->ifindex();
    phy_port = sdk::lib::catalog::ifindex_to_phy_port(port_info.port_num);
    if (phy_port != -1) {
        ret = sdk::platform::xcvr_get(phy_port - 1, &port_info.xcvr_event_info);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to get xcvr for port %u, err %u",
                          phy_port, ret);
        }
    }
    // TODO: @akoradha port_args is exposed all the way to the agent
    //       with the current design, we should create port_spec_t,
    //       port_status_t and port_stats_t like any other object or
    //       better approach is to fold all the port stuff into if_entry
    //       and CLIs etc. will naturally work with current db walks etc.
    //       we have all eth ports in if db already. with port_args_t
    //       going directly upto agent svc layer, there is no way to send uuid
    //       now), so hijacking this pointer field
    port_info.port_an_args = (port_an_args_t *)&key;
    cb_ctxt->port_get_cb(&port_info, cb_ctxt->ctxt);
    return false;
}

/**
 * @brief    get port information based on port number
 * @param[in]    key         key/uuid of the port or k_pds_obj_key_invalid for
 *                           all ports
 * @param[in]    port_get_cb callback invoked per port
 * @param[in]    ctxt        opaque context passed back to the callback
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
port_get (const pds_obj_key_t *key, port_get_cb_t port_get_cb, void *ctxt)
{
    if_entry *intf;
    port_get_cb_ctxt_t cb_ctxt;

    cb_ctxt.ctxt = ctxt;
    cb_ctxt.port_get_cb = port_get_cb;
    if (*key == k_pds_obj_key_invalid) {
        if_db()->walk(IF_TYPE_ETH, if_walk_port_get_cb, &cb_ctxt);
    } else {
        intf = if_db()->find(key);
        if (intf == NULL)  {
            PDS_TRACE_ERR("Port %s not found", key->str());
            return SDK_RET_INVALID_OP;
        }
        if_walk_port_get_cb(intf, &cb_ctxt);
    }
    return SDK_RET_OK;
}

}    // namespace api
