/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    port.hpp
 *
 * @brief   APIs for port implementation
 */

#if !defined (__PORT_HPP__)
#define __PORT_HPP__

namespace api {

/**
 * @brief     create all ports based on the catalog information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t create_ports(void);

}    // namespace api

#endif    /** __PORT_HPP__ */
