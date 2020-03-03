//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mappping implementation state
///
//----------------------------------------------------------------------------

#ifndef __MAPPING_IMPL_STATE_HPP__
#define __MAPPING_IMPL_STATEHPP__

#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
extern "C" {
#include <dhcpctl/dhcpctl.h>
}

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL_STATE - mapping state functionality
/// \ingroup PDS_MAPPING
/// \@{

// forward declaration
class mapping_impl;

/// \brief state maintained for mappings
class mapping_impl_state : public state_base {
public:
    /// \brief constructor
    mapping_impl_state(pds_state *state);

    /// \brief destructor
    ~mapping_impl_state();

    /// \brief  allocate memory required for a mapping impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    mapping_impl *alloc(void);

    /// \brief     free mapping impl instance back to slab
    /// \param[in] impl pointer to the allocated impl instance
    void free(mapping_impl *impl);

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief     API to get table stats
    /// \param[in] cb   callback to be called on stats
    /// \param[in] ctxt opaque ctxt passed to the callback
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt);

    /// \brief     API to dump mapping entries
    /// \param[in] fd   file descriptor where entries are dumped
    /// \param[in] args command arguments
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t mapping_dump(int fd, cmd_args_t *args);

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief add dhcp binding for this mapping
    /// \param[in] spec    mapping configuration
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert_dhcp_binding(const pds_mapping_spec_t *spec);

    /// \brief add remove binding corresponding to this mapping
    /// \param[in] hostnmae    hostname corresponding to the mapping
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t remove_dhcp_binding(const char *hostname);

private:
    mem_hash *local_mapping_tbl(void) { return local_mapping_tbl_; }
    mem_hash *mapping_tbl(void) { return mapping_tbl_; }
    rte_indexer *ip_mac_binding_idxr(void) { return ip_mac_binding_idxr_; }
    slab *mapping_impl_slab(void) { return mapping_impl_slab_; }
    friend class mapping_impl;    ///< friend of mapping_impl_state
    friend class vnic_impl;       ///< friend of mapping_impl_state
    friend void local_mapping_dump_cb(sdk_table_api_params_t *params);

private:
    mem_hash *mapping_tbl_;
    mem_hash *local_mapping_tbl_;
    rte_indexer *ip_mac_binding_idxr_;
    dhcpctl_handle dhcp_connection_;
    slab *mapping_impl_slab_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MAPPING_IMPL_STATE_HPP__
