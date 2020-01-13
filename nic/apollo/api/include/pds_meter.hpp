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

#define PDS_MAX_METER               64       ///< Maximum meters
#define PDS_MAX_PREFIX_PER_METER    1023     ///< Maximum prefixes per meter

/// \brief meter type
typedef enum pds_meter_type_e {
    PDS_METER_TYPE_NONE        = 0,
    PDS_METER_TYPE_PPS_POLICER = 1,    ///< packet per second (pps) policer
    PDS_METER_TYPE_BPS_POLICER = 2,    ///< bytes per second (bps) policer
    PDS_METER_TYPE_ACCOUNTING  = 3,    ///< no policing, just counting
} pds_meter_type_t;

/// \brief meter rule
typedef struct pds_meter_rule_s {
    pds_meter_type_t type;            ///< meter type
    union {
        // packets per second (pps) policer info
        struct {
            uint32_t pps;
            uint32_t pkt_burst;
        };
        // bytes per second (pps) policer info
        struct {
            uint64_t bps;
            uint64_t byte_burst;
        };
    };
    uint32_t         priority;        ///< priority of the rule (numerically
                                      ///< lower values is higher priority)
    uint32_t         num_prefixes;    ///< number of prefixes in the list
    ip_prefix_t      *prefixes;       ///< prefixes using this meter
} __PACK__ pds_meter_rule_t;

/// \brief metering configuration
typedef struct pds_meter_spec_s    pds_meter_spec_t;
/// \brief metering configuration
struct pds_meter_spec_s {
    pds_obj_key_t  key;          ///< key
    uint8_t          af;           ///< address family - v4 or v6
    uint32_t         num_rules;    ///< number of metering rules in this policy
    pds_meter_rule_t *rules;       ///< metering rules

    // constructor
    pds_meter_spec_s() { rules = NULL; }

    // destructor
    ~pds_meter_spec_s() {
        if (rules) {
            for (uint32_t i = 0; i < num_rules; i++) {
                SDK_FREE(PDS_MEM_ALLOC_ID_METER, rules[i].prefixes);
            }
        }
        SDK_FREE(PDS_MEM_ALLOC_ID_METER, rules);
    }

    /// assignment operator
    pds_meter_spec_t& operator= (const pds_meter_spec_t& spec) {
        // self-assignment guard
        if (this == &spec) {
            return *this;
        }
        key = spec.key;
        af = spec.af;
        if (rules) {
            for (uint32_t i = 0; i < num_rules; i++) {
                SDK_FREE(PDS_MEM_ALLOC_ID_METER, rules[i].prefixes);
            }
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, rules);
        }
        num_rules = spec.num_rules;
        rules = (pds_meter_rule_t *)
                    SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                               num_rules * sizeof(pds_meter_rule_t));
        for (uint32_t i = 0; i < num_rules; i++) {
            rules[i].type = spec.rules[i].type;
            rules[i].priority = spec.rules[i].priority;
            if (rules[i].type == PDS_METER_TYPE_PPS_POLICER) {
                rules[i].pps = spec.rules[i].pps;
                rules[i].pkt_burst = spec.rules[i].pkt_burst;
            } else if (rules[i].type == PDS_METER_TYPE_BPS_POLICER) {
                rules[i].bps = spec.rules[i].bps;
                rules[i].byte_burst = spec.rules[i].byte_burst;
            }
            rules[i].num_prefixes = spec.rules[i].num_prefixes;
            rules[i].prefixes =
                (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                                          rules[i].num_prefixes * sizeof(ip_prefix_t));
            memcpy(rules[i].prefixes, spec.rules[i].prefixes,
                   rules[i].num_prefixes * sizeof(ip_prefix_t));
        }
        return *this;
    }
} __PACK__;

/// \brief meter status
typedef struct pds_meter_status_s {
    // TODO : only base address of the tree stored in HBM is read
} pds_meter_status_t;

/// \brief meter stats
typedef struct pds_meter_stats_s {
    uint32_t idx;         ///< meter index
    uint64_t rx_bytes;    ///< received bytes
    uint64_t tx_bytes;    ///< transmitted bytes
} pds_meter_stats_t;

/// \brief meter information
typedef struct pds_meter_info_s {
    pds_meter_spec_t spec;        ///< specification
    pds_meter_status_t status;    ///< status
    pds_meter_stats_t stats;      ///< statistics
} __PACK__ pds_meter_info_t;

/// \brief     create meter
/// \param[in] spec meter configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_meter_create(pds_meter_spec_t *spec,
                           pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read meter
/// \param[in]  key key
/// \param[out] info meter information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_meter_read(pds_obj_key_t *key, pds_meter_info_t *info);

/// \brief     update meter
/// \param[in] spec meter configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_meter_update(pds_meter_spec_t *spec,
                           pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete meter
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_meter_delete(pds_obj_key_t *key,
                           pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_METER_HPP__
