/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci.h
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

/**
 * @brief Defined API sets have assigned IDs.
 */
typedef enum _oci_class_id_t
{
    OCI_CLASS_ID_UNSPECIFIED      =  0, /**< unspecified */
    OCI_CLASS_ID_VCN              =  1, /**< vnc */
    OCI_CLASS_ID_MAX              =  2, /**< total number of class-id */
} oci_class_id_t;

/**
 * @brief Defines log level
 */
typedef enum _oci_log_level_t
{
    /** Log Level Debug */
    OCI_LOG_LEVEL_DEBUG            = 0,

    /** Log Level Info */
    OCI_LOG_LEVEL_INFO             = 1,

    /** Log Level Notice */
    OCI_LOG_LEVEL_NOTICE           = 2,

    /** Log level Warning */
    OCI_LOG_LEVEL_WARN             = 3,

    /** Log Level Error */
    OCI_LOG_LEVEL_ERROR            = 4,

    /** Log Level Critical */
    OCI_LOG_LEVEL_CRITICAL         = 5

} oci_log_level_t;

/**
 * @brief Set log level for OCI API module
 *
 * The default log level is #OCI_LOG_LEVEL_WARN.
 *
 * @param[in] oci_api_id OCI API ID
 * @param[in] log_level Log level
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_log_set(
        _In_ oci_class_id_t oci_class_id,
        _In_ oci_log_level_t log_level);

/**
 * @}
 */
#endif /** __OCI_H_ */
