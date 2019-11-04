/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    utils.hpp
 *
 * @brief   helper functions
 */

#if !defined (__UTILS_HPP__)
#define __UTILS_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**
 * @brief    given two epochs, pick invalid one or the lowest numbered epoch so
 *           we can overwrite its contents, and return its index
 * @param[in] epoch1    epoch 1 value
 * @param[in] epoch2    epoch 2 value
 * @return index of lowered numbered epoch or 1st invalid one
 */
// TODO: we will handle the case when epoch rollsover later !!
static inline uint8_t
pick_older_epoch_idx (uint32_t epoch1, uint32_t epoch2)
{
    if (epoch1 == PDS_EPOCH_INVALID) {
        return 0;
    } else if ((epoch2 == PDS_EPOCH_INVALID) || (epoch2 < epoch1)) {
        return 1;
    }
    return 0;
}

}    // namespace api

#endif    /** __UTILS_HPP__ */

