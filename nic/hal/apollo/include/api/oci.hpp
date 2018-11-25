/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci.hpp
 *
 * @brief   This module defines an entry point into OCI API
 */

#if !defined (__OCI_H_)
#define __OCI_H_

/**
 * @defgroup OCI - Entry point specific API definitions.
 *
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/**
 * @brief API status codes
 */

typedef enum oci_status_e {
    OCI_STAUS_OK,
    OCI_STATUS_EXISTS,
    OCI_STATUS_NOT_FOUND,
    OCI_STATUS_OOM,
    OCI_STATUS_INVALID_OPERATION,
    OCI_STATUS_HW_PROGRAMMING_ERR,
    OCI_STAUS_ERR,
} oci_status_t;

/**
 * @brief Defined API sets have assigned IDs.
 */
typedef enum oci_class_id_e {
    OCI_CLASS_ID_UNSPECIFIED      =  0,    /**< unspecified */
    OCI_CLASS_ID_VCN              =  1,    /**< vnc */
    OCI_CLASS_ID_SUBNET           =  2,    /**< subnet */
    OCI_CLASS_ID_TEP              =  3,    /**< tunnel */
    OCI_CLASS_ID_VNIC             =  4,    /**< vnic */
    OCI_CLASS_ID_ROUTE            =  5,    /**< route */
    OCI_CLASS_ID_SECRULE          =  6,    /**< security rule */
    OCI_CLASS_SWITCHPORT          =  7,    /**< switchport params */
    OCI_CLASS_ID_MAX              =  8,    /**< total number of class-id */
} oci_class_id_t;

// TODO:
// all log related APIs should be wrapped in oci_init() API
/**
 * @brief Defines log level
 */
typedef enum oci_log_level_e {
    OCI_LOG_LEVEL_DEBUG            = 0,    /**< Log level debug */
    OCI_LOG_LEVEL_INFO             = 1,    /**< Log level info */
    OCI_LOG_LEVEL_NOTICE           = 2,    /**< Log level notice */
    OCI_LOG_LEVEL_WARN             = 3,    /**< Log level warn */
    OCI_LOG_LEVEL_ERROR            = 4,    /**< Log level error */
    OCI_LOG_LEVEL_CRITICAL         = 5     /**< Log level critical */
} oci_log_level_t;

/**
 * @brief Set log level for OCI API module
 *
 * The default log level is #OCI_LOG_LEVEL_WARN.
 *
 * @param[in] oci_api_id OCI CLASS ID
 * @param[in] log_level Log level
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_log_set(_In_ oci_class_id_t oci_class_id,
                         _In_ oci_log_level_t log_level);

typedef uint64_t  oci_vcn_id_t;
typedef uint64_t  oci_subnet_id_t;
typedef uint64_t  oci_vnic_id_t;
typedef uint64_t  oci_rule_id_t;
typedef uint64_t  oci_rsrc_pool_id_t;

/**
 * @}
 */
#endif /** __OCI_H_ */
