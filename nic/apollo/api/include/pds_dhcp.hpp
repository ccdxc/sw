//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines DHCP APIs
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

#define PDS_MAX_DHCP_RELAY          16
#define PDS_MAX_DHCP_POLICY         128
#define PDS_MAX_BOOT_FILENAME_LEN   128

/// \brief DHCP relay spec
typedef struct pds_dhcp_relay_spec_s {
    pds_obj_key_t    vpc;          ///< vpc of the DHCP server
    ip_addr_t        server_ip;    ///< DHCP server ip
    ip_addr_t        agent_ip;     ///< (local) DHCP relay agent's IP address
} __PACK__ pds_dhcp_relay_spec_t;

/// \brief DHCP relay policy status
typedef struct pds_dhcp_relay_status_s {
} __PACK__ pds_dhcp_relay_status_t;

/// \brief DHCP relay statistics
typedef struct pds_dhcp_relay_stats_s {
} __PACK__ pds_dhcp_relay_stats_t;

/// \brief DHCP proxy policy spec
typedef struct pds_dhcp_proxy_spec_s {
    ip_addr_t        server_ip;                                     ///< server ip
    uint32_t         mtu;                                           ///< MTU specified to clients
    ip_addr_t        gateway_ip;                                    ///< gateway ip
    ip_addr_t        dns_server_ip;                                 ///< DNS server ip
    ip_addr_t        ntp_server_ip;                                 ///< NTP server ip
    char             domain_name[PDS_MAX_DOMAIN_NAME_LEN + 1];      ///< domain name
    char             boot_filename[PDS_MAX_BOOT_FILENAME_LEN + 1];  ///< initial/pxe boot file name
    uint32_t         lease_timeout;                                 ///< DHCP lease timeout
} __PACK__ pds_dhcp_proxy_spec_t;

/// \brief DHCP proxy policy status
typedef struct pds_dhcp_proxy_status_s {
} __PACK__ pds_dhcp_proxy_status_t;

/// \brief DHCP proxy policy statistics
typedef struct pds_dhcp_proxy_stats_s {
} __PACK__ pds_dhcp_proxy_stats_t;

/// \brief type of the DHCP policies supported
typedef enum pds_dhcp_policy_type_e {
    PDS_DHCP_POLICY_TYPE_NONE  = 0,
    PDS_DHCP_POLICY_TYPE_RELAY = 1,
    PDS_DHCP_POLICY_TYPE_PROXY = 2,
} pds_dhcp_policy_type_t;

/// \brief DHCP policy configuration
typedef struct pds_dhcp_policy_spec_s {
    /// DHCP policy key
    pds_obj_key_t key;
    /// type of the DHCP policy
    pds_dhcp_policy_type_t type;
    union {
        /// DHCP relay configuration
        pds_dhcp_relay_spec_t relay_spec;
        /// DHCP proxy configuration
        pds_dhcp_proxy_spec_t proxy_spec;
    };
} pds_dhcp_policy_spec_t;

/// \brief DHCP policy operational status
typedef struct pds_dhcp_policy_status_s {
    union {
        pds_dhcp_relay_status_t relay_status;
        pds_dhcp_proxy_status_t proxy_status;
    };
} pds_dhcp_policy_status_t;

/// \brief DHCP policy operational stats, if any
typedef struct pds_dhcp_policy_stats_s {
    union {
        pds_dhcp_relay_stats_t relay_stats;
        pds_dhcp_proxy_stats_t proxy_stats;
    };
} pds_dhcp_policy_stats_t;

/// \brief DHCP policy information
typedef struct pds_dhcp_policy_info_s {
    pds_dhcp_policy_spec_t spec;        ///< specification
    pds_dhcp_policy_status_t status;    ///< status
    pds_dhcp_policy_stats_t stats;      ///< statistics
} __PACK__ pds_dhcp_policy_info_t;

/// \brief create DHCP policy
/// \param[in] spec     DHCP policy configuration
/// \param[in] bctxt    batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_create(pds_dhcp_policy_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief read DHCP policy
/// \param[in]  key     DHCP policy key
/// \param[out] info    DHCP policy information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_read(pds_obj_key_t *key,
                               pds_dhcp_policy_info_t *info);

typedef void (*dhcp_policy_read_cb_t)(const pds_dhcp_policy_info_t *info,
                                      void *ctxt);

/// \brief      read all DHCP policies
/// \param[in]  cb      callback function
/// \param[in]  ctxt    opaque context passed to cb
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_read_all(dhcp_policy_read_cb_t cb, void *ctxt);

/// \brief update DHCP policy
/// \param[in] spec     DHCP policy configuration
/// \param[in] bctxt    batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_update(pds_dhcp_policy_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete DHCP policy
/// \param[in] key      DHCP policy key
/// \param[in] bctxt    batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_dhcp_policy_delete(pds_obj_key_t *key,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);
/// @}

#endif    // __INCLUDE_API_PDS_DHCP_HPP__
