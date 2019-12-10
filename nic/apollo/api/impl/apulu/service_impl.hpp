//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// service mapping implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __SVC_MAPPING_IMPL_HPP__
#define __SVC_MAPPING_IMPL_HPP__

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"

using sdk::table::handle_t;

namespace api {
namespace impl {

/// \defgroup PDS_SVC_MAPPING_IMPL service mapping implementation class
/// \ingroup PDS_SVC_MAPPING
/// @{

/// \brief  service mapping implementation
class svc_mapping_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize service mapping
    ///             impl instance
    /// \param[in]  spec service mapping information
    /// \return     new instance of service mapping or NULL, in case of error
    static svc_mapping_impl *factory(pds_svc_mapping_spec_t *spec);

    /// \brief      release all the s/w state associated with the given service
    ///             mapping, if any, and free the memory
    /// \param[in]  impl service mapping impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(svc_mapping_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(svc_mapping_impl *impl);

    /// \brief      instantiate a service mapping impl object based on current
    ///             state
    ///             (sw and/or hw) given its key
    /// \param[in]  key service mapping entry's key
    /// \param[in]  mapping service mapping's API object
    /// \return     new instance of service mapping implementation object or NULL
    static svc_mapping_impl *build(pds_svc_mapping_key_t *key,
                                   svc_mapping *mapping);

    /// \brief      free a stateless entry's temporary s/w only resources like
    ///             memory etc., for a stateless entry calling destroy() will
    ///             remove resources from h/w, which can't be done during ADD/UPD
    ///             etc. operations esp. when object is constructed on the fly
    /// \param[in]  impl service mapping impl instance to be freed
    static void soft_delete(svc_mapping_impl *impl);

    /// \brief      allocate/reserve h/w resources for this object
    /// \param[in]  api_obj API object for which resources are being reserved
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief      free h/w resources used by this object, if any
    /// \param[in]  api_obj api object holding the resources
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief      free h/w resources used by this object, if any
    ///             (this API is invoked during object deletes)
    /// \param[in]  api_obj api object holding the resources
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief      program all h/w tables relevant to this object except
    ///             stage 0 table(s), if any
    /// \param[in]  api_obj  api object being programmed
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief      cleanup all h/w tables relevant to this object except
    ///             stage 0 table(s), if any, by updating packed entries with
    ///             latest epoch#
    /// \param[in]  api_obj     api object being cleaned up
    /// \param[in]  obj_ctxt    transient state associated with this API
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

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to service mapping key
    /// \param[out] info pointer to service mapping info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

private:
    /// \brief  constructor
    svc_mapping_impl() {
        to_vip_nat_idx_ = PDS_IMPL_RSVD_NAT_HW_ID;
        to_vip_handle_ = handle_t::null();
    }

    /// \brief  destructor
    ~svc_mapping_impl() {}

    /// \brief     program and activate service mapping related tables during
    ///            create by enabling stage0 tables corresponding to the
    ///            new epoch
    /// \param[in] epoch       epoch being activated
    /// \param[in] mapping     service mapping instance
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \param[in] spec        service mapping configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_create_(pds_epoch_t epoch,
                                       svc_mapping *mapping,
                                       obj_ctxt_t *obj_ctxt,
                                       pds_svc_mapping_spec_t *spec);

    /// \brief     program and activate service mapping related tables during
    ///            delete by enabling stage0 tables corresponding to the
    ///            new epoch
    /// \param[in] epoch       epoch being activated
    /// \param[in] key         key identifying the mapping
    /// \param[in] mapping     service mapping instance
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_delete_(pds_epoch_t epoch,
                                       pds_svc_mapping_key_t *key,
                                       svc_mapping* mapping);

    /// \brief      fill the service mapping spec
    /// \param[out] spec specification
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_svc_mapping_spec_t *spec);

    /// \brief      fill the service mapping stats
    /// \param[out] stats statistics
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_stats_(pds_svc_mapping_stats_t *stats);

    /// \brief      fill the service mapping status
    /// \param[out] status status
    void fill_status_(pds_svc_mapping_status_t *status);

private:
    // handles or indices to NAT/NAT2 table(s)
    uint32_t    to_vip_nat_idx_;

    // handles to SERVICE_MAPPING table
    handle_t    to_vip_handle_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __SVC_MAPPING_IMPL_HPP__
