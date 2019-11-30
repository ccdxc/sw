//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// route table implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __ROUTE_IMPL_HPP__
#define __ROUTE_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ROUTE_TABLE_IMPL - route table functionality
/// \ingroup PDS_ROUTE
/// @{

/// \brief route table implementation
class route_table_impl : public impl_base {
public:
     /// \brief     factory method to allocate & initialize
     ///            route table impl instance
     /// \param[in] spec route table configuration
     /// \return    new instance of route table or NULL, in case of error
    static route_table_impl *factory(pds_route_table_spec_t *spec);

    /// \brief     release all the s/w state associated with the given
    ///            route table instance, if any, and free the memory
    /// \param[in] impl route table impl instance to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this)
    static void destroy(route_table_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;


    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] api_obj  API object holding the resources
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief     cleanup all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] api_obj  API object holding the resources
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] api_obj  (cloned) API object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to route table key
    /// \param[out] info pointer to route table info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    mem_addr_t lpm_root_addr(void) { return lpm_root_addr_; }

private:
    /// \brief constructor
    route_table_impl() {
        lpm_root_addr_ = 0xFFFFFFFFFFFFFFFFUL;
    }

    /// \brief destructor
    ~route_table_impl() {}

    /// \brief      fill the route table status
    /// \param[out] status status
    void fill_status_(pds_route_table_status_t *status);

private:
    // P4 datapath specific state
    mem_addr_t lpm_root_addr_;    ///< LPM tree's root node address
} __PACK__;

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __ROUTE_IMPL_HPP__
