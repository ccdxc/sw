//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines meter policy APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_METER_HPP__
#define __INCLUDE_API_PDS_METER_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_METER Meter APIs
/// @{

#define PDS_MAX_METER                 64
#define PDS_MAX_PREFIX_PER_METER      1023

/// \brief meter type
typedef enum pds_meter_type_e {
    PDS_METER_TYPE_NONE        = 0,
    PDS_METER_TYPE_PPS_POLICER = 1,    ///< packet per second (pps) policer
    PDS_METER_TYPE_BPS_POLICER = 2,    ///< bytes per second (bps) policer
    PDS_METER_TYPE_ACCOUNTING  = 3,    ///< no policing, just counting
} pds_meter_type_t;

/// \brief meter specific attributes
typedef struct pds_meter_attr_s {
    pds_meter_type_t    type;    ///< meter type
    union {
        // packets per second (pps) policer info
        struct {
            uint32_t    pps;
            uint32_t    pkt_burst;
        };
        // bytes per second (pps) policer info
        struct {
            uint64_t    bps;
            uint64_t    byte_burst;
        };
    };
} __PACK__ pds_meter_attr_t;

/// \brief metering configuration
typedef struct pds_meter_spec_s    pds_meter_spec_t;
struct pds_meter_spec_s {
    pds_meter_key_t     key;             ///< key
    uint8_t             af;              ///< address family - v4 or v6
    pds_meter_attr_t    info;            ///< meter action + data
    uint32_t            num_prefixes;    ///< number of prefixes in the list
    ip_prefix_t         *prefixes;       ///< prefixes using this meter

    // constructor
    pds_meter_spec_s() { prefixes = NULL; }

    // destructor
    ~pds_meter_spec_s() {
        if (prefixes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, prefixes);
        }
    }

    // assignment operator
    pds_meter_spec_t& operator= (const pds_meter_spec_t& spec) {
        // self-assignment guard
        if (this == &spec) {
            return *this;
        }
        key = spec.key;
        af = spec.af;
        info = spec.info;
        num_prefixes = spec.num_prefixes;
        if (prefixes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, prefixes);
        }
        prefixes =
            (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                                      num_prefixes * sizeof(ip_prefix_t));
        memcpy(prefixes, spec.prefixes, num_prefixes * sizeof(ip_prefix_t));
        return *this;
    }
} __PACK__;

/// \brief meter status
typedef struct pds_meter_status_s {
} pds_meter_status_t;

/// \brief meter stats
typedef struct pds_meter_stats_s {
} pds_meter_stats_t;

/// \brief meter information
typedef struct pds_meter_info_s {
    pds_meter_spec_t spec;        ///< Specification
    pds_meter_status_t status;    ///< Status
    pds_meter_stats_t stats;      ///< Statistics
} __PACK__ pds_meter_info_t;

/// \brief create meter
/// \param[in] spec meter configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_meter_create(pds_meter_spec_t *spec);

/// \brief delete meter
/// \param[in] key key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_meter_delete(pds_meter_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_METER_HPP__
