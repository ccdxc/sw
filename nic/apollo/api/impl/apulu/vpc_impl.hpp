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
    /// \param[in]  api_obj  object for which resources are being reserved
    /// \param[in]  orig_obj old version of the unmodified object
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj, api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief      free h/w resources used by this object, if any
    ///             (this API is invoked during object deletes)
    /// \param[in]  api_obj    api object holding the resources
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] api_obj     api object associated with the impl instance
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg, api_base *api_obj,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      program all h/w tables relevant to this object except
    ///             stage 0 table(s), if any
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      cleanup all h/w tables relevant to this object except
    ///             stage 0 table(s), if any, by updating packed entries with
    ///             latest epoch#
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      update all h/w tables relevant to this object except stage 0
    ///             table(s), if any, by updating packed entries with latest
    ///             epoch#
    /// \param[in]  orig_obj old version of the unmodified object
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

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
                                  api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      re-program all hardware tables relevant to this object
    ///             except stage 0 table(s), if any and this reprogramming
    ///             must be based on existing state and any of the state
    ///             present in the dirty object list (like clone objects etc.)
    /// \param[in]  api_obj API object being activated
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reprogram_hw(api_base *api_obj,
                                   api_obj_ctxt_t *obj_ctxt) override {
        // other object updates don't affect subnet h/w programming when subnet
        // is sitting in the dependent object list, any updates to route
        // table(s) and/or policy table(s) that subnet is pointing to are
        // propagated to  respective vnics by now
        return SDK_RET_OK;
    }

    /// \brief      re-activate config in the hardware stage 0 tables relevant
    ///             to this object, if any, this reactivation must be based on
    ///             existing state and any of the state present in the dirty
    ///             object list (like clone objects etc.) only and not directly
    ///             on db objects
    /// \param[in]  api_obj  (cloned) API api object being activated
    /// \param[in]  epoch    epoch being activated
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                                    api_obj_ctxt_t *obj_ctxt) override {
        // other object updates don't affect subnet h/w programming when subnet
        // is sitting in the dependent object list, any updates to route
        // table(s) and/or policy table(s) that subnet is pointing to are
        // propagated to  respective vnics by now
        return SDK_RET_OK;
    }

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to vpc key
    /// \param[out] info pointer to vpc info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief     return vpc's h/w id
    /// \return    h/w id assigned to the vpc
    uint16_t hw_id(void) const { return hw_id_; }

    /// \brief     return vpc's BD h/w id
    /// \return    BD h/w id assigned to the vpc
    uint16_t bd_hw_id(void) const { return bd_hw_id_; }

    /// \brief      get the key from entry in hash table context
    /// \param[in]  entry in the hash table context
    /// \return     hw id from the entry
    static void *key_get(void *entry) {
        vpc_impl *vpc = (vpc_impl *) entry;
        return (void *)&(vpc->hw_id_);
    }

    /// \brief      accessor API for key
    pds_obj_key_t *key(void) { return &key_; }

    /// \brief      accessor API for hash table context
    ht_ctxt_t *ht_ctxt(void) { return &ht_ctxt_; }
private:
    /// \brief  constructor
    vpc_impl() {
        hw_id_  = 0xFFFF;
        bd_hw_id_ = 0xFFFF;
        vni_hdl_ = handle_t::null();
        ht_ctxt_.reset();
    }

    /// \brief  constructor with spec
    vpc_impl(pds_vpc_spec_t *spec) {
        hw_id_ = 0xFFFF;
        bd_hw_id_ = 0xFFFF;
        vni_hdl_ = handle_t::null();
        key_ = spec->key;
        ht_ctxt_.reset();
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
                               api_obj_ctxt_t *obj_ctxt);

    /// \brief      program vpc related tables during vpc delete by disabling
    ///             stage0 tables corresponding to the new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  vpc    vpc obj being programmed
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, vpc_entry *vpc);

private:
    uint16_t    hw_id_;
    uint16_t    bd_hw_id_;
    handle_t    vni_hdl_;
    /// PI specific info
    struct {
        pds_obj_key_t key_;
    };
    ht_ctxt_t ht_ctxt_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __VPC_IMPL_HPP__
