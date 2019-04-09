//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Storage virtualization controller APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NSV_CNTRLR_HPP__
#define __INCLUDE_API_PDS_NSV_CNTRLR_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_types.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_subsys.hpp"

/// \defgroup PDS_NSV_CNTRLR Controller API
/// @{

/// \brief Unique Controller Key
typedef struct pds_nsv_cntrlr_key_s {
    pds_nsv_cntrlr_id_t  id;   ///< Controller in the subsystem
    pds_nsv_subsys_key_t skey; ///< Parent susbsystem
} __PACK__ pds_nsv_cntrlr_key_t;

/// \brief Create controller Key
/// \param[in] id controller ID within the subsys
/// \param[in] skey Parent Subsys Key
/// \param[out] key controller key to be created
static inline void
pds_nsv_cntrlr_key_create(pds_nsv_cntrlr_id_t id,
                          const pds_nsv_subsys_key_t *skey,
                          ppds_nsv_cntrlr_key_t *key)
{
    key->id   = id;
    key->skey = *skey;
}

/// \brief Controller Spec
typedef struct pds_nsv_cntrlr_spec_s {
    pds_nsv_cntrlr_key_t key;   ///< Unique Key for this cntrlrtem
} __PACK__ pds_nsv_cntrlr_spec_t;

/// \brief Controller Status
typedef struct pds_nsv_cntrlr_status_s {
    // TBD
} __PACK__ pds_nsv_cntrlr_status_t;

/// \brief Controller Stats
typedef struct pds_nsv_cntrlr_stats_s {
    // TBD
} __PACK__ pds_nsv_cntrlr_stats_t;

/// \brief Controller Info
typedef struct pds_nsv_cntrlr_info_s {
    pds_nsv_cntrlr_spec_t spec;     ///< Specification
    pds_nsv_cntrlr_status_t status; ///< Status
    pds_nsv_cntrlr_stats_t stats;   ///< Statistics
} __PACK__ pds_nsv_cntrlr_info_t;

/// \brief Create Controller
///
/// \param[in] spec Controller Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_cntrlr_create(pds_nsv_cntrlr_spec_t *spec);

/// \brief Read Controller
///
/// \param[in] key Controller Key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_cntrlr_read(pds_nsv_cntrlr_key_t *key, pds_nsv_cntrlr_info_t *info);

/// \brief Update Controller
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_cntrlr_update(pds_nsv_cntrlr_spec_t *spec);

/// \brief Delete Controller
///
/// \param[in] key Controller Key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_cntrlr_delete(pds_nsv_cntrlr_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_NSV_CNTRLR_HPP__
