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
#include "nic/apollo/api/include/pds_vpc.hpp"

/// \defgroup PDS_MIRROR    mirror session APIs
/// @{

#define PDS_MAX_MIRROR_SESSION    8   ///< Maximum mirror sessions

/// \brief    RSPAN configuration
typedef struct pds_rspan_spec_s {
    pds_obj_key_t interface;    ///< outgoing (lif/uplink) interface
    pds_encap_t encap;          ///< encap details
} __PACK__ pds_rspan_spec_t;

/// \brief    ERSPAN configuration
/// \remark    source IP used in the ERSPAN packet is either:
///            1. subnet VR IP in case DstIP is in a VPC of type VPC_TYPE_TENANT
///            2. local TEP (MyTEP) IP in case DstIP is in VPC  of type
///               VPC_TYPE_UNDERLAY
typedef struct pds_erspan_spec_s {
    pds_obj_key_t vpc;                ///< vpc of the destination IP
    union {
        ///< ERSPAN destination is underlay TEP (vpc is underlay VPC in
        ///< this case)
        pds_obj_key_t tep;
        ///< ERSPAN destination is local or remote mapping IP (vpc is
        ///< overlay IP in this case)
        pds_obj_key_t mapping;
    };
    uint32_t dscp;                    ///< DSCP value to use in the packet
    uint32_t span_id;                 ///< SPAN ID used in ERSPAN header
} __PACK__ pds_erspan_spec_t;

/// \brief    mirror session type
typedef enum pds_mirror_session_type_e {
    PDS_MIRROR_SESSION_TYPE_RSPAN  = 0,    ///< RSPAN mirror session type
    PDS_MIRROR_SESSION_TYPE_ERSPAN = 1,    ///< ERSPAN mirror session type
    PDS_MIRROR_SESSION_TYPE_MAX    = PDS_MIRROR_SESSION_TYPE_ERSPAN,
} pds_mirror_session_type_t;

/// \brief    mirror session configuration
typedef struct pds_mirror_session_spec_s {
    pds_obj_key_t key;                    ///< key of the mirror session
    pds_mirror_session_type_t type;       ///< mirror session type
    uint16_t snap_len;                    ///< max len. of pkt mirrored
    union {
        pds_rspan_spec_t rspan_spec;      ///< RSPAN configuration
        pds_erspan_spec_t erspan_spec;    ///< ERSPAN configuration
    };
} __PACK__ pds_mirror_session_spec_t;

/// \brief    mirror session operational status
typedef struct pds_mirror_session_status_s {
} __PACK__ pds_mirror_session_status_t;

/// \brief    mirror session statistics, if any
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
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_create(pds_mirror_session_spec_t *spec,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    read mirror session information
/// \param[in] key    key of the mirror session
/// \param[out] info    mirror session information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_read(pds_obj_key_t *key,
                                  pds_mirror_session_info_t *info);

typedef void (*mirror_session_read_cb_t)(const pds_mirror_session_info_t *info, void *ctxt);

/// \brief    read all mirror session information
/// \param[in]  read_cb    callback function
/// \param[in]  ctxt    opaque context passed to cb
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_read_all(mirror_session_read_cb_t read_cb,
                                      void *ctxt);

/// \brief read mirror session
/// \param[in] key    pointer to mirror session key
/// \param[out] info  mirror session information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_read(pds_obj_key_t *key,
                                  pds_mirror_session_info_t *info);

/// \brief    update mirror session
/// \param[in] spec    mirror session configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_update(pds_mirror_session_spec_t *spec,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief    delete mirror session
/// \param[in] key    mirror session key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_mirror_session_delete(pds_obj_key_t *key,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    ///  __INCLUDE_API_PDS_MIRROR_HPP__
