//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines policer API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_POLICER_HPP__
#define __INCLUDE_API_PDS_POLICER_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/qos.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_POLICER Policer API
/// @{

/// invalid policer key
#define PDS_POLICER_ID_INVALID    k_pds_obj_key_invalid
#define PDS_MAX_TX_POLICER        1024
#define PDS_MAX_RX_POLICER        1024
#define PDS_MAX_POLICER           (PDS_MAX_TX_POLICER + PDS_MAX_RX_POLICER)

/// \brief direction of the traffic the policer is applied on
typedef enum pds_policer_dir_e {
    PDS_POLICER_DIR_NONE    = 0,
    /// INGRESS is w.r.t vnic, policer is applied on traffic sent towards vnic
    PDS_POLICER_DIR_INGRESS = 1,
    /// EGRESS is w.r.t vnic, policer is applied on traffic sent from vnic
    PDS_POLICER_DIR_EGRESS  = 2,
} pds_policer_dir_t;

/// \brief policer specification
typedef struct pds_policer_spec_s {
    pds_obj_key_t key;              ///< policer's unique key
    sdk::qos::policer_type_t type;  ///< type of the policer
    pds_policer_dir_t dir;          ///< traffic direction policer is applied on
    union {
        struct {
            uint64_t pps;           ///< packets per second threshold
            uint64_t pps_burst;     ///< burst to absorb
        };
        struct {
            uint64_t bps;           ///< bytes per second threshold
            uint64_t bps_burst;     ///< burst to absorb
        };
    };
} __PACK__ pds_policer_spec_t;

/// \brief policer status
typedef struct pds_policer_status_s {
    uint16_t hw_id;    ///< hardware id
} __PACK__ pds_policer_status_t;

/// \brief policer statistics, if any
typedef struct pds_policer_stats_s {
} __PACK__ pds_policer_stats_t;

/// \brief policer information
typedef struct pds_policer_info_s {
    pds_policer_spec_t   spec;      ///< specification
    pds_policer_status_t status;    ///< status
    pds_policer_stats_t  stats;     ///< statistics
} __PACK__ pds_policer_info_t;

/// \brief     create policer
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid policer id should be passed (PDS_POLICER_ID_INVALID
///            is reserved to indicate that policing is not needed)
sdk_ret_t pds_policer_create(pds_policer_spec_t *spec,
                             pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read policer
/// \param[in]  key     key of the policer
/// \param[out] info    policer information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policer_read(pds_obj_key_t *key, pds_policer_info_t *info);

typedef void (*policer_read_cb_t)(pds_policer_info_t *info, void *ctxt);

/// \brief      read all policer
/// \param[in]  cb      callback function
/// \param[in]  ctxt    opaque context passed to cb
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_policer_read_all(policer_read_cb_t cb, void *ctxt);

/// \brief     update policer
/// \param[in] spec new config specification of the policer
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid policer specification should be passed
sdk_ret_t pds_policer_update(pds_policer_spec_t *spec,
                             pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete policer
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid policer key should be passed
sdk_ret_t pds_policer_delete(pds_obj_key_t *key,
                             pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_POLICER_HPP__
