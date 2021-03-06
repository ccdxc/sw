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
#include "nic/sdk/lib/table/slhash/slhash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"

using sdk::table::slhash;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL_STATE - mapping state functionality
/// \ingroup PDS_MAPPING
/// \@{

// forward declaration
class mapping_impl;

///< state maintained for mappings
class mapping_impl_state : public state_base {
public:
    ///< \brief    constructor
    mapping_impl_state(pds_state *state);

    ///< \brief    destructor
    ~mapping_impl_state();

    /// \brief      allocate memory required for a mapping impl instance
    /// \return     pointer to the allocated instance, NULL if no memory
    mapping_impl *alloc(void);

    /// \brief      free mapping impl instance back to slab
    /// \param[in]  impl    pointer to the allocated impl instance
    void free(mapping_impl *impl);

    /// \brief    API to initiate transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief    API to end transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief     API to get table stats
    /// \param[in]  cb    callback to be called on stats
    ///             ctxt    opaque ctxt passed to the callback
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt);

private:
    slhash *local_ip_mapping_tbl(void) { return local_ip_mapping_tbl_; }
    mem_hash *mapping_tbl(void) { return mapping_tbl_; }
    slab *mapping_impl_slab(void) { return mapping_impl_slab_; }
    // mapping_impl class is friend of mapping_impl_state
    friend class mapping_impl;

private:
    mem_hash *mapping_tbl_;
    slhash *local_ip_mapping_tbl_;
    slab *mapping_impl_slab_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MAPPING_IMPL_STATE_HPP__
