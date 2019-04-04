/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   APIs for port implementation
 */

#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/apollo/api/include/pds.hpp"

#if !defined (__PORT_HPP__)
#define __PORT_HPP__

namespace api {

#define PDS_MAX_PORT        16

typedef void (*port_get_cb_t)(sdk::linkmgr::port_args_t *port_info, void *ctxt);
/**
  * @brief    get port information based on port number
  * @param[in]    fp_port     front panel port number or 0 for all ports
  * @param[in]    port_get_cb callback invoked per port
  * @param[in]    ctxt        opaque context passed back to the callback
  * @return    SDK_RET_OK on success, failure status code on error
  */
sdk_ret_t port_get(uint32_t fp_port, port_get_cb_t port_get_cb, void *ctxt);

/**
 * @brief        get port information based on port number
 * @param[in]    ifindex       interface index
 * @param[in]    admin_state   port admin state
 * @return       SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t update_port(pds_ifindex_t ifindex, port_admin_state_t admin_state);

/**
 * @brief     create all ports based on the catalog information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t create_ports(void);

/**
 * @brief        Handle transceiver insert/remove events
 * @param[in]    xcvr_event_info    transceiver info filled by linkmgr
 */
void xcvr_event_cb(xcvr_event_info_t *xcvr_event_info);

/**
 * @brief        Handle link UP/Down events
 * @param[in]    port_num    port number of the port
 * @param[in]    event       link UP/Down event
 * @param[in]    port_speed  speed of the port
 */
void port_event_cb(uint32_t port_num,
                   port_event_t event,
                   port_speed_t port_speed);
}    // namespace api

#endif    /** __PORT_HPP__ */
