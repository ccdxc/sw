//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines mirror specific APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_MIRROR_HPP__
#define __INCLUDE_API_PDS_MIRROR_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_MIRROR    mirrir session APIs
/// @{

/// \brief    mirror session key
typedef struct pds_mirror_session_key_s {
    pds_mirror_session_id_t id;    ///< unique mirror session id
} __PACK__ pds_mirror_session_key_t;

///< \brief    RSPAN configuration
typedef struct pds_rspan_spec_s {
} __PACK__ pds_rspan_spec_t;

///< \brief    ERSPAN configuration
typedef struct pds_erspan_spec_s {
} __PACK__ pds_erspan_spec_t;

///< \brief    mirror session configuration
typedef struct pds_mirror_session_spec_s {
    pds_mirror_session_key_t key;
    uint32_t snap_len;
    union {
        pds_rspan_spec_t rspan_spec;
        pds_erspan_spec_t erspan_spec;
    };
} __PACK__ pds_mirror_session_spec_t;

///< \brief    mirror session operational status
typedef struct pds_mirror_session_status_s {
} __PACK__ pds_mirror_session_status_t;

///< \brief    mirror session statistics, if any
typedef struct pds_mirror_session_stats_s {
} __PACK__ pds_mirror_session_stats_t;

/// \brief    create mirror session
/// \param[in] spec    mirror session configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_create(pds_mirror_session_spec_t *spec);

/// \brief    delete mirror session
/// \param[in] key    mirror session key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_delete(pds_mirror_session_key_t *key);

/// @}

#endif    ///  __INCLUDE_API_PDS_MIRROR_HPP__
