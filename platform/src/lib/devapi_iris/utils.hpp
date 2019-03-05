//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "include/sdk/base.hpp"

namespace iris {

bool is_multicast(uint64_t mac);

#define VERIFY_HAL() \
    if (!hal) {                                     \
        ret = SDK_RET_COMM_FAIL;                    \
        NIC_LOG_DEBUG("Hal down: ret: {}", ret);    \
        goto end;                                   \
    }

} // namepsace iris

#endif /* __UTILS_HPP__ */
