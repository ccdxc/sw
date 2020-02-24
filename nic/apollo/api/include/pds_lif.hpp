//----------------------------------------------------------------------------
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_LIF_HPP__
#define __INCLUDE_API_PDS_LIF_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/framework/state_base.hpp"

typedef struct pds_lif_spec_s {
    ///< key for the lif
    pds_obj_key_t    key;
    ///< internal lif id
    pds_lif_id_t     id;
    ///< name of the lif, if any
    char name[SDK_MAX_NAME_LEN];
    ///< if index of the pinned port/lif
    pds_ifindex_t    pinned_ifidx;
    ///< type of lif
    lif_type_t       type;
    ///< vlan_strip_en is set to true if vlan needs to be stripped in datapath
    bool vlan_strip_en;
    ///< mac address of the device
    mac_addr_t mac;
    // Tx/egress mirror session id list, if any
    pds_obj_key_t tx_mirror_sessions[PDS_MAX_MIRROR_SESSION];
    // Rx/ingress mirror session id list, if any
    pds_obj_key_t rx_mirror_sessions[PDS_MAX_MIRROR_SESSION];
} pds_lif_spec_t;

/// \brief lif status
typedef struct pds_lif_status_s {
    ///< encoded interface index of this lif
    pds_ifindex_t  ifindex;
    ///< name of the lif (as seen on host)
    char           name[SDK_MAX_NAME_LEN];
    ///< operational status of the lif
    pds_if_state_t state;
} __PACK__ pds_lif_status_t;

typedef struct pds_lif_info_s {
    pds_lif_spec_t spec;
    pds_lif_status_t status;
} __PACK__ pds_lif_info_t;

/// \brief Read LIF information
///
/// \param[in]  key     key
/// \param[out] info    lif info
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_lif_read(pds_obj_key_t *key, pds_lif_info_t *info);

typedef void (*lif_read_cb_t)(void *spec, void *ctxt);

/// \brief Read all LIF information
///
/// \param[in] cb      callback to be called on each lif_impl
/// \param[in] ctxt    context for the callback
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_lif_read_all(lif_read_cb_t cb, void *ctxt);

#endif    // __INCLUDE_API_PDS_LIF_HPP__
