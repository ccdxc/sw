//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Storage virtualization session APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NSV_SESSION_HPP__
#define __INCLUDE_API_PDS_NSV_SESSION_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_types.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_ns.hpp"

/// \defgroup PDS_NSV_SESSION Session API
/// @{

/// \brief Unique Session key
typedef struct pds_nsv_session_key_s {
    pds_nsv_session_id_t id; ///< Session num in the namespace
    pds_nsv_ns_key_t nkey;   ///< Parent Namespace
} __PACK__ pds_nsv_session_key_t;

/// \biref Create a Session key
///
/// \param[in] id Session ID
/// \param[in] nskey Parent Namespace key
/// \param[out] key Session key to be created
static inline void
pds_nsv_session_key_create(pds_nsv_session_id_t id,
                           const pds_nsv_ns_key_t *nskey,
                           pds_nsv_session_key_t *key)
{
    key->id = id;
    key->nkey = *nskey;
}

/// \brief Session Spec
typedef struct pds_nsv_session_spec_s {
    pds_nsv_session_key_t key;   ///< Unique key for this session
} __PACK__ pds_nsv_session_spec_t;

/// \brief Session Status
typedef struct pds_nsv_session_status_s {
    // TBD
} __PACK__ pds_nsv_session_status_t;

/// \brief Session Stats
typedef struct pds_nsv_session_stats_s {
    // TBD
} __PACK__ pds_nsv_session_stats_t;

/// \brief Session Info
typedef struct pds_nsv_session_info_s {
    pds_nsv_session_spec_t spec;     ///< Specification
    pds_nsv_session_status_t status; ///< Status
    pds_nsv_session_stats_t stats;   ///< Statistics
} __PACK__ pds_nsv_session_info_t;

/// \brief Create Session
///
/// \param[in] spec Session Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_session_create(pds_nsv_session_spec_t *spec);

/// \brief Read Session
///
/// \param[in] key Session key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_session_read(pds_nsv_session_key_t *key, pds_nsv_session_info_t *info);

/// \brief Update Session
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_session_update(pds_nsv_session_spec_t *spec);

/// \brief Delete Session
///
/// \param[in] key Session key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_session_delete(pds_nsv_session_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_NSV_SESSION_HPP__
