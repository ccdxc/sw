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
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_INTF Interface API
/// @{

#define PDS_INTF_ID_INVALID 0     ///< invalid interface id

/// \brief interface type
typedef enum pds_if_type_e {
    PDS_IF_TYPE_NONE      = 0,
    // L2 uplink interface
    PDS_IF_TYPE_UPLINK    = 1,
    // L2 port-channel
    PDS_IF_TYPE_UPLINK_PC = 2,
    // L3 interface
    PDS_IF_TYPE_L3        = 3,
} pds_if_type_t;

/// \brief interface admin/operational state
typedef enum pds_if_state_e {
    PDS_IF_STATE_NONE = 0,
    PDS_IF_STATE_DOWN = 1,
    PDS_IF_STATE_UP   = 2,
} pds_if_state_t;

/// \brief uplink specific configuration
typedef struct pds_uplink_info_s {
    uint8_t    port_num;    ///< uplink port# this interface corresponds to
} __PACK__ pds_uplink_info_t;

/// \brief uplink port-channel specific configuration
typedef struct pds_uplink_pc_info_s {
    uint64_t    port_bmap;    ///< port bitmap
} __PACK__ pds_uplink_pc_info_t;

/// \brief L3 interface specific configuration
typedef struct pds_l3_if_info_s {
    pds_obj_key_t   vpc;          ///< vpc this L3 if belongs to
    ip_prefix_t     ip_prefix;    ///< IP address and subnet of this L3 if
    pds_ifindex_t   eth_ifindex;  ///< Eth ifindex of this L3 if
    pds_encap_t     encap;        ///< (optional) encap used for egress rewrite
    mac_addr_t      mac_addr;     ///< MAC address of this L3 interface
} __PACK__ pds_l3_if_info_t;

/// \brief interface specification
typedef struct pds_if_spec_s {
    pds_if_key_t             key;           ///< interface key
    pds_if_type_t            type;          ///< type of the interface
    pds_if_state_t           admin_state;   ///< admin state of the interface
    union {
        pds_uplink_info_t    uplink_info;
        pds_uplink_pc_info_t uplink_pc_info;
        pds_l3_if_info_t     l3_if_info;
    };
} __PACK__ pds_if_spec_t;

/// \brief uplink interface status
typedef struct pds_if_uplink_status_s {
    uint16_t lif_id;
} __PACK__ pds_if_uplink_status_t;

/// \brief interface status
typedef struct pds_if_status_s {
    pds_if_state_t    state;    ///< operational status of the interface
    union {
        pds_if_uplink_status_t uplink_status;
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
sdk_ret_t pds_if_read(pds_if_key_t *key, pds_if_info_t *info);

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
sdk_ret_t pds_if_delete(pds_if_key_t *key,
                        pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_INTF_HPP__
