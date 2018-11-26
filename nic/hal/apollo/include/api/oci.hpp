/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci.hpp
 *
 * @brief   This module defines an entry point into OCI API
 */

#if !defined (__OCI_HPP__)
#define __OCI_HPP__

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
// TODO: what is the use of this ?
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

/**
 * @brief Defines trace levels
 */
typedef enum oci_trace_level_e {
    OCI_TRACE_LEVEL_DEBUG            = 0,    /**< trace level debug */
    OCI_TRACE_LEVEL_INFO             = 1,    /**< trace level info */
    OCI_TRACE_LEVEL_NOTICE           = 2,    /**< trace level notice */
    OCI_TRACE_LEVEL_WARN             = 3,    /**< trace level warn */
    OCI_TRACE_LEVEL_ERROR            = 4,    /**< trace level error */
    OCI_TRACE_LEVEL_CRITICAL         = 5     /**< trace level critical */
} oci_trace_level_t;

/**
 * @brief    invalid epoch
 */
#define OCI_EPOCH_INVALID        0x0

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
oci_status_t oci_init(oci_init_params_t *params);

typedef uint32_t  oci_vcn_id_t;
typedef uint32_t  oci_subnet_id_t;
typedef uint16_t  oci_vnic_id_t;
typedef uint32_t  oci_rule_id_t;
typedef uint32_t  oci_rsrc_pool_id_t;
typedef uint32_t  oci_epoch_t;

/**
 * @}
 */
#endif    /** __OCI_HPP__ */
