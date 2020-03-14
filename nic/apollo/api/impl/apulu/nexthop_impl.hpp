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
#include "nic/apollo/api/impl/apulu/if_impl.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "gen/p4gen/p4/include/ftl.h"

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

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(nexthop_impl *impl);

    /// \brief      allocate/reserve h/w resources for this object
    /// \param[in]  api_obj API object for which resources are being reserved
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

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
                                 api_obj_ctxt_t *obj_ctxt) override {
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
                                   api_obj_ctxt_t *obj_ctxt) override;

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
                                    api_obj_ctxt_t *obj_ctxt) override;

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

    /// \brief     program nexthop related tables during create/update
    /// \param[in] epoch epoch being activated
    /// \param[in] nh    nexthop obj being programmed
    /// \param[in] spec  nexthop configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_update_(pds_epoch_t epoch, nexthop *nh,
                                      pds_nexthop_spec_t *spec);

    /// \brief     program nexthop related tables during delete
    /// \param[in] epoch epoch being activated
    /// \param[in] nh    nexthop obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, nexthop *nh);

    /// \brief      populate specification with hardware information
    /// \param[out] spec specification
    /// \param[in]  nh_data nexthop info entry
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_nexthop_spec_t *spec, nexthop_info_entry_t *nh_data);

    /// \brief      populate status with hardware information
    /// \param[out] status status
    /// \param[in]  nh_data nexthop info entry
    void fill_status_(pds_nexthop_status_t *status, nexthop_info_entry_t *nh_data);

    /// \brief      populate nh info with hardware information
    /// \param[out] info nexthop info
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_info_(pds_nexthop_info_t *info);

private:
    uint32_t    hw_id_;    ///< hardware id
};

/// helper function to populate nexthop related information
/// in the nexthop P4 table entry
#define nexthop_info    action_u.nexthop_nexthop_info
static inline sdk_ret_t
populate_underlay_nh_info_ (pds_nexthop_spec_t *spec,
                            nexthop_info_entry_t *nh_data)
{
    if_entry *intf, *eth_if;
    lif_impl *lif;
    pds_encap_t encap;

    PDS_TRACE_ERR("l3_if %s", spec->l3_if.str());
    memset(nh_data, 0, nexthop_info_entry_t::entry_size());
    intf = if_db()->find(&spec->l3_if);
    if (!intf) {
        PDS_TRACE_ERR("L3 intf %s not found for nexthop %s",
                      spec->l3_if.str(), spec->key.str());
        return SDK_RET_INVALID_ARG;
    }
    if (intf->type() != PDS_IF_TYPE_L3) {
        PDS_TRACE_ERR("Unsupported interface %s type %u in nexthop %s",
                      intf->key().str(), intf->type(), spec->key.str());
        return SDK_RET_INVALID_ARG;
    }
    nh_data->set_port(if_impl::port(intf));
    encap = intf->l3_encap();
    if (encap.type == PDS_ENCAP_TYPE_DOT1Q) {
        nh_data->set_vlan(encap.val.vlan_tag);
    }
    nh_data->set_dmaco(MAC_TO_UINT64(spec->underlay_mac));

    // program the src mac
    if (!is_mac_set(intf->l3_mac())) {
        // if user didn't give MAC explicitly, use the MAC of the corresponding
        // lif (that is visible on DSC's linux)
        eth_if = (if_entry *)if_entry::eth_if(intf);
        lif = lif_impl_db()->find(sdk::platform::LIF_TYPE_MNIC_INBAND_MGMT,
                                  eth_if->ifindex());
        nh_data->set_smaco(MAC_TO_UINT64(lif->mac()));
    } else {
        // use the MAC coming in the config
        nh_data->set_smaco(MAC_TO_UINT64(intf->l3_mac()));
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
fill_nh_spec_ (pds_nexthop_spec_t *spec, uint16_t hw_id) {
    sdk_ret_t ret;
    nexthop_info_entry_t nh_data;

    if ((unlikely(hw_id == PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID))) {
        spec->type = PDS_NH_TYPE_BLACKHOLE;
        return SDK_RET_OK;
    }
    memset(&nh_data, 0, nexthop_info_entry_t::entry_size());
    spec->type = PDS_NH_TYPE_UNDERLAY;
    ret = nh_data.read(hw_id);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read nexthop table at index %u", hw_id);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    MAC_UINT64_TO_ADDR(spec->underlay_mac, nh_data.get_dmaco());
    // TODO walk if db and identify the l3_if
    return SDK_RET_OK;
}

static inline sdk_ret_t
fill_nh_status_ (pds_nexthop_status_t *status, uint16_t hw_id) {
    sdk_ret_t ret;
    nexthop_info_entry_t nh_data;

    if ((unlikely(hw_id == PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID))) {
        return SDK_RET_OK;
    }
    memset(&nh_data, 0, nexthop_info_entry_t::entry_size());
    ret = nh_data.read(hw_id);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read nexthop table at index %u", hw_id);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    status->port = nh_data.get_port();
    status->vlan = nh_data.get_vlan();
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_IMPL_HPP__
