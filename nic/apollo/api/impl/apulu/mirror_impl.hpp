//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mirror feature implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __MIRROR_IMPL_HPP__
#define __MIRROR_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/mirror.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {


/// \defgroup PDS_MIRROR_IMPL - mirror functionality
/// \ingroup PDS_MIRROR
/// @{

/// \brief mirror implementation
class mirror_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize mirror impl instance
    /// \param[in] spec mirror session information
    /// \return    new instance of miror session or NULL, in case of error
    static mirror_impl *factory(pds_mirror_session_spec_t *spec);

    /// \brief     release all the s/w state associated with the given mirror
    ///            session, if any, and free the memory
    /// \param[in] impl mirror session impl instance to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(mirror_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(mirror_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] api_obj API object for which resources are being reserved
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object,
    ///            if any (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     cleanup all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] api_obj  API object holding the resources
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     update all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] curr_obj current version of the unmodified object
    /// \param[in] prev_obj previous version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] api_obj (cloned) API object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   API operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to mirror session key
    /// \param[out] info pointer to mirror session info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief     return mirror session's h/w id
    /// \return    h/w id assigned to the mirror session
    uint16_t hw_id(void) const { return hw_id_; }

private:
    /// \brief constructor
    mirror_impl() {
        hw_id_ = 0xFFFF;
    }

    /// \brief destructor
    ~mirror_impl() {}

    /// \brief      program mirror session related tables during mirror session
    ///             create by enabling stage0 tables corresponding to new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  ms    mirror session being created
    /// \param[in]  spec    mirror session configuration
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_(pds_epoch_t epoch, mirror_session *ms,
                               pds_mirror_session_spec_t *spec);

    /// \brief      program mirror session related tables during mirror session
    ///             update by enabling stage0 tables corresponding to new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  ms    mirror session being updated
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_update_(pds_epoch_t epoch, mirror_session *ms,
                               api_obj_ctxt_t *obj_ctxt);

    /// \brief      program mirror session related tables during mirror session
    ///             delete by disabling stage0 tables corresponding to new epoch
    /// \param[in]  epoch epoch being activated
    /// \param[in]  ms    mirror session being deleted
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, mirror_session *ms);

    /// \brief      fill the mirror session spec
    /// \param[out] spec mirror session specification to be filled
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_mirror_session_spec_t *spec);

    /// \brief      fill the mirror session status
    /// \param[out] status mirror session h/w status
    void fill_status_(pds_mirror_session_status_t *status);

private:
    // P4 datapath specific state
    uint16_t hw_id_;    ///< hardware id
} __PACK__;

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MIRROR_IMPL_HPP__
