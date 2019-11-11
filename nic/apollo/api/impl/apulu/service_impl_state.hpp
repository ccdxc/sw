//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// service mappping implementation state
///
//----------------------------------------------------------------------------

#ifndef __SVC_MAPPING_IMPL_STATE_HPP__
#define __SVC_MAPPING_IMPL_STATEHPP__

#include "nic/sdk/lib/table/slhash/slhash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SVC_MAPPING_IMPL_STATE - service mapping state functionality
/// \ingroup PDS_SERVICE
/// \@{

// forward declaration
class svc_mapping_impl;

///< state maintained for service mappings
class svc_mapping_impl_state : public state_base {
public:
    ///< \brief    constructor
    svc_mapping_impl_state(pds_state *state);

    ///< \brief    destructor
    ~svc_mapping_impl_state();

    /// \brief      allocate memory required for a service mapping impl instance
    /// \return     pointer to the allocated instance, NULL if no memory
    svc_mapping_impl *alloc(void);

    /// \brief      free service mapping impl instance back to slab
    /// \param[in]  impl    pointer to the allocated impl instance
    void free(svc_mapping_impl *impl);

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

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

private:
    slhash *svc_mapping_tbl(void) { return svc_mapping_tbl_; }
    slab *svc_mapping_impl_slab(void) { return svc_mapping_impl_slab_; }
    // svc_mapping_impl class is friend of svc_mapping_impl_state
    friend class svc_mapping_impl;

private:
    slhash *svc_mapping_tbl_;
    slab *svc_mapping_impl_slab_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __SVC_MAPPING_IMPL_STATE_HPP__
