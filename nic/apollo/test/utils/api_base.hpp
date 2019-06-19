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

#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/utils/base.hpp"

namespace api_test {

template <typename feeder_T, typename info_T>
sdk::sdk_ret_t api_info_compare(feeder_T& feeder, info_T *info) {
    if (!feeder.key_compare(&info->spec.key)) {
        std::cout << "key compare failed; " << feeder << info << std::endl;
        return sdk::SDK_RET_ERR;
    }

    if (!feeder.spec_compare(&info->spec)) {
        std::cout << "spec compare failed; " << feeder << info << std::endl;
        return sdk::SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

template <typename feeder_T, typename info_T>
sdk::sdk_ret_t api_info_compare_singleton(
    feeder_T& feeder, info_T *info) {

    if (!feeder.spec_compare(&info->spec)) {
        std::cout << "spec compare failed; " << feeder << info << std::endl;
        return sdk::SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

#define API_CREATE(_api_str)                                                 \
inline sdk::sdk_ret_t                                                        \
create(_api_str##_feeder& feeder) {                                          \
    pds_##_api_str##_spec_t spec;                                            \
                                                                             \
    feeder.spec_build(&spec);                                                \
    return (pds_##_api_str##_create(&spec));                                 \
}

#define API_NO_READ(_api_str)                                                \
inline sdk::sdk_ret_t                                                        \
read(_api_str##_feeder& feeder) {                                            \
    return sdk::SDK_RET_OK;                                                  \
}

#define API_READ(_api_str)                                                   \
inline sdk::sdk_ret_t                                                        \
read(_api_str##_feeder& feeder) {                                            \
    sdk_ret_t rv;                                                            \
    pds_##_api_str##_key_t key;                                              \
    pds_##_api_str##_info_t info;                                            \
                                                                             \
    feeder.key_build(&key);                                                  \
    memset(&info, 0, sizeof(pds_##_api_str##_info_t));                       \
    if ((rv = pds_##_api_str##_read(&key, &info)) != sdk::SDK_RET_OK)        \
        return rv;                                                           \
                                                                             \
    return (api_info_compare<_api_str##_feeder, pds_##_api_str##_info_t>(    \
                feeder, &info));                                             \
}

#define API_READ_SINGLETON(_api_str)                                         \
inline sdk::sdk_ret_t                                                        \
read(_api_str##_feeder& feeder) {                                            \
    sdk_ret_t rv;                                                            \
    pds_##_api_str##_info_t info;                                            \
                                                                             \
    memset(&info, 0, sizeof(pds_##_api_str##_info_t));                       \
    if ((rv = pds_##_api_str##_read(&info)) != sdk::SDK_RET_OK)              \
        return rv;                                                           \
                                                                             \
    return (api_info_compare_singleton<_api_str##_feeder,                    \
            pds_##_api_str##_info_t>(feeder, &info));                        \
}

#define API_UPDATE(_api_str)                                                 \
inline sdk::sdk_ret_t                                                        \
update(_api_str##_feeder& feeder) {                                          \
    pds_##_api_str##_spec_t spec;                                            \
                                                                             \
    feeder.spec_build(&spec);                                                \
    return (pds_##_api_str##_update(&spec));                                 \
}

#define API_DELETE(_api_str)                                                 \
inline sdk::sdk_ret_t                                                        \
del(_api_str##_feeder& feeder) {                                             \
    pds_##_api_str##_key_t key;                                              \
                                                                             \
    feeder.key_build(&key);                                                  \
    return (pds_##_api_str##_delete(&key));                                  \
}

#define API_DELETE_SINGLETON(_api_str)                                       \
inline sdk::sdk_ret_t                                                        \
del(_api_str##_feeder& feeder) {                                             \
    return (pds_##_api_str##_delete());                                      \
}

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
    if (feeder.read_unsupported())
        return;

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
