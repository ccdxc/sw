//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// security policy implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __SECURITY_POLICY_IMPL_HPP__
#define __SECURITY_POLICY_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SECURITY_POLICY_IMPL - security policy functionality
/// \ingroup PDS_SECURITY_POLICY
/// @{

/// \brief security policy implementation
class security_policy_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize
    ///            security policy impl instance
    /// \param[in] spec security policy spec
    /// \return    new instance of security policy or NULL, in case of error
    static security_policy_impl *factory(pds_policy_spec_t *spec);

    /// \brief     release all the s/w state associated with the given
    ///            security policy instance, if any, and free the memory
    /// \param[in] impl security policy impl instance to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this)
    static void destroy(security_policy_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(security_policy_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding this resource
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] api_obj  API object holding this resource
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries with
    ///            latest epoch#
    /// \param[in] curr_obj API object of this resource
    /// \param[in] prev_obj API object of this resource
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] api_obj  (cloned) API object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   API operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to policy key
    /// \param[out] info pointer to policy info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    mem_addr_t security_policy_root_addr(void) {
        return security_policy_root_addr_;
    }

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

private:
    /// \brief constructor
    security_policy_impl() {
        security_policy_root_addr_ = 0xFFFFFFFFFFFFFFFFUL;
    }

    /// \brief  destructor
    ~security_policy_impl() {}

    /// \brief      fill the policy status
    /// \param[out] status status
    void fill_status_(pds_policy_status_t *status);

private:
    //P4 datapath specific state
    mem_addr_t  security_policy_root_addr_;    ///< policy root node address
} __PACK__;

 /// \@}

}    // namespace impl
}    // namespace api

#endif    // __SECURITY_POLICY_IMPL_HPP__
