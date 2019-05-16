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

/// \defgroup PDS_TAG (Service) Tag APIs
/// @{

#define PDS_MAX_TAG                 64
#define PDS_MAX_PREFIX_PER_TAG      1023

/// \brief tag configuration
typedef struct pds_tag_spec_s    pds_tag_spec_t;
struct pds_tag_spec_s {
    pds_tag_key_t     key;             ///< key
    uint8_t           af;              ///< address family - v4 or v6
    uint32_t          num_prefixes;    ///< number of prefixes in the list
    ip_prefix_t       *prefixes;       ///< prefixes using this tag

    // constructor
    pds_tag_spec_s() { prefixes = NULL; }

    // destructor
    ~pds_tag_spec_s() {
        if (prefixes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_TAG, prefixes);
        }
    }

    // assignment operator
    pds_tag_spec_t& operator= (const pds_tag_spec_t& spec) {
        // self-assignment guard
        if (this == &spec) {
            return *this;
        }
        key = spec.key;
        af = spec.af;
        num_prefixes = spec.num_prefixes;
        if (prefixes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_TAG, prefixes);
        }
        prefixes =
            (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                                      num_prefixes * sizeof(ip_prefix_t));
        memcpy(prefixes, spec.prefixes, num_prefixes * sizeof(ip_prefix_t));
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
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_create(pds_tag_spec_t *spec);

/// \brief update tag
/// \param[in] spec tag configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_update(pds_tag_spec_t *spec);

/// \brief delete tag
/// \param[in] key key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_delete(pds_tag_key_t *key);

/// \brief read meter
/// \param[in] key key
/// \param[out] info meter information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_tag_read(pds_tag_key_t *key, pds_tag_info_t *info);

/// \@}

#endif    // __INCLUDE_API_PDS_TAG_HPP__
