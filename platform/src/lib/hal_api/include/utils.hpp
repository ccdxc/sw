
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "hal_types.hpp"

bool is_multicast(uint64_t mac);

#define VERIFY_HAL() \
    if (!hal) {                                     \
        ret = HAL_IRISC_RET_HAL_DOWN;               \
        NIC_LOG_DEBUG("Hal down: ret: {}", ret);    \
        goto end;                                   \
    }

#endif /* __UTILS_HPP__ */
