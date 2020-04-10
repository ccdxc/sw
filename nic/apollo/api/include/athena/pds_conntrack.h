//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines Connection track API
///
//----------------------------------------------------------------------------


#ifndef __PDS_CONNTRACK_H__
#define __PDS_CONNTRACK_H__

#include "pds_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_CONNTRACK
/// @{

#define    PDS_CONNTRACK_ID_MAX    (3774874)

/// \brief Connection Tracking Flow Types
typedef enum pds_flow_type_e {
    PDS_FLOW_TYPE_TCP = 0,    ///< TCP flow
    PDS_FLOW_TYPE_UDP,        ///< UDP flow
    PDS_FLOW_TYPE_ICMP,       ///< ICMP flow
    PDS_FLOW_TYPE_OTHERS      ///< Other flow
} pds_flow_type_t;

/// \brief Connection Tracking Flow States
typedef enum pds_flow_state_e {
    UNESTABLISHED = 0,             ///< Connection unestablished
    PDS_FLOW_STATE_SYN_SENT,       ///< TCP SYN sent
    PDS_FLOW_STATE_SYN_RECV,       ///< TCP SYN received
    PDS_FLOW_STATE_SYNACK_SENT,    ///< TCP SYN ACK sent
    PDS_FLOW_STATE_SYNACK_RECV,    ///< TCP SYN ACK received
    ESTABLISHED,                   ///< Established
    FIN_SENT,                      ///< FIN Sent
    FIN_RECV,                      ///< FIN received
    TIME_WAIT,                     ///< Wait
    RST_CLOSE,                     ///< RST close
    REMOVED                        ///< Connection removed
} pds_flow_state_t;

/// \brief Connection tracking data
typedef struct pds_conntrack_data_s {
    pds_flow_type_t     flow_type;     ///< Flow Type
    pds_flow_state_t    flow_state;    ///< Flow State
} __PACK__ pds_conntrack_data_t;

/// \brief Connection tracking key
typedef struct pds_conntrack_key_s {
    uint32_t    conntrack_id;    ///< Connection Tracking State ID
} __PACK__ pds_conntrack_key_t;

/// \brief Connection tracking specification
typedef struct pds_conntrack_spec_s {
    pds_conntrack_key_t     key;    ///< Connection tracking key
    pds_conntrack_data_t    data;  ///< Connection tracking data
} __PACK__ pds_conntrack_spec_t;

/// \brief Connection tracking status
typedef struct pds_conntrack_status_s {
    uint32_t    timestamp;          ///< Current timestamp
} __PACK__ pds_conntrack_status_t;

/// \brief Epoch info
typedef struct pds_conntrack_info_s {
    pds_conntrack_spec_t      spec;      ///< Specification
    pds_conntrack_status_t    status;    ///< Status
} __PACK__ pds_conntrack_info_t;

/// \brief     create conntrack state entry
/// \param[in] spec conntrack specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid flow type and conntrack id should be passed
pds_ret_t pds_conntrack_state_create(pds_conntrack_spec_t *spec);

/// \brief     read conntrack state entry
/// \param[in] key conntrack key
//  \param[in] info conntrack info
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid conntrack id should be passed
pds_ret_t pds_conntrack_state_read(pds_conntrack_key_t *key,
                                   pds_conntrack_info_t *info);

/// \brief     update conntrack state entry
/// \param[in] spec conntrack specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid flow type and conntrack id should be passed
pds_ret_t pds_conntrack_state_update(pds_conntrack_spec_t *spec);

/// \brief     delete conntrack state entry
/// \param[in] key conntrack key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid conntrack id should be passed
pds_ret_t pds_conntrack_state_delete(pds_conntrack_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif    // __PDS_CONNTRACK_H__
