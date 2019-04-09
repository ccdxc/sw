//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Storage virtualization subsys APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NSV_SUBSYS_HPP__
#define __INCLUDE_API_PDS_NSV_SUBSYS_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_types.hpp"

/// \defgroup PDS_NSV_SUBSYS Subsystem API
/// @{

/// \brief Unique Subsystem key
typedef struct pds_nsv_subsys_key_s {
    pds_nsv_subsys_id_t id; ///< Subsystem ID
} __PACK__ pds_nsv_subsys_key_t;

/// \biref Create a Subsystem key
///
/// \param[in] id Subsystem ID
/// \param[out] key Subsystem key to be created
static inline void
pds_nsv_subsys_key_create(pds_nsv_subsys_id_t id, pds_nsv_subsys_key_t *key);
{
    key->id = id;
}

/// \brief Subsystem Spec
typedef struct pds_nsv_subsys_spec_s {
    pds_nsv_subsys_key_t key;  ///< Unique key for this subsystem
    pds_nsv_nqn_t subsys_nqn;  ///< Unique NQN identifer for this subsystem
    pds_nsv_hostid_t hostid;   ///< Host ID to use for communication with backend Targets
} __PACK__ pds_nsv_subsys_spec_t;

/// \brief Subsystem Status
typedef struct pds_nsv_subsys_status_s {
    // TBD
} __PACK__ pds_nsv_subsys_status_t;

/// \brief Subsystem Stats
typedef struct pds_nsv_subsys_stats_s {
    // TBD
} __PACK__ pds_nsv_subsys_stats_t;

/// \brief Subsystem Info
typedef struct pds_nsv_subsys_info_s {
    pds_nsv_subsys_spec_t spec;     ///< Specification
    pds_nsv_subsys_status_t status; ///< Status
    pds_nsv_subsys_stats_t stats;   ///< Statistics
} __PACK__ pds_nsv_subsys_info_t;

/// \brief Create Subsystem
///
/// \param[in] spec Subsystem Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_subsys_create(pds_nsv_subsys_spec_t *spec);

/// \brief Read Subsystem
///
/// \param[in] key Subsystem key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_subsys_read(pds_nsv_subsys_key_t *key, pds_nsv_subsys_info_t *info);

/// \brief Update Subsystem
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
///
sdk_ret_t pds_nsv_subsys_update(pds_nsv_subsys_spec_t *spec);

/// \brief Delete Subsystem
///
/// \param[in] key Subsystem key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_subsys_delete(pds_nsv_subsys_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_NSV_SUBSYS_HPP__
