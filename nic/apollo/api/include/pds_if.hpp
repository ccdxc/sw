//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines interface API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_INTF_HPP__
#define __INCLUDE_API_PDS_INTF_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"

/// \defgroup PDS_INTF Interface API
/// @{

#define PDS_INTF_ID_INVALID 0     ///< invalid interface id

/// \brief interface type
typedef enum pds_if_type_e {
    PDS_IF_TYPE_NONE      = 0,
    // physical ports
    PDS_IF_TYPE_ETH       = 1,
    // L2 uplink interface
    PDS_IF_TYPE_UPLINK    = 2,
    // L2 port-channel
    PDS_IF_TYPE_UPLINK_PC = 3,
    // L3 interface
    PDS_IF_TYPE_L3        = 4,
    // Loopback interface
    PDS_IF_TYPE_LOOPBACK  = 5,
} pds_if_type_t;

static inline pds_if_type_t
ifindex_to_pds_if_type (pds_ifindex_t ifindex)
{
    switch (IFINDEX_TO_IFTYPE(ifindex)) {
    case IF_TYPE_ETH:
        return PDS_IF_TYPE_ETH;
    case IF_TYPE_UPLINK:
        return PDS_IF_TYPE_UPLINK;
    case IF_TYPE_UPLINK_PC:
        return PDS_IF_TYPE_UPLINK_PC;
    case IF_TYPE_L3:
        return PDS_IF_TYPE_L3;
    case IF_TYPE_LOOPBACK:
        return PDS_IF_TYPE_LOOPBACK;
    default:
        return PDS_IF_TYPE_NONE;
    }
}

/// \brief interface admin/operational state
typedef enum pds_if_state_e {
    PDS_IF_STATE_NONE = 0,
    PDS_IF_STATE_DOWN = 1,
    PDS_IF_STATE_UP   = 2,
} pds_if_state_t;

/// \brief uplink specific configuration
typedef struct pds_uplink_info_s {
    pds_obj_key_t   port;    ///< uplink port# this interface corresponds to
} __PACK__ pds_uplink_info_t;

/// \brief uplink port-channel specific configuration
typedef struct pds_uplink_pc_info_s {
    uint64_t    port_bmap;    ///< port bitmap
} __PACK__ pds_uplink_pc_info_t;

/// \brief L3 interface specific configuration
typedef struct pds_l3_if_info_s {
    pds_obj_key_t   vpc;          ///< vpc this L3 if belongs to
    ip_prefix_t     ip_prefix;    ///< IP address and subnet of this L3 intf
    pds_obj_key_t   port;         ///< physical port of this L3 interface
    pds_encap_t     encap;        ///< (optional) encap used for egress rewrite
    mac_addr_t      mac_addr;     ///< MAC address of this L3 interface
} __PACK__ pds_l3_if_info_t;

/// \brief loopback interface specific configuration
typedef struct pds_loopback_info_s {
    ip_prefix_t    ip_prefix;     ///< IP address and subnet host on this intf
} __PACK__ pds_loopback_info_t;

/// \brief interface specification
typedef struct pds_if_spec_s {
    pds_obj_key_t            key;           ///< interface key
    pds_if_type_t            type;          ///< type of the interface
    pds_if_state_t           admin_state;   ///< admin state of the interface
    union {
        pds_uplink_info_t    uplink_info;
        pds_uplink_pc_info_t uplink_pc_info;
        pds_l3_if_info_t     l3_if_info;
        pds_loopback_info_t  loopback_if_info;
    };
    // Tx/egress mirror session id list, if any
    uint8_t num_tx_mirror_sessions;
    pds_obj_key_t tx_mirror_sessions[PDS_MAX_MIRROR_SESSION];
    // Rx/ingress mirror session id list, if any
    uint8_t num_rx_mirror_sessions;
    pds_obj_key_t rx_mirror_sessions[PDS_MAX_MIRROR_SESSION];
} __PACK__ pds_if_spec_t;

/// \brief uplink interface status
typedef struct pds_if_uplink_status_s {
    uint16_t lif_id;
} __PACK__ pds_if_uplink_status_t;

/// \brief L3 interface status
typedef struct pds_l3_if_status_s {
} __PACK__ pds_l3_if_status_t;

typedef struct pds_if_loopback_status_s {
    ///< name of the loopback interface as seen on linux
    char name[SDK_MAX_NAME_LEN];
} __PACK__ pds_if_loopback_status_t;

/// \brief interface status
typedef struct pds_if_status_s {
    pds_ifindex_t ifindex;   ///< encoded interface index
    pds_if_state_t state;    ///< operational status of the interface
    union {
        /// uplink interface operational status
        pds_if_uplink_status_t uplink_status;
        /// L3 interface operational status
        pds_l3_if_status_t l3_if_status;
        /// loopback interface operational status
        pds_if_loopback_status_t loopback_status;
    };
} __PACK__ pds_if_status_t;

/// \brief interface statistics
typedef struct pds_if_stats_s {
} __PACK__ pds_if_stats_t;

/// \brief interface information
typedef struct pds_if_info_s {
    pds_if_spec_t      spec;      ///< specification
    pds_if_status_t    status;    ///< status
    pds_if_stats_t     stats;     ///< statistics
} __PACK__ pds_if_info_t;

typedef void (*if_read_cb_t)(void *info, void *ctxt);

/// \brief     create interface
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_if_create(pds_if_spec_t *spec,
                        pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read interface
/// \param[in]  key  key
/// \param[out] info information
/// \return     #SDK_RET_OK on success, failure status code on error
/// \remark     valid interface key should be passed
sdk_ret_t pds_if_read(pds_obj_key_t *key, pds_if_info_t *info);

/// \brief      read interface
/// \param[in]  cb   callback
/// \param[out] ctxt context for callback
/// \return     #SDK_RET_OK on success, failure status code on error
/// \remark     valid interface key should be passed
sdk_ret_t pds_if_read_all(if_read_cb_t cb, void *ctxt);

/// \brief     update interface
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid interface specification should be passed
sdk_ret_t pds_if_update(pds_if_spec_t *spec,
                        pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete interface
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid interface key should be passed
sdk_ret_t pds_if_delete(pds_obj_key_t *key,
                        pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_INTF_HPP__
