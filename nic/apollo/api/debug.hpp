/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    debug.hpp
 *
 * @brief   APIs for troubleshooting
 */

#include "nic/sdk/include/sdk/base.hpp"

#if !defined (__DEBUG_HPP__)
#define __DEBUG_HPP__

namespace debug {

sdk_ret_t system_dump(const char *filename);

}    // namespace debug

#endif    /** __DEBUG_HPP__ */
