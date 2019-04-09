//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Storage virtualization target APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NSV_TGT_HPP__
#define __INCLUDE_API_PDS_NSV_TGT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_types.hpp"
#include "nic/apollo/api/include/nsv/pds_nsv_session.hpp"

/// \defgroup PDS_NSV_TGT Target API
/// @{

/// \brief Target key
typedef struct pds_nsv_tgt_key_s {
    pds_nsv_tgt_id_t  id;       ///< Target ID in the session
    pds_nsv_session_key_t skey; ///< Parent Session key
} __PACK__ pds_nsv_tgt_key_t;

/// \brief create a tgt key
///
/// \param[in] id tgt_id within the session
/// \param[in] skey Parent session key pointer
/// \param[out] key Tgt key to be created
static inline void
pds_nsv_tgt_key_create(pds_nsv_tgt_id_t id, const pds_nsv_session_key_t *skey,
                       pds_nsv_tgt_key_t *key)
{
    key->id   = id;
    key->skey = *skey;
}

/// \brief Target Spec
typedef struct pds_nsv_tgt_spec_s {
    pds_nsv_tgt_key_t key; ///< Unique tgt key
    ip_addr_t  tgt_addr;   ///< Target IP address
    uint16_t   tgt_port;   ///< Target Port, 0: Default NVME-TCP port is used
} __PACK__ pds_nsv_tgt_spec_t;

/// \brief Target Status
typedef struct pds_nsv_tgt_status_s {
    // TBD
} __PACK__ pds_nsv_tgt_status_t;

/// \brief Target Stats
typedef struct pds_nsv_tgt_stats_s {
    // TBD
} __PACK__ pds_nsv_tgt_stats_t;

/// \brief Target Info
typedef struct pds_nsv_tgt_info_s {
    pds_nsv_tgt_spec_t spec;     ///< Specification
    pds_nsv_tgt_status_t status; ///< Status
    pds_nsv_tgt_stats_t stats;   ///< Statistics
} __PACK__ pds_nsv_tgt_info_t;

/// \brief Create Target
///
/// \param[in] spec Target Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_tgt_create(pds_nsv_tgt_spec_t *spec);

/// \brief Read Target
///
/// \param[in] key Target key
/// \param[out] info Information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_tgt_read(pds_nsv_tgt_key_t *key, pds_nsv_tgt_info_t *info);

/// \brief Update Target
///
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_tgt_update(pds_nsv_tgt_spec_t *spec);

/// \brief Delete Target
///
/// \param[in] key Target key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_tgt_delete(pds_nsv_tgt_key_t *key);

/// @}

#endif    // __INCLUDE_API_PDS_NSV_TGT_HPP__
