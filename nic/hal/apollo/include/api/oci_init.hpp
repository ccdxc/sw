/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_init.hpp
 *
 * @brief   This module defines OCI init and teardown APIs
 */

#if !defined (__OCI_INIT_HPP__)
#define __OCI_INIT_HPP__

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
    oci_init_mode_t      init_mode;
    oci_trace_level_t    log_level;
} oci_init_params_t;

/**< @brief    initialize OCI HAL */
oci_status_t oci_init(oci_init_params_t *params);

/**< @brief    teardown OCI HAL */
oci_status_t oci_teardown(void);

/**
 * @}
 */
#endif    /** __OCI_INIT_HPP__ */
