//----------------------------------------------------------------------------
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_LIF_HPP__
#define __INCLUDE_API_PDS_LIF_HPP__

#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/framework/state_base.hpp"

typedef struct pds_lif_spec_s {
    ///< key for the lif
    pds_lif_key_t    key;
    ///< if index of the pinned port/lif
    pds_ifindex_t    pinned_ifidx;
    ///< type of lif
    lif_type_t       type;
    ///< vlan_strip_en is set to true if vlan needs to be stripped in datapath
    bool vlan_strip_en;
} pds_lif_spec_t;

/// \brief Read LIF information
///
/// \param[in]  key     key
/// \param[out] spec    lif spec
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_lif_read(pds_lif_key_t *key, pds_lif_spec_t *spec);

typedef void (*lif_read_cb_t)(void *spec, void *ctxt);

/// \brief Read all LIF information
///
/// \param[in] cb      callback to be called on each lif_impl
/// \param[in] ctxt    context for the callback
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_lif_read_all(lif_read_cb_t cb, void *ctxt);

#endif    ///  __INCLUDE_API_PDS_LIF_HPP__
