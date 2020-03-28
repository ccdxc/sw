//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// DHCP implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __DHCP_IMPL_HPP__
#define __DHCP_IMPL_HPP__

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/dhcp.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

///\defgroup PDS_DHCP_IMPL - DHCP functionality
///\ingroup PDS_DHCP
/// @{

///\brief DHCP policy implementation
class dhcp_policy_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize DHCP replay instance
    /// \param[in] spec DHCP policy specification
    /// \return    new instance of DHCP policy impl or NULL, in case of error
    static dhcp_policy_impl *factory(pds_dhcp_policy_spec_t *spec);

    /// \brief     release all the s/w state associated with the given DHCP
    ///            policy instance, if any, and free the memory
    /// \param[in] impl DHCP policy instance to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(dhcp_policy_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(dhcp_policy_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] api_obj  object for which resources are being reserved
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj, api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding this resource
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

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

    /// \brief      read h/w state corresponding to this object, if any
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to DHCP policy object key
    /// \param[out] info pointer to DHCP policy info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief     return nacl table (base) index allocated for this policy
    /// \return    nacl table (base) idx assigned to the vnic
    uint16_t nacl_idx(void) const { return nacl_idx_; }

private:
    /// \brief constructor
    dhcp_policy_impl() {
        nacl_idx_ = 0xFFFF;
    }

    /// \brief destructor
    ~dhcp_policy_impl() {}

private:
    uint16_t nacl_idx_;    ///< base index of this policy in NACL table
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __DHCP_IMPL_HPP__
