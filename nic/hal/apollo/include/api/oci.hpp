/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci.hpp
 *
 * @brief   basic enums and macros for OCI APIs
 */

#if !defined (__OCI_HPP__)
#define __OCI_HPP__

/**
 * @defgroup OCI - base macros, enums etc. for OCI APIs
 *
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/**
 * @brief    API status codes
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
 * @brief    invalid epoch
 */
#define OCI_EPOCH_INVALID        0x0

/**
 * @brief    basic OCI data types
 */
typedef uint32_t  oci_vcn_id_t;
typedef uint32_t  oci_subnet_id_t;
typedef uint16_t  oci_vnic_id_t;
typedef uint32_t  oci_rule_id_t;
typedef uint32_t  oci_rsc_pool_id_t;
typedef uint32_t  oci_epoch_t;
typedef uint32_t  oci_slot_id_t;

/**
 * @}
 */

#endif    /** __OCI_HPP__ */
