//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines Device API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_DEVICE_HPP__
#define __INCLUDE_API_DEVICE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds.hpp"

#define PDS_DROP_REASON_MAX        64    ///< Maximum packet drop reasons
#define PDS_MAX_DROP_NAME_LEN      32    ///< Packet drop reason string length

/// \defgroup PDS_DEVICE Device API
/// \@{

///< operational mode of the device
typedef enum pds_device_oper_mode_e {
    PDS_DEV_OPER_MODE_NONE = 0,
    ///< bump-in-the-wire mode with workloads on uplink port(s)
    PDS_DEV_OPER_MODE_BITW = 1,
    ///< host path mode with workloads on pcie
    PDS_DEV_OPER_MODE_HOST = 2,
} pds_device_oper_mode_t;

/// \brief device specification
typedef struct pds_device_s {
    ip_addr_t              device_ip_addr;     ///< device IP address
    mac_addr_t             device_mac_addr;    ///< device MAC address
    ip_addr_t              gateway_ip_addr;    ///< gateway IP address
    bool                   bridging_en;        ///< enable or disable L2 functionality
    bool                   learning_en;        ///< enable or disable L2/IP learning
    uint32_t               learn_age_timeout;  ///< MAC, IP aging timeout (in seconds)  for learnt entries
    bool                   overlay_routing_en; ///< enable or disable control plane stack
                                               ///  for overlay routing
    pds_device_oper_mode_t dev_oper_mode;      ///< device operational mode
} __PACK__ pds_device_spec_t;

/// \brief device status
typedef struct pds_device_status_s {
    mac_addr_t  fru_mac;    ///< FRU MAC
    uint8_t     memory_cap; ///< Memory capacity
} __PACK__ pds_device_status_t;

/// \brief Drop statistics
typedef struct pds_device_drop_stats_s {
    char     name[PDS_MAX_DROP_NAME_LEN];   ///< drop reason name
    uint64_t count;                         ///< drop count
} __PACK__ pds_device_drop_stats_t;

/// \brief device statistics
typedef struct pds_device_stats_s {
    ///< number of entries in the ingress drop statistics
    uint32_t ing_drop_stats_count;
    ///< number of entries in the egress drop statistics
    uint32_t egr_drop_stats_count;
    ///<< ingress drop statistics
    pds_device_drop_stats_t ing_drop_stats[PDS_DROP_REASON_MAX];
    ///< egress drop statistics
    pds_device_drop_stats_t egr_drop_stats[PDS_DROP_REASON_MAX];
} __PACK__ pds_device_stats_t;

/// \brief device information
typedef struct pds_device_info_s {
    pds_device_spec_t   spec;      ///< specification
    pds_device_status_t status;    ///< status
    pds_device_stats_t  stats;     ///< statistics
} __PACK__ pds_device_info_t;

/// \brief     create device
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    device is a global configuration and can be created only once.
///            Any other validation that is expected on the TEP should be done
///            by the caller
sdk_ret_t pds_device_create(pds_device_spec_t *spec,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read device
/// \param[out] info device information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_read(pds_device_info_t *info);

/// \brief     update device
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid device specification should be passed
sdk_ret_t pds_device_update(pds_device_spec_t *spec,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief  delete device
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_delete(pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_DEVICE_HPP__
