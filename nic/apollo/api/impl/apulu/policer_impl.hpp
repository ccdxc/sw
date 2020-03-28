//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// policer implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __POLICER_IMPL_HPP__
#define __POLICER_IMPL_HPP__

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/policer.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_POLICER_IMPL - policer functionality
/// \ingroup PDS_POLICER
/// @{

/// \brief  policer implementation
class policer_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize policer
    ///             impl instance
    /// \param[in]  spec    policer information
    /// \return     new instance of policer or NULL, in case of error
    static policer_impl *factory(pds_policer_spec_t *spec);

    /// \brief      release all the s/w state associated with the given policer,
    ///             if any, and free the memory
    /// \param[in]  impl    policer impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(policer_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(policer_impl *impl);

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

    /// \brief      re-program all hardware tables relevant to this object
    ///             except stage 0 table(s), if any and this reprogramming
    ///             must be based on existing state and any of the state
    ///             present in the dirty object list (like clone objects etc.)
    /// \param[in]  api_obj API object being activated
    /// \param[in]  api_obj    api object holding the resources
    /// \return     #SDK_RET_OK on success, failure status code on error
    // NOTE: this method is called when an object is in the dependent/puppet
    //       object list
    virtual sdk_ret_t reprogram_hw(api_base *api_obj,
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
                                    api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj API object
    /// \param[in]  key pointer to policer key
    /// \param[out] info pointer to policer info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    uint16_t hw_id(void) { return hw_id_; }

private:
    /// \brief  constructor
    policer_impl() {
        hw_id_ = 0xFFFF;
    }

    /// \brief  destructor
    ~policer_impl() {}

    /// \brief      program policer related tables during policer create by
    ///             enabling stage0 tables corresponding to the new epoch
    /// \param[in]  epoch      epoch being activated
    /// \param[in]  policer    policer obj being programmed
    /// \param[in]  spec       policer configuration
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_(pds_epoch_t epoch, policer_entry *policer,
                               pds_policer_spec_t *spec);

    /// \brief      program policer related tables during policer update by
    ///             enabling stage0 tables corresponding to the new epoch
    /// \param[in]  epoch      epoch being activated
    /// \param[in]  policer    cloned policer obj being programmed
    /// \param[in]  spec       policer configuration
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_update_(pds_epoch_t epoch, policer_entry *policer,
                               pds_policer_spec_t *spec);

    /// \brief     program policer related tables during delete
    /// \param[in] epoch epoch being activated
    /// \param[in] policer    policer obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, policer_entry *policer);

    /// \brief      populate specification with hardware information
    /// \param[out] spec specification
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_policer_spec_t *spec);

    /// \brief      populate status with hardware information
    /// \param[out] status status
    void fill_status_(pds_policer_status_t *status);

    /// \brief      populate stats
    /// \param[out] stats stats
    void fill_stats_(pds_policer_stats_t *stats);

private:
    uint16_t    hw_id_;    ///< hardware id
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __POLICER_IMPL_HPP__
