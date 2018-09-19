/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci.h
 *
 * @brief   This module defines an entry point into OCI API
 */

#if !defined (__OCI_H_)
#define __OCI_H_

#include "vcn.h"

/**
 * @defgroup OCI - Entry point specific API definitions.
 *
 * @{
 */

/**
 * @brief Defined API sets have assigned IDs.
 */
typedef enum _oci_api_t
{
    OCI_API_UNSPECIFIED      =  0, /**< unspecified API */
    OCI_API_VCN              =  1, /**< oci_vcn_api_t */
    OCI_API_MAX              =  2, /**< total number of APIs */
} oci_api_t;

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

typedef const char* (*oci_profile_get_value_fn)(
        _In_ oci_switch_profile_id_t profile_id,
        _In_ const char *variable);

typedef int (*oci_profile_get_next_value_fn)(
        _In_ oci_switch_profile_id_t profile_id,
        _Out_ const char **variable,
        _Out_ const char **value);

/**
 * @brief Method table that contains function pointers for services exposed by
 * adapter host for adapter.
 */
typedef struct _oci_service_method_table_t
{
    /**
     * @brief Get variable value given its name
     */
    oci_profile_get_value_fn        profile_get_value;

    /**
     * @brief Enumerate all the K/V pairs in a profile.
     *
     * Pointer to NULL passed as variable restarts enumeration. Function
     * returns 0 if next value exists, -1 at the end of the list.
     */
    oci_profile_get_next_value_fn   profile_get_next_value;

} oci_service_method_table_t;

/**
 * @brief Adapter module initialization call
 *
 * This is NOT for SDK initialization.
 *
 * @param[in] flags Reserved for future use, must be zero
 * @param[in] services Methods table with services provided by adapter host
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_api_initialize(
        _In_ uint64_t flags,
        _In_ const oci_service_method_table_t *services);

/**
 * @brief Retrieve a pointer to the C-style method table for desired OCI
 * functionality as specified by the given oci_api_id.
 *
 * @param[in] oci_api_id OCI API ID
 * @param[out] api_method_table Caller allocated method table. The table must
 * remain valid until the oci_api_uninitialize() is called.
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_api_query(
        _In_ oci_api_t oci_api_id,
        _Out_ void **api_method_table);

/**
 * @brief Uninitialize adapter module. OCI functionalities,
 * retrieved via oci_api_query() cannot be used after this call.
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_api_uninitialize(void);

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
        _In_ oci_api_t oci_api_id,
        _In_ oci_log_level_t log_level);

/**
 * @brief Query OCI object type.
 *
 * @param[in] oci_object_id Object id
 *
 * @return #OCI_OBJECT_TYPE_NULL when oci_object_id is not valid.
 * Otherwise, return a valid OCI object type OCI_OBJECT_TYPE_XXX.
 */
oci_object_type_t oci_object_type_query(
        _In_ oci_object_id_t oci_object_id);

/**
 * @brief Query OCI switch id.
 *
 * @param[in] oci_object_id Object id
 *
 * @return #OCI_NULL_OBJECT_ID when oci_object_id is not valid.
 * Otherwise, return a valid OCI_OBJECT_TYPE_SWITCH object on which
 * provided object id belongs. If valid switch id object is provided
 * as input parameter it should return itself.
 */
oci_object_id_t oci_switch_id_query(
        _In_ oci_object_id_t oci_object_id);

/**
 * @brief Generate dump file. The dump file may include OCI state information and vendor SDK information.
 *
 * @param[in] dump_file_name Full path for dump file
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_dbg_generate_dump(
        _In_ const char *dump_file_name);

/**
 * @}
 */
#endif /** __OCI_H_ */
