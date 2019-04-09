//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Storage virtualization namespace APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NSV_NS_HPP__
#define __INCLUDE_API_PDS_NSV_NS_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_types.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_cntrlr.hpp"

/// \defgroup PDS_NSV_NAMESPACE Namespace API
/// @{

/// \brief Namespace key
typedef struct pds_nsv_ns_key_s {
    pds_nsv_nsid_t id; ///< namespace ID local to the controller
    pds_nsv_cntrlr_key_t ckey; ///< Parent controller key
} __PACK__ pds_nsv_ns_key_t;

/// \brief Namespace key Create
///
/// \param[in] id Namespace ID
/// \param[in] ckey Parent controller key
/// \param[out] key Namespace key to be created
static inline void
pds_nsv_ns_key_create(pds_nsv_nsid_t id, const pds_nsv_cntrlr_key_t *ckey,
                      pds_nsv_ns_key_t *key)
{
    key->id   = id;
    key->ckey = *ckey;
}

/// \brief Namespace Spec
typedef struct pds_nsv_ns_spec_s {
    pds_nsv_ns_key_t key;         ///< Unique Namespace key
    uint32_t block_sz;            ///< Block size in bytes
    bool is_boot;                 ///< True: Boot volume
    bool is_encrypted;            ///< True: Encryption enabled
    pds_nsv_nsid_t  remote_nsid;  ///< corresponding backend namespace ID
    pds_nvs_nsze_t capacity;      ///< NS size in blocks
    pds_nsv_crypto_key_t enc_key; ///< Encryption key
    pds_nsv_nqn_t ns_nqn;         ///< NQN used to identify namespace
} __PACK__ pds_nsv_ns_spec_t;

/// \brief Namespace Status
typedef struct pds_nsv_ns_status_s {
    // TBD
}__PACK__ pds_nsv_ns_status_t;

/// \brief Namespace Stats
typedef struct pds_nsv_ns_stats_s {
    // TBD
} __PACK__ pds_nsv_ns_stats_t;

/// \brief Namespace Info
typedef struct pds_nsv_ns_info_s {
    pds_nsv_ns_spec_t spec;     ///< Specification
    pds_nsv_ns_status_t status; ///< Status
    pds_nsv_ns_stats_t stats;   ///< Statistics
} __PACK__ pds_nsv_ns_info_t;

/// \brief Create Namespace
///
/// \param[in] spec Namespace Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_ns_create(pds_nsv_ns_spec_t *spec);

/// \brief Read Namespace
///
/// \param[in] key Namespace key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_ns_read(pds_nsv_ns_key_t *key, pds_nsv_ns_info_t *info);

/// \brief Update Namespace
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_ns_update(pds_nsv_ns_spec_t *spec);

/// \brief Delete Namespace
///
/// \param key[in] Namespace key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_ns_delete(pds_nsv_ns_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_NSV_NS_HPP__
