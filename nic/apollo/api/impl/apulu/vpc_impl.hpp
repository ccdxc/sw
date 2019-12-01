//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __VPC_IMPL_HPP__
#define __VPC_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

using sdk::table::handle_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL - vpc datapath functionality
/// \ingroup PDS_VPC
/// @{

/// \brief  VPC implementation
class vpc_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize vpc impl instance
    /// \param[in]  spec    vpc information
    /// \return     new instance of vpc or NULL, in case of error
    static vpc_impl *factory(pds_vpc_spec_t *spec);

    /// \brief      release all the state associated with the given vpc,
    ///             if any, and free the memory
    /// \param[in]  impl    vpc impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(vpc_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(vpc_impl *impl);

    /// \brief      allocate/reserve h/w resources for this object
    /// \param[in]  orig_obj old version of the unmodified object
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief      free h/w resources used by this object, if any
    ///             (this API is invoked during object deletes)
    /// \param[in]  api_obj    api object holding the resources
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief      program all h/w tables relevant to this object except
    ///             stage 0 table(s), if any
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief      re-program all hardware tables relevant to this object
    ///             except stage 0 table(s), if any and this reprogramming
    ///             must be based on existing state and any of the state
    ///             present in the dirty object list (like clone objects etc.)
    /// \param[in]  api_obj API object being activated
    /// \param[in]  api_op API operation
    /// \return     #SDK_RET_OK on success, failure status code on error
    // NOTE: this method is called when an object is in the dependent/puppet
    //       object list
    virtual sdk_ret_t reprogram_hw(api_base *api_obj, api_op_t api_op) override;

    /// \brief      cleanup all h/w tables relevant to this object except
    ///             stage 0 table(s), if any, by updating packed entries with
    ///             latest epoch#
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief      update all h/w tables relevant to this object except stage 0
    ///             table(s), if any, by updating packed entries with latest
    ///             epoch#
    /// \param[in]  orig_obj old version of the unmodified object
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /// \brief      activate the epoch in the dataplane by programming stage 0
    ///             tables, if any
    /// \param[in]  api_obj  (cloned) API object being activated
    /// \param[in]  orig_obj previous/original unmodified object
    /// \param[in]  epoch    epoch being activated
    /// \param[in]  api_op   api operation
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) override;

    /// \brief      re-activate config in the hardware stage 0 tables relevant
    ///             to this object, if any, this reactivation must be based on
    ///             existing state and any of the state present in the dirty
    ///             object list (like clone objects etc.) only and not directly
    ///             on db objects
    // \param[in]   api_obj API object being activated
    /// \param[in]  api_op API operation
    /// \return     #SDK_RET_OK on success, failure status code on error
    // NOTE: this method is called when an object is in the dependent/puppet
    //       object list
    virtual sdk_ret_t reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                                    api_op_t api_op) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to vpc key
    /// \param[out] info pointer to vpc info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

private:
    /// \brief  constructor
    vpc_impl() {
        vni_hdl_ = handle_t::null();
    }

    /// \brief  destructor
    ~vpc_impl() {}

    /// \brief      program vpc related tables during vpc create by enabling
    ///             stage0 tables corresponding to the new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  vpc    vpc obj being programmed
    /// \param[in]  spec    vpc configuration
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_(pds_epoch_t epoch, vpc_entry *vpc,
                               pds_vpc_spec_t *spec);

    /// \brief      program vpc related tables during vpc update by enabling
    ///             stage0 tables corresponding to the new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  vpc    vpc obj being programmed
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_update_(pds_epoch_t epoch, vpc_entry *vpc,
                               obj_ctxt_t *obj_ctxt);

    /// \brief      program vpc related tables during vpc delete by disabling
    ///             stage0 tables corresponding to the new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  vpc    vpc obj being programmed
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, vpc_entry *vpc);

private:
    handle_t    vni_hdl_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __VPC_IMPL_HPP__
