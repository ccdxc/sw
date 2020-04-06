/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    debug.hpp
 *
 * @brief   APIs for troubleshooting
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/apollo/framework/state_base.hpp"

#if !defined (__DEBUG_HPP__)
#define __DEBUG_HPP__

namespace debug {

typedef struct dump_port_fsm_record_args_s {
    int        fd;
    uint32_t   index;
    timespec_t prev_ts;
} dump_port_fsm_record_args_t;

}    // namespace debug

#endif    /** __DEBUG_HPP__ */
