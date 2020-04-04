//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the object util class
///
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_API_BASE_HPP__
#define __TEST_API_UTILS_API_BASE_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#ifdef AGENT_MODE
#include "nic/apollo/agent/test/client/app.hpp"
#endif
#include "nic/apollo/test/scale/api.hpp"

namespace test {
namespace api {

/// \addtogroup  PDS_FEEDER
/// @{

/// \brief Compare the config with values read from hardware
/// Compares both key and spec
template <typename feeder_T, typename info_T>
sdk_ret_t api_info_compare(feeder_T& feeder, info_T *info) {
    if (!feeder.key_compare(&info->spec.key)) {
        cerr << "key compare failed" << endl;
        cerr << "\t" << feeder << endl << "\t" << info;
        return sdk::SDK_RET_ERR;
    }

    if (!feeder.spec_compare(&info->spec)) {
        cerr << "spec compare failed" << endl;
        cerr << "\t" << feeder << endl << "\t" << info;
        return sdk::SDK_RET_ERR;
    }
    cout << "info compare success" << endl;
    cout << "\t" << feeder << endl << "\t" << info;

    return SDK_RET_OK;
}

/// \brief Compare the config with values read from hardware
/// Compares only spec
template <typename feeder_T, typename info_T>
sdk_ret_t api_info_compare_singleton(
    feeder_T& feeder, info_T *info) {

    if (!feeder.spec_compare(&info->spec)) {
        cerr << "spec compare failed" << endl;
        cerr << "\t" << feeder << endl << "\t" << info;
        return sdk::SDK_RET_ERR;
    }

    cout << "info compare success" << endl;
    cout << "\t" << feeder << endl << "\t" << info;

    return SDK_RET_OK;
}

/// \brief Invokes the PDS create apis for test objects
#define API_CREATE(_api_str)                                                 \
inline sdk_ret_t                                                             \
create(pds_batch_ctxt_t bctxt, _api_str##_feeder& feeder) {                  \
    pds_##_api_str##_spec_t spec;                                            \
                                                                             \
    memset(&spec, 0, sizeof(spec));                                          \
    feeder.spec_build(&spec);                                                \
    return (pds_##_api_str##_create(&spec, bctxt));                          \
}

/// \brief Dummy function for Read
#define API_NO_READ(_api_str)                                                \
inline sdk_ret_t                                                             \
read(_api_str##_feeder& feeder) {                                            \
    return SDK_RET_OK;                                                       \
}

/// \brief Invokes the PDS read apis for test objects
/// Also it compares the config values with values read from hardware
#define API_READ(_api_str)                                                   \
inline sdk_ret_t                                                             \
read(_api_str##_feeder& feeder) {                                            \
    sdk_ret_t rv;                                                            \
    pds_obj_key_t key;                                                       \
    pds_##_api_str##_info_t info;                                            \
                                                                             \
    memset(&key, 0, sizeof(key));                                            \
    feeder.key_build(&key);                                                  \
    memset(&info, 0, sizeof(info));                                          \
    if ((rv = pds_##_api_str##_read(&key, &info)) != SDK_RET_OK)             \
        return rv;                                                           \
                                                                             \
    return (api_info_compare<_api_str##_feeder, pds_##_api_str##_info_t>(    \
                feeder, &info));                                             \
}

/// \brief Invokes the PDS read apis for test objects
/// Also it compares the config values with values read from hardware
#define API_READ_SINGLETON(_api_str)                                         \
inline sdk_ret_t                                                             \
read(_api_str##_feeder& feeder) {                                            \
    sdk_ret_t rv;                                                            \
    pds_##_api_str##_info_t info;                                            \
                                                                             \
    memset(&info, 0, sizeof(info));                                          \
    if ((rv = pds_##_api_str##_read(&info)) != SDK_RET_OK)                   \
        return rv;                                                           \
                                                                             \
    return (api_info_compare_singleton<_api_str##_feeder,                    \
            pds_##_api_str##_info_t>(feeder, &info));                        \
}

/// \brief Invokes the PDS update apis for test objects
#define API_UPDATE(_api_str)                                                 \
inline sdk_ret_t                                                             \
update(pds_batch_ctxt_t bctxt, _api_str##_feeder& feeder) {                  \
    pds_##_api_str##_spec_t spec;                                            \
                                                                             \
    memset(&spec, 0, sizeof(spec));                                          \
    feeder.spec_build(&spec);                                                \
    return (pds_##_api_str##_update(&spec, bctxt));                          \
}

/// \brief Invokes the PDS delete apis for test objects
#define API_DELETE(_api_str)                                                 \
inline sdk_ret_t                                                             \
del(pds_batch_ctxt_t bctxt, _api_str##_feeder& feeder) {                     \
    pds_obj_key_t key;                                                       \
                                                                             \
    memset(&key, 0, sizeof(key));                                            \
    feeder.key_build(&key);                                                  \
    return (pds_##_api_str##_delete(&key, bctxt));                           \
}

// TOD: remove this macro later
#define API_READ_TMP(_api_str)                                               \
inline sdk_ret_t                                                             \
read(_api_str##_feeder& feeder) {                                            \
    sdk_ret_t rv;                                                            \
    pds_##_api_str##_key_t key;                                              \
    pds_##_api_str##_info_t info;                                            \
                                                                             \
    memset(&key, 0, sizeof(key));                                            \
    feeder.key_build(&key);                                                  \
    memset(&info, 0, sizeof(info));                                          \
    if ((rv = pds_##_api_str##_read(&key, &info)) != SDK_RET_OK)             \
        return rv;                                                           \
                                                                             \
    return (api_info_compare<_api_str##_feeder, pds_##_api_str##_info_t>(    \
                feeder, &info));                                             \
}

// TOD: remove this macro later
/// \brief Invokes the PDS delete apis for test objects
#define API_DELETE_TMP(_api_str)                                             \
inline sdk_ret_t                                                             \
del(pds_batch_ctxt_t bctxt, _api_str##_feeder& feeder) {                     \
    pds_##_api_str##_key_t key;                                              \
                                                                             \
    memset(&key, 0, sizeof(key));                                            \
    feeder.key_build(&key);                                                  \
    return (pds_##_api_str##_delete(&key, bctxt));                           \
}

/// \brief Invokes the PDS delete apis for test objects
#define API_DELETE_SINGLETON(_api_str)                                       \
inline sdk_ret_t                                                             \
del(pds_batch_ctxt_t bctxt, _api_str##_feeder& feeder) {                     \
    return (pds_##_api_str##_delete(bctxt));                                 \
}

// TODO - Defining the following APIs temporarily till all objects are changed
#define API_CREATE1(_api_str)                                                \
inline sdk_ret_t                                                             \
create(_api_str##_feeder& feeder) {                                          \
    pds_##_api_str##_spec_t spec;                                            \
                                                                             \
    if (feeder.num_obj == 0)                                                 \
        return (create_##_api_str(NULL));                                    \
    feeder.spec_build(&spec);                                                \
    return (create_##_api_str(&spec));                                       \
}

#define API_READ1(_api_str)                                                  \
inline sdk_ret_t                                                             \
read(_api_str##_feeder& feeder) {                                            \
    sdk_ret_t rv;                                                            \
    pds_obj_key_t key;                                                       \
    pds_##_api_str##_info_t info;                                            \
                                                                             \
    feeder.key_build(&key);                                                  \
    memset(&info, 0, sizeof(info));                                          \
    if ((rv = read_##_api_str(&key, &info)) != SDK_RET_OK)                   \
        return rv;                                                           \
                                                                             \
    return (api_info_compare<_api_str##_feeder, pds_##_api_str##_info_t>(    \
                feeder, &info));                                             \
}

#define API_UPDATE1(_api_str)                                                \
inline sdk_ret_t                                                             \
update(_api_str##_feeder& feeder) {                                          \
    pds_##_api_str##_spec_t spec;                                            \
                                                                             \
    if (feeder.num_obj == 0)                                                 \
        return (update_##_api_str(NULL));                                    \
    feeder.spec_build(&spec);                                                \
    return (update_##_api_str(&spec));                                       \
}

#define API_DELETE1(_api_str)                                                \
inline sdk_ret_t                                                             \
del(_api_str##_feeder& feeder) {                                             \
    pds_obj_key_t key;                                                       \
                                                                             \
    if (feeder.num_obj == 0)                                                 \
        return (delete_##_api_str(NULL));                                    \
    feeder.key_build(&key);                                                  \
    return (delete_##_api_str(&key));                                        \
}

/// \brief Invokes the create apis for all the config objects
template <typename feeder_T>
sdk_ret_t many_create(pds_batch_ctxt_t bctxt, feeder_T& feeder) {
    sdk_ret_t ret;
    feeder_T tmp = feeder;

    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        if ((ret = create(bctxt, tmp)) != SDK_RET_OK) {
            cerr << "MANY CREATE failed" << endl;
            cerr << "\t" << tmp << endl;
            return ret;
        }
    }
    return ret;
}

/// \brief Invokes the read apis for all the config objects
template <typename feeder_T>
sdk_ret_t many_read(feeder_T& feeder, sdk_ret_t exp_result = SDK_RET_OK) {
    feeder_T tmp = feeder;

    if (feeder.read_unsupported())
        return SDK_RET_OK;

    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        if (read(tmp) != exp_result) {
            cerr << "MANY READ failed" << endl;
            cerr << "\t" << tmp << endl;
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}

/// \brief Invokes the update apis for all the config objects
template <typename feeder_T>
sdk_ret_t many_update(pds_batch_ctxt_t bctxt, feeder_T& feeder) {
    sdk_ret_t ret;
    feeder_T tmp = feeder;

    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        if ((ret = update(bctxt, tmp)) != SDK_RET_OK) {
            cerr << "MANY UPDATE failed" << endl;
            cerr << "\t" << tmp << endl;
            return ret;
        }
    }
    return ret;
}

/// \brief Invokes the delete apis for all the config objects
template <typename feeder_T>
sdk_ret_t many_delete(pds_batch_ctxt_t bctxt, feeder_T& feeder) {
    sdk_ret_t ret;
    feeder_T tmp = feeder;

    for (tmp.iter_init(); tmp.iter_more(); tmp.iter_next()) {
        if ((ret = del(bctxt, tmp)) != SDK_RET_OK) {
            cerr << "MANY DELETE failed" << endl;
            cerr << tmp << endl;
            return ret;
        }
    }
    return ret;
}

/// @}

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_API_BASE_HPP__
