//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the object util class
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_API_BASE_HPP__
#define __TEST_UTILS_API_BASE_HPP__

#include "nic/sdk/include/sdk/base.hpp"

namespace api_test {

template <typename feeder_T>
void many_create(feeder_T& feeder) {
    feeder_T tmp = feeder;
    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        SDK_ASSERT(create(tmp) == sdk::SDK_RET_OK);
    }
}

template <typename feeder_T>
void many_read(feeder_T& feeder,
               sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK) {
    feeder_T tmp = feeder;
    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        SDK_ASSERT(read(tmp) == expected_result);
    }
}

template <typename feeder_T>
void many_update(feeder_T& feeder) {
    feeder_T tmp = feeder;
    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        SDK_ASSERT(update(tmp) == sdk::SDK_RET_OK);
    }
}

template <typename feeder_T>
void many_delete(feeder_T& feeder) {
    feeder_T tmp = feeder;
    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        SDK_ASSERT(del(tmp) == sdk::SDK_RET_OK);
    }
}

}    // namespace api_test

#endif    // __TEST_UTILS_API_BASE_HPP__
