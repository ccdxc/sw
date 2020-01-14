//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc peering implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __VPC_PEER_IMPL_HPP__
#define __VPC_PEER_IMPL_HPP__

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/vpc_peer.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"

using sdk::table::handle_t;

namespace api {
namespace impl {

///\defgroup PDS_VPC_PEER_IMPL - vpc peering functionality
///\ingroup PDS_VPC
/// @{

///\brief vpc peering implementation
class vpc_peer_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize vpc peering impl instance
    /// \param[in] spec    vpc peering specification
    /// \return    new instance of vpc peering impl instance or NULL, in case of error
    static vpc_peer_impl *factory(pds_vpc_peer_spec_t *spec);

    /// \brief     release all the s/w state associated with the given vpc
    ///            peering, if any, and free the memory
    /// \param[in] impl    vpc peering impl instance to be freed
    static void destroy(vpc_peer_impl *impl);

    /// \brief     instantiate a vpc peering impl object based on current state
    ///            (sw and/or hw) given its key
    /// \param[in] key    vpc peering entry's key
    /// \param[in] vpc_peer    vpc peer API object
    /// \return    new instance of vpc peering implementation object or NULL
    static vpc_peer_impl *build(pds_obj_key_t *key,
                                vpc_peer_entry *vpc_peer);

    /// \brief     free a stateless entry's temporary s/w only resources like
    ///            memory etc., for a stateless entry calling destroy() will
    ///            remove resources from h/w, which can't be done during ADD/UPDATE
    ///            etc. operations esp. when object is constructed on the fly
    /// \param[in] impl    vpc peering impl instance to be freed
    static void soft_delete(vpc_peer_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief     program all h/w tables relevant to this object except stage 0
    ///            table(s), if any
    /// \param[in] api_obj  API object being programmed
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] api_obj  API object being cleaned up
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming stage 0
    ///            tables, if any
    /// \param[in] api_obj  (cloned) API api object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch   epoch being activated
    /// \param[in] api_op  API operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  key  pointer to vpc peering key
    /// \param[out] info pointer to vpc peering info
    /// \param[in]  arg  pointer to boolean having local true/false
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_hw(pds_obj_key_t *key, pds_vpc_peer_info_t *info);

private:
    /// \brief constructor
    vpc_peer_impl() {
        vpc_peer_handle1_ = handle_t::null();
        vpc_peer_handle2_ = handle_t::null();
    }

    /// \brief destructor
    ~vpc_peer_impl() {}

private:
    handle_t vpc_peer_handle1_;    // handle for (vpc1, vpc2) key
    handle_t vpc_peer_handle2_;    // handle for (vpc2, vpc1) key
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __VPC_PEER_IMPL_HPP__
