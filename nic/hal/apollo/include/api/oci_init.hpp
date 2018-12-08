/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_init.hpp
 *
 * @brief   This module defines OCI init and teardown APIs
 */

#if !defined (__OCI_INIT_HPP__)
#define __OCI_INIT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

using sdk_logger = sdk::lib::logger;

/**
 * @defgroup OCI_INIT - initialization and teardown APIs
 *
 * @{
 */

/**
 * @brief    OCI library initialization modes
 */
typedef enum oci_init_mode_e {
    OCI_INIT_MODE_NONE,                 /**< invalid mode */
    OCI_INIT_MODE_FRESH_START,          /**< initialize from scratch, ignore
                                          any state if preserved previously */
    OCI_INIT_MODE_POST_UPGRADE,         /**< initialize using state preserved,
                                              if any */
} oci_init_mode_t;

/**
 * @brief    OCI library initialization time parameters
 */
typedef struct oci_init_params_s {
    oci_init_mode_t            init_mode;         /**< mode of initialization */
    sdk_logger::logger_cb_t    debug_trace_cb;    /**< callback for debug traces */
    sdk_logger::logger_cb_t    error_trace_cb;    /**< callback for error traces */
} oci_init_params_t;

/**
 * @brief        initialize OCI HAL
 * @param[in]    params init time parameters
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_init(oci_init_params_t *params);

/**
 * @brief    teardown OCI HAL
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_teardown(void);

/**
 * @}
 */
#endif    /** __OCI_INIT_HPP__ */
