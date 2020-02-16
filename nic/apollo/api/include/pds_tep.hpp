//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines Tunnel EndPoint (TEP) APIs
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_TEP_HPP__
#define __INCLUDE_API_PDS_TEP_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"

/// \defgroup PDS_TEP Tunnel End Point API
/// @{

#define PDS_MAX_TEP 1023    ///< maximum tunnels

/// \brief type of the TEP
typedef enum pds_tep_type_e {
    PDS_TEP_TYPE_NONE     = 0,    ///< invalid TEP type
    PDS_TEP_TYPE_IGW      = 1,    ///< TEP for north-south traffic going
                                  ///< to internet
    PDS_TEP_TYPE_WORKLOAD = 2,    ///< TEP for east-west traffic between
                                  ///< workloads
    PDS_TEP_TYPE_INTER_DC = 3,    ///< TEP for inter-DC traffic
    PDS_TEP_TYPE_SERVICE  = 4,    ///< service tunnel
} pds_tep_type_t;

/// \brief TEP specification
typedef struct pds_tep_spec_s {
    pds_obj_key_t  key;        ///< key
    pds_obj_key_t  vpc;        ///< VPC this tunnel belongs to
    ip_addr_t      ip_addr;    ///< outer source IP to be used
                               ///< (unused currently)
    ip_addr_t      remote_ip;  ///< TEP IP
    mac_addr_t     mac;        ///< MAC address of this TEP
    pds_tep_type_t type;       ///< type/role of the TEP
    /// encap to be used, if specified
    /// for PDS_TEP_TYPE_WORKLOAD type TEP, encap value itself comes from the
    /// mapping configuration so need not be specified here, however for the
    /// PDS_TEP_TYPE_IGW, encap has to be specified here ... PDS will take
    /// the encap value, if specified here, always so agent needs to set this
    /// appropriately
    pds_encap_t    encap;
    /// perform SNAT for traffic going to this TEP and DNAT for traffic coming
    /// from this TEP, if this is set to true (note that mappings need to have
    /// public IP configured for this to take effect)
    bool           nat;
    /// true, if tunnel is of type PDS_TEP_TYPE_SERVICE and service is in
    /// remote data center/pod
    bool           remote_svc;
    /// fabric encap for remote service
    pds_encap_t    remote_svc_encap;
    /// public IP of this service tunnel that is used as SIPo in the
    /// encapsulated VXLAN packet in the Tx direction
    ip_addr_t      remote_svc_public_ip;
    /// type of the nexthop for this mapping
    pds_nh_type_t  nh_type;
    /// forwarding information for this TEP can be one of
    /// 1. nexthop
    /// 2. nexthop group
    /// 3. another TEP (with encap type as MPLSoUDP)
    /// NOTE:
    /// 1. A tunnel can point to another tunnel is supported only when the
    ///    2nd tunnel's encap is MPLSoUDP
    /// 2. 2nd tunnel can't point to another tunnel
    union {
        pds_obj_key_t nh;                ///< underlay nexthop
        pds_obj_key_t nh_group;    ///< underlay nexthop group
        pds_obj_key_t tep;                   ///< another tunnel/TEP
    };
} __PACK__ pds_tep_spec_t;

/// \brief TEP status
typedef struct pds_tep_status_s {
    uint16_t nh_id;                 ///< next hop id for this TEP
    uint16_t hw_id;                 ///< hardware id
    uint8_t  dmac[ETH_ADDR_LEN];    ///< outer destination MAC
} __PACK__ pds_tep_status_t;

/// \brief TEP statistics
typedef struct pds_tep_stats_s {
    // TODO: No Stats for TEP
} __PACK__ pds_tep_stats_t;

/// \brief TEP information
typedef struct pds_tep_info_s {
    pds_tep_spec_t   spec;      ///< specification
    pds_tep_status_t status;    ///< status
    pds_tep_stats_t  stats;     ///< statistics
} __PACK__ pds_tep_info_t;

/// \brief     create TEP
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    TEP with same key (i.e., IP address) should not be created again
///            Any other validation that is expected on the TEP should be done
///            by the caller
sdk_ret_t pds_tep_create(pds_tep_spec_t *spec,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read TEP
/// \param[in]  key key
/// \param[out] info information
/// \return     #SDK_RET_OK on success, failure status code on error
/// \remark     TEP spec containing a valid tep key should be passed
sdk_ret_t pds_tep_read(pds_obj_key_t *key, pds_tep_info_t *info);

/// \brief     update TEP
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    valid TEP specification should be passed
sdk_ret_t pds_tep_update(pds_tep_spec_t *spec,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete TEP
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    valid TEP key should be passed
sdk_ret_t pds_tep_delete(pds_obj_key_t *key,
                         pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_TEP_HPP__
