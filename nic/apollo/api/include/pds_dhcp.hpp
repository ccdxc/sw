//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines dhcp APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_DHCP_HPP__
#define __INCLUDE_API_PDS_DHCP_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

/// \defgroup PDS_DHCP DHCP API
/// @{

/// \brief dhcp relay agent spec
typedef struct pds_dhcp_relay_agent_spec_s {
    pds_dhcp_relay_agent_id_t id;           ///< id
    pds_vpc_key_t             vpc;          ///< vpc
    ip_addr_t                 server_ip;    ///< server ip
    ip_addr_t                 agent_ip;     ///< agent ip
} __PACK__ pds_dhcp_relay_agent_spec_t;

/// \brief dhcp relay agent status
typedef struct pds_dhcp_relay_agent_status_s {
} __PACK__ pds_dhcp_relay_agent_status_t;

/// \brief dhcp relay agent statistics
typedef struct pds_dhcp_relay_agent_stats_s {
} __PACK__ pds_dhcp_relay_agent_stats_t;

/// \brief dhcp relay agent information
typedef struct pds_dhcp_relay_agent_info_s {
    pds_dhcp_relay_agent_spec_t spec;            ///< specification
    pds_dhcp_relay_agent_status_t status;        ///< status
    pds_dhcp_relay_agent_stats_t stats;          ///< statistics
} __PACK__ pds_dhcp_relay_agent_info_t;

#define PDS_MAX_DOMAIN_NAME_LEN     (128)
/// \brief dhcp policy spec
typedef struct pds_dhcp_policy_spec_s {
    pds_dhcp_policy_id_t id;                                    ///< id
    ip_addr_t            server_ip;                             ///< server ip
    uint32_t             mtu;                                   ///< MTU specified to clients
    ip_addr_t            gw_ip;                                 ///< gateway ip
    ip_addr_t            dns_server_ip;                         ///< DNS server ip
    ip_addr_t            ntp_server_ip;                         ///< NTP server ip
    char                 domain_name[PDS_MAX_DOMAIN_NAME_LEN];  ///< domain name
} __PACK__ pds_dhcp_policy_spec_t;

/// \brief dhcp policy status
typedef struct pds_dhcp_policy_status_s {
} __PACK__ pds_dhcp_policy_status_t;

/// \brief dhcp policy statistics
typedef struct pds_dhcp_policy_stats_s {
} __PACK__ pds_dhcp_policy_stats_t;

/// \brief dhcp policy information
typedef struct pds_dhcp_policy_info_s {
    pds_dhcp_policy_spec_t spec;            ///< specification
    pds_dhcp_policy_status_t status;        ///< status
    pds_dhcp_policy_stats_t stats;          ///< statistics
} __PACK__ pds_dhcp_policy_info_t;

/// \brief create dhcp relay agent
/// \param[in] spec dhcp relay agent configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_relay_agent_create(pds_dhcp_relay_agent_spec_t *spec,
                                      pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief read dhcp relay agent
/// \param[in] key dhcp relay agent key
/// \param[out] info dhcp relay agent information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_relay_agent_read(pds_dhcp_relay_agent_id_t *id,
                                    pds_dhcp_relay_agent_info_t *info);

/// \brief update dhcp relay agent
/// \param[in] spec dhcp relay agent configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_relay_agent_update(pds_dhcp_relay_agent_spec_t *spec,
                                      pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete dhcp relay agent
/// \param[in] id id
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_relay_agent_delete(pds_dhcp_relay_agent_id_t *id,
                                      pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief create dhcp policy
/// \param[in] spec dhcp policy configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_create(pds_dhcp_policy_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief read dhcp policy
/// \param[in] key dhcp policy key
/// \param[out] info dhcp policy information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_read(pds_dhcp_policy_id_t *id,
                               pds_dhcp_policy_info_t *info);

/// \brief update dhcp policy
/// \param[in] spec dhcp policy configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_update(pds_dhcp_policy_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete dhcp policy
/// \param[in] id id
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_delete(pds_dhcp_policy_id_t *id,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);
/// @}

#endif    // __INCLUDE_API_PDS_DHCP_HPP__
