//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines (service) tag APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_TAG_HPP__
#define __INCLUDE_API_PDS_TAG_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_TAG Service Tag APIs
/// @{

#define PDS_MAX_TAG                 64       ///< Maximum tags
#define PDS_MAX_PREFIX_PER_TAG      16383    ///< Maximum prefix per tag

/// \brief tag rule
typedef struct pds_tag_rule_s {
    uint32_t       tag;             ///< tag value
    uint32_t       priority;        ///< priority of the rule (numerically
                                    ///< lower values is higher priority)
    uint32_t       num_prefixes;    ///< number of prefixes in this tag
    ip_prefix_t    *prefixes;       ///< prefixes of this tag rule
} __PACK__ pds_tag_rule_t;

/// \brief tag configuration
typedef struct pds_tag_spec_s    pds_tag_spec_t;
/// \brief tag configuration
struct pds_tag_spec_s {
    pds_obj_key_t     key;             ///< key
    uint8_t           af;              ///< address family - v4 or v6
    uint32_t          num_rules;       ///< number of tag tag rules
    pds_tag_rule_t    *rules;          ///< metering rules

    // constructor
    pds_tag_spec_s() { rules = NULL; }

    // destructor
    ~pds_tag_spec_s() {
        if (rules) {
            for (uint32_t i = 0; i < num_rules; i++) {
                SDK_FREE(PDS_MEM_ALLOC_ID_TAG, rules[i].prefixes);
            }
            SDK_FREE(PDS_MEM_ALLOC_ID_TAG, rules);
        }
    }

    /// assignment operator
    pds_tag_spec_t& operator= (const pds_tag_spec_t& spec) {
        // self-assignment guard
        if (this == &spec) {
            return *this;
        }
        key = spec.key;
        af = spec.af;
        if (rules) {
            for (uint32_t i = 0; i < num_rules; i++) {
                SDK_FREE(PDS_MEM_ALLOC_ID_TAG, rules[i].prefixes);
            }
            SDK_FREE(PDS_MEM_ALLOC_ID_TAG, rules);
        }
        num_rules = spec.num_rules;
        rules = (pds_tag_rule_t *)
                    SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                               num_rules * sizeof(pds_tag_rule_t));
        for (uint32_t i = 0; i < num_rules; i++) {
            rules[i].tag = spec.rules[i].tag;
            rules[i].priority = spec.rules[i].priority;
            rules[i].num_prefixes = spec.rules[i].num_prefixes;
            rules[i].prefixes =
                (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                                          rules[i].num_prefixes * sizeof(ip_prefix_t));
            memcpy(rules[i].prefixes, spec.rules[i].prefixes,
                   rules[i].num_prefixes * sizeof(ip_prefix_t));
        }
        return *this;
    }
} __PACK__;

/// \brief tag status
typedef struct pds_tag_status_s {
    // TODO : only base address of the tree stored in HBM is read
} pds_tag_status_t;

/// \brief tag statistics
typedef struct pds_tag_stats_s {
} pds_tag_stats_t;

/// \brief tag information
typedef struct pds_tag_info_s {
    pds_tag_spec_t spec;        ///< Specification
    pds_tag_status_t status;    ///< Status
    pds_tag_stats_t stats;      ///< Statistics
} __PACK__ pds_tag_info_t;

/// \brief create tag
/// \param[in] spec tag configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_create(pds_tag_spec_t *spec,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief read tag
/// \param[in] key key
/// \param[out] info tag information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_read(pds_obj_key_t *key, pds_tag_info_t *info);

typedef void (*tag_read_cb_t)(pds_tag_info_t *info, void *ctxt);

/// \brief      read all tag
/// \param[in]  cb      callback function
/// \param[in]  ctxt    opaque context passed to cb
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_read_all(tag_read_cb_t tag_read_cb, void *ctxt);

/// \brief update tag
/// \param[in] spec tag configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_update(pds_tag_spec_t *spec,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete tag
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_delete(pds_obj_key_t *key,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_TAG_HPP__
