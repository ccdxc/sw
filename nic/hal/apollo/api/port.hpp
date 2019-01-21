/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   APIs for port implementation
 */

#include "nic/sdk/linkmgr/linkmgr.hpp"

#if !defined (__PORT_HPP__)
#define __PORT_HPP__

namespace api {

#define OCI_MAX_PORT        16

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
 * @brief     create all ports based on the catalog information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t create_ports(void);

}    // namespace api

#endif    /** __PORT_HPP__ */
