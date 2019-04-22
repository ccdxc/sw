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
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vcn.hpp"

/// \defgroup PDS_MIRROR    mirrir session APIs
/// @{

#define PDS_MAX_MIRROR_SESSION    8

/// \brief    mirror session key
typedef struct pds_mirror_session_key_s {
    ///< unique mirror session id in the range [1-8]
    pds_mirror_session_id_t id;
} __PACK__ pds_mirror_session_key_t;

///< \brief    RSPAN configuration
typedef struct pds_rspan_spec_s {
  pds_ifindex_t interface;    ///< outgoing interface
  pds_encap_t encap;          ///< encap details
} __PACK__ pds_rspan_spec_t;

///< \brief    ERSPAN configuration
typedef struct pds_erspan_spec_s {
    pds_vcn_key_t vcn;    ///< vcn of the destination IP
    ip_addr_t dst_ip;     ///< IP address of ERSPAN destination
    ip_addr_t src_ip;     ///< IP address of ERSPAN source
    uint32_t dscp;        ///< DSCP value to use in the packet
    uint32_t span_id;     ///< SPAN ID to use in packet / ERSPAN Header
} __PACK__ pds_erspan_spec_t;

/// \brief    mirror session type
typedef enum pds_mirror_session_type_e {
    PDS_MIRROR_SESSION_TYPE_RSPAN  = 0,    ///< RSPAN session type
    PDS_MIRROR_SESSION_TYPE_ERSPAN = 1,    ///< ERSPAN session type
    PDS_MIRROR_SESSION_TYPE_MAX    = PDS_MIRROR_SESSION_TYPE_ERSPAN,
} pds_mirror_session_type_t;

///< \brief    mirror session configuration
typedef struct pds_mirror_session_spec_s {
    pds_mirror_session_key_t key;         ///< key of the mirror session
    pds_mirror_session_type_t type;       ///< mirror session type
    uint16_t snap_len;                    ///< max len. of pkt mirrored
    union {
        pds_rspan_spec_t rspan_spec;      ///< RSPAN configuration
        pds_erspan_spec_t erspan_spec;    ///< ERSPAN configuration
    };
} __PACK__ pds_mirror_session_spec_t;

///< \brief    mirror session operational status
typedef struct pds_mirror_session_status_s {
} __PACK__ pds_mirror_session_status_t;

///< \brief    mirror session statistics, if any
typedef struct pds_mirror_session_stats_s {
} __PACK__ pds_mirror_session_stats_t;

/// \brief mirror session info
typedef struct pds_mirror_session_info_s {
    pds_mirror_session_spec_t spec;        ///< specification
    pds_mirror_session_status_t status;    ///< status
    pds_mirror_session_stats_t stats;      ///< statistics
} __PACK__ pds_mirror_session_info_t;

/// \brief    create mirror session
/// \param[in] spec    mirror session configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_create(pds_mirror_session_spec_t *spec);

/// \brief    delete mirror session
/// \param[in] key    mirror session key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_delete(pds_mirror_session_key_t *key);

/// \brief get mirror session
/// \param[in] key    pointer to spec
/// \param[out] info  mirror session information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_get(pds_mirror_session_key_t *key,
                                 pds_mirror_session_info_t *info);

/// @}

#endif    ///  __INCLUDE_API_PDS_MIRROR_HPP__
