//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __NEXTHOP_IMPL_HPP__
#define __NEXTHOP_IMPL_HPP__

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/nexthop.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL - nexthop functionality
/// \ingroup PDS_NEXTHOP
/// @{

/// \brief  nexthop implementation
class nexthop_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize nexthop
    ///             impl instance
    /// \param[in]  spec    nexthop information
    /// \return     new instance of nexthop or NULL, in case of error
    static nexthop_impl *factory(pds_nexthop_spec_t *spec);

    /// \brief      release all the s/w state associated with the given nexthop,
    ///             if any, and free the memory
    /// \param[in]  impl    nexthop impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(nexthop_impl *impl);

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
    /// \param[in]  epoch    epoch being activated
    /// \param[in]  api_op   api operation
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj,
                                  pds_epoch_t epoch,
                                  api_op_t api_op,
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
    /// \param[in]  api_obj API object
    /// \param[in]  key pointer to nexthop key
    /// \param[out] info pointer to nexthop info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    uint16_t hw_id(void) { return hw_id_; }

private:
    /// \brief  constructor
    nexthop_impl() {
        hw_id_ = 0xFFFF;
    }

    /// \brief  destructor
    ~nexthop_impl() {}

    /// \brief     program nexthop related tables during create
    /// \param[in] epoch epoch being activated
    /// \param[in] nh    nexthop obj being programmed
    /// \param[in] spec  nexthop configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_(pds_epoch_t epoch, nexthop *nh,
                               pds_nexthop_spec_t *spec);

    /// \brief     program nexthop related tables during delete
    /// \param[in] epoch epoch being activated
    /// \param[in] nh    nexthop obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, nexthop *nh);

    /// \brief      populate specification with hardware information
    /// \param[out] spec specification
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_nexthop_spec_t *spec);

    /// \brief      populate status with hardware information
    /// \param[out] status status
    void fill_status_(pds_nexthop_status_t *status);

private:
    uint32_t    hw_id_;    ///< hardware id
};

/// helper function to populate nexthop related information
/// in the nexthop P4 table entry
#define nexthop_info    action_u.nexthop_nexthop_info
static inline sdk_ret_t
populate_nh_info_ (pds_nexthop_spec_t *spec,
                  nexthop_actiondata_t *nh_data)
{
    if_entry *intf;
    pds_encap_t encap;

    memset(nh_data, 0, sizeof(*nh_data));
    nh_data->action_id = NEXTHOP_NEXTHOP_INFO_ID;
    intf = if_db()->find(&spec->l3_if);
    if (!intf) {
        PDS_TRACE_ERR("Interface not found for nh group %u l3 if %u",
                      spec->key.id, spec->l3_if.id);
        return SDK_RET_INVALID_ARG;
    }
    if (intf->type() != PDS_IF_TYPE_L3) {
        PDS_TRACE_ERR("Unsupported interface %u type %u in nexthop %u",
                      intf->key().id, intf->type(), spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    nh_data->nexthop_info.port = intf->port();
    encap = intf->l3_encap();
    if (encap.type == PDS_ENCAP_TYPE_DOT1Q) {
        nh_data->nexthop_info.vlan = encap.val.vlan_tag;
    }
    sdk::lib::memrev(nh_data->nexthop_info.dmaco,
                     spec->underlay_mac, ETH_ADDR_LEN);
    // TODO: get this from the pinned mnic
    sdk::lib::memrev(nh_data->nexthop_info.smaco,
                     intf->l3_mac(), ETH_ADDR_LEN);
    return SDK_RET_OK;
}

static inline sdk_ret_t
fill_nh_spec_ (pds_nexthop_spec_t *spec, uint16_t hw_id) {
    p4pd_error_t p4pd_ret;
    nexthop_actiondata_t nh_data;

    if ((unlikely(hw_id == PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID))) {
        spec->type = PDS_NH_TYPE_BLACKHOLE;
        return SDK_RET_OK;
    }
    spec->type = PDS_NH_TYPE_UNDERLAY;
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_NEXTHOP,
                                      hw_id,
                                      NULL, NULL, &nh_data);
    if (unlikely(p4pd_ret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("Failed to read nexthop table at index %u", hw_id);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    sdk::lib::memrev(spec->underlay_mac,
                     nh_data.nexthop_info.dmaco, ETH_ADDR_LEN);
    // TODO walk if db and identify the l3_if
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_IMPL_HPP__
