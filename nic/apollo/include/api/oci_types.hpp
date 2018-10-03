/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_types.h
 *
 * @brief   This module defines OCI portable types
 */

#if !defined (__OCI_TYPES_H_)
#define __OCI_TYPES_H_

/**
 * @defgroup OCI_TYPES - Types definitions
 *
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef uint64_t  oci_vcn_id_t;
typedef uint64_t  oci_subnet_id_t;
typedef uint64_t  oci_vnic_id_t;
typedef uint64_t  oci_rule_id_t;
typedef uint64_t  oci_rsrc_pool_id_t;

/**
 * @}
 */
#endif /** __OCI_TYPES_H_ */
