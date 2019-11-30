//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mapping implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __MAPPING_IMPL_HPP__
#define __MAPPING_IMPL_HPP__

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"

using sdk::table::handle_t;

namespace api {
namespace impl {

///\defgroup PDS_MAPPING_IMPL - mapping functionality
///\ingroup PDS_MAPPING
/// @{

///\brief mapping implementation
class mapping_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize mapping impl instance
    /// \param[in] spec mapping specification
    /// \return    new instance of mapping or NULL, in case of error
    static mapping_impl *factory(pds_mapping_spec_t *spec);

    /// \brief     release all the s/w state associated with the given mapping,
    ///            if any, and free the memory
    /// \param[in] impl mapping to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(mapping_impl *impl);

    /// \brief     instantiate a mapping impl object based on current state
    ///            (sw and/or hw) given its key
    /// \param[in] key mapping entry's key
    /// \param[in] mapping mapping entry's API object
    /// \return    new instance of mapping implementation object or NULL
    static mapping_impl *build(pds_mapping_key_t *key, mapping_entry *mapping);

    /// \brief     free a stateless entry's temporary s/w only resources like
    ///            memory etc., for a stateless entry calling destroy() will
    ///            remove resources from h/w, which can't be done during ADD/UPDATE
    ///            etc. operations esp. when object is constructed on the fly
    /// \param[in] impl mapping to be freed
    static void soft_delete(mapping_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief     program all h/w tables relevant to this object except stage 0
    ///            table(s), if any
    /// \param[in] api_obj  API object being programmed
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] api_obj  API object being cleaned up
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                obj_ctxt_t *obj_ctxt) override;

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
                                  obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  key  pointer to mapping key
    /// \param[out] info pointer to mapping info
    /// \param[in]  arg  pointer to boolean having local true/false
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_hw(pds_mapping_key_t *key,
                      pds_mapping_info_t *info);

    /// \brief  return true if mapping is local, false otherwise
    /// \return true or false
    bool is_local(void) const { return is_local_; }

    /**
     * @brief    set is_local_ value of this mapping_impl object
     * @param[in] val    set is_local_ for this mapping_impl object
     */
    void set_is_local(bool val);

private:
    /// \brief constructor
    mapping_impl() {
        overlay_ip_to_public_ip_nat_hdl_ = PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX;
        overlay_ip_to_provider_ip_nat_hdl_ = PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX;
        to_overlay_ip_nat_hdl_ = PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX;
        nh_idx_ = 0xFFFFFFFF;
        overlay_ip_hdl_ = handle_t::null();
        public_ip_hdl_ = handle_t::null();
        provider_ip_hdl_ = handle_t::null();
        mapping_hdl_ = handle_t::null();
    }

    /// \brief destructor
    ~mapping_impl() {}

    /// \brief     add necessary entries to NAT table
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_nat_entries_(pds_mapping_spec_t *spec);

    /// \brief     reserve necessary entries in local mapping tables
    /// \param[in] api_obj API object being processed
    /// \param[in] vpc     VPC of this IP
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_local_ip_mapping_resources_(api_base *api_obj,
                                                  vpc_entry *vpc,
                                                  pds_mapping_spec_t *spec);

    /// \brief     add necessary entries to local mapping tables
    /// \param[in] vpc  VPC of this IP
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_local_ip_mapping_entries_(vpc_entry *vpc,
                                            pds_mapping_spec_t *spec);

    /// \brief     reserve necessary entries in remote mapping tables
    /// \param[in] api_obj API object being processed
    /// \param[in] vpc     VPC of this IP
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_remote_mapping_resources_(api_base *api_obj,
                                                vpc_entry *vpc,
                                                pds_mapping_spec_t *spec);

    /// \brief     add necessary mapping table entries for remote IPs
    /// \param[in] vpc    VPC of this IP
    /// \param[in] subnet subnet of this IP
    /// \param[in] spec   IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_remote_mapping_entries_(vpc_entry *vpc,
                                          pds_mapping_spec_t *spec);

    /// \brief         read the configured values from the local mapping tables
    /// \param[in]     vpc  pointer to the vpc entry
    /// \param[in/out] spec pointer to the spec
    /// \return        SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_local_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec);

    /// \brief         read the configured values from the local mapping tables
    /// \param[in]     vpc  pointer to the vpc entry
    /// \param[in/out] spec pointer to the spec
    /// \return        SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_remote_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec);

    /// \brief     release all the resources reserved for local IP mapping
    /// \param[in] api_obj mapping object
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t release_local_ip_mapping_resources_(api_base *api_obj);

    /// \brief     release all the resources reserved for remote IP mapping
    /// \param[in] api_obj mapping object
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t release_remote_mapping_resources_(api_base *api_obj);

private:
    bool    is_local_;

    // handle or indices to NAT table
    uint32_t    overlay_ip_to_public_ip_nat_hdl_;
    uint32_t    overlay_ip_to_provider_ip_nat_hdl_;
    uint32_t    to_overlay_ip_nat_hdl_;

    // nexhop table index
    uint32_t    nh_idx_;

    // handles to entries in LOCAL_IP_MAPPING table
    handle_t    overlay_ip_hdl_;     // xlate idx to provider_ip & public_ip
                                     // are stored here
    handle_t    public_ip_hdl_;      // xlate idx to overlay_ip is stored here
    handle_t    provider_ip_hdl_;    // xlate idx to overlay_ip is stored here

    // mapping_hdl_ is used for both local and remote mappings
    handle_t    mapping_hdl_;        // MAPPINGS table handle is stored here
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MAPPING_IMPL_HPP__
