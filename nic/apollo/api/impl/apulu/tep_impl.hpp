//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __TEP_IMPL_HPP__
#define __TEP_IMPL_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL - TEP functionality
/// \ingroup PDS_TEP
/// @{

/// \brief TEP implementation
class tep_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize TEP impl instance
    /// \param[in] spec specification
    /// \return    new instance of TEP or NULL, in case of error
    static tep_impl *factory(pds_tep_spec_t *spec);

    /// \brief     release all the s/w state associated with the given TEP,
    ///            if any, and free the memory
    /// \param[in] impl TEP to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(tep_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(tep_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] api_obj API object for which resources are being reserved
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj api object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj api object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to tep key. Not used.
    /// \param[out] info pointer to tep info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
        return SDK_RET_OK;
    }

    /// \brief     cleanup all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries with
    ///            latest epoch#
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
        return SDK_RET_OK;
    }

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with
    ///            latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) {
        return SDK_RET_OK;
    }

    /// \brief     activate the epoch in the dataplane by programming stage 0
    ///            tables, if any
    /// \param[in] api_obj  (cloned) API object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt) override;

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
                                    api_op_t api_op) override {
        // TODO: real implementation will come later based on pipeline layout
        return SDK_RET_INVALID_OP;
    }

    /// \brief     return h/w index for this TEP
    /// \return    h/w table index for this TEP
    uint16_t hw_id(void) const { return hw_id_; }

private:
    /// \brief constructor
    tep_impl() {
        hw_id_ = 0xFFFF;
    }

    /// \brief destructor
    ~tep_impl() {}

    /// \brief     program outer tunnel (MPLSoUDP in this case) related tables
    ///            during TEP create by enabling stage0 tables corresponding to
    ///            the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] tep  TEP obj being programmed
    /// \param[in] spec TEP configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_tunnel2_(pds_epoch_t epoch, tep_entry *tep,
                                       pds_tep_spec_t *spec);

    /// \brief     program TEP related tables during TEP create by enabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] tep  TEP obj being programmed
    /// \param[in] spec TEP configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_tunnel_table_(pds_epoch_t epoch, tep_entry *tep,
                                            pds_tep_spec_t *spec);

    /// \brief     program TEP related tables during TEP create by enabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] tep  TEP obj being programmed
    /// \param[in] spec TEP configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_(pds_epoch_t epoch, tep_entry *tep,
                               pds_tep_spec_t *spec);

    /// \brief     program outer tunnel (MPLSoUDP in this case) related tables
    ///            during TEP delete by disabling stage0 tables corresponding to
    ///            the new epoch
    /// \param[in] epoch epoch being deleted
    /// \param[in] tep  TEP obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_tunnel2_(pds_epoch_t epoch, tep_entry *tep);

    /// \brief     program TEP related tables during TEP delete by disabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being deleted
    /// \param[in] tep  TEP obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_tunnel_table_(pds_epoch_t epoch, tep_entry *tep);

    /// \brief     program TEP related tables during TEP delete by disabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being deleted
    /// \param[in] tep   TEP obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, tep_entry *tep);

    /// \brief     update outer tunnel (MPLSoUDP in this case) related tables
    ///            during TEP update by enabling stage0 tables corresponding to
    ///            the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] tep  TEP obj being updated
    /// \param[in] spec TEP configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_update_tunnel2_(pds_epoch_t epoch, tep_entry *tep,
                                       pds_tep_spec_t *spec);

    /// \brief     program TEP related tables during TEP update by enabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] tep  TEP obj being updated
    /// \param[in] spec TEP configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_update_tunnel_table_(pds_epoch_t epoch, tep_entry *tep,
                                            pds_tep_spec_t *spec);

    /// \brief     program TEP related tables during TEP update by enabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] tep  TEP obj being updated
    /// \param[in] orig_tep old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_update_(pds_epoch_t epoch, tep_entry *tep,
                               tep_entry *orig_tep, api_obj_ctxt_t *obj_ctxt);

    /// \brief      populate specification with hardware information
    /// \param[out] spec     specification
    sdk_ret_t fill_spec_(pds_tep_spec_t *spec);

    /// \brief      populate status with hardware information
    /// \param[out] status   status
    void fill_status_(pds_tep_status_t *status);

private:
    // P4 datapath specific state
    uint16_t   hw_id_;    ///< hardware id for this tep
} __PACK__;

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __TEP_IMPL_HPP__
