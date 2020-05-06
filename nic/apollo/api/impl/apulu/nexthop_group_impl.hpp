//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __NEXTHOP_GROUP_IMPL_HPP__
#define __NEXTHOP_GROUP_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/nexthop_group.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_GROUP_IMPL - nexthop group functionality
/// \ingroup PDS_NEXTHOP
/// @{

/// \brief  nexthop group implementation
class nexthop_group_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize nexthop group
    ///             impl instance
    /// \param[in]  spec    nexthop group information
    /// \return     new instance of nexthop group or NULL, in case of error
    static nexthop_group_impl *factory(pds_nexthop_group_spec_t *spec);

    /// \brief      release all the s/w state associated with the given
    ///             nexthop group, if any, and free the memory
    /// \param[in]  impl    nexthop group impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(nexthop_group_impl *impl);

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(nexthop_group_impl *impl);

    /// \brief     stash this object into persistent storage
    /// \param[in] info pointer to the info object
    /// \param[in] upg_info contains location to put stashed object
    /// \return    sdk_ret_ok or error code
    virtual sdk_ret_t backup(obj_info_t *info, upg_obj_info_t *upg_info) override;

    /// \brief     restore stashed object from persistent storage
    /// \param[in] info pointer to the info object
    /// \param[in] upg_info contains location to read stashed object
    /// \return    sdk_ret_ok or error code
    virtual sdk_ret_t restore(obj_info_t *info, upg_obj_info_t *upg_info) override;

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

    /// \brief restore h/w resources for this obj from persistent storage
    /// \param[in] info pointer to the info object
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t restore_resources(obj_info_t *info) override;

    /// \brief      program all h/w tables relevant to this object except
    ///             stage 0 table(s), if any
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) {
        return SDK_RET_OK;
    }

    /// \brief      re-program all hardware tables relevant to this object
    ///             except stage 0 table(s), if any and this reprogramming
    ///             must be based on existing state and any of the state
    ///             present in the dirty object list (like clone objects etc.)
    /// \param[in]  api_obj API object being activated
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    // NOTE: this method is called when an object is in the dependent/puppet
    //       object list
    virtual sdk_ret_t reprogram_hw(api_base *api_obj,
                                   api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_INVALID_OP;
    }

    /// \brief      cleanup all h/w tables relevant to this object except
    ///             stage 0 table(s), if any, by updating packed entries with
    ///             latest epoch#
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief      update all h/w tables relevant to this object except stage 0
    ///             table(s), if any, by updating packed entries with latest
    ///             epoch#
    /// \param[in]  orig_obj old version of the unmodified object
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

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
                                    api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_INVALID_OP;
    }

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj API object
    /// \param[in]  key pointer to nexthop group key
    /// \param[out] info pointer to nexthop group info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief     return nexthop group's h/w id
    /// \return    h/w id of this nexthop group
    uint16_t hw_id(void) const { return hw_id_; }

    /// \brief     return base index of the group of nexthops in this group
    /// \return    base idx of 1st nexthop in this nexthop group
    uint16_t nh_base_hw_id(void) const { return nh_base_hw_id_; }

private:
    /// \brief  constructor
    nexthop_group_impl() {
        hw_id_ = 0xFFFF;
        nh_base_hw_id_ = 0xFFFF;
    }

    /// \brief  destructor
    ~nexthop_group_impl() {}

    /// \brief     populate overlay ECMP information in the ECMP table
    ///            entry
    /// \param[in] spec         nexthop group specification
    /// \param[in] ecmp_data    P4 table entry pointer
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t populate_ecmp_tep_info_(pds_nexthop_group_spec_t *spec,
                                      ecmp_actiondata_t *ecmp_data);

    /// \brief     program nexthop group related tables during create/update by
    ///            enabling stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] nh_group nexthop group obj being programmed
    /// \param[in] spec  nexthop configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_update_(pds_epoch_t epoch,
                                      nexthop_group *nh_group,
                                      pds_nexthop_group_spec_t *spec);

    /// \brief     program nexthop group related tables during delete by
    ///            disabling stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] nh_group nexthop group obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, nexthop_group *nh_group);

    /// \brief      populate specification with hardware information
    /// \param[out] spec specification
    /// \param[in]  ecmp_data ecmp data
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_nexthop_group_spec_t *spec,
                         ecmp_actiondata_t *ecmp_data);

    /// \brief      populate status with hardware information
    /// \param[out] status status
    /// \param[in]  ecmp_data ecmp data
    /// \param[in]  spec specification
    sdk_ret_t fill_status_(pds_nexthop_group_status_t *status,
                           ecmp_actiondata_t *ecmp_data,
                           pds_nexthop_group_spec_t *spec);

    /// \brief      populate info with hardware information
    /// \param[out] info information
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_info_(pds_nexthop_group_info_t *info);

private:
    ///< hardware id of this nexthop group
    uint16_t    hw_id_;
    ///< base index of NEXTHOP table in case this group consists of underlay
    ///< nexthops
    uint16_t    nh_base_hw_id_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_GROUP_IMPL_HPP__
