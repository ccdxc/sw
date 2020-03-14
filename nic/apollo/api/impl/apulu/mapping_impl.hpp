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
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

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

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(mapping_impl *impl);

    /// \brief     instantiate a mapping impl object based on current state
    ///            (sw and/or hw) given its key
    /// \param[in] key mapping entry's key
    /// \param[in] mapping mapping entry's API object
    /// \return    new instance of mapping implementation object or NULL
    static mapping_impl *build(pds_mapping_key_t *key, mapping_entry *mapping);

    /// \brief     free a stateless entry's temporary s/w only resources like
    ///            memory etc., for a stateless entry calling destroy() will
    ///            remove resources from h/w, which can't be done during
    ///            ADD/UPDATE etc. operations esp. when object is constructed
    ///            on the fly
    /// \param[in] impl mapping to be freed
    static void soft_delete(mapping_impl *impl);

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
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] api_obj  API object being cleaned up
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

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

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj API object
    /// \param[in]  key  pointer to mapping key
    /// \param[out] info pointer to mapping info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief  return the nexthop type this mapping is pointing to
    /// \return type of the nexthop
    uint8_t nexthop_type(void) const { return nexthop_type_; }

    /// \brief  return the nexthop table index corresponding to this mapping
    /// \return nexthop index
    uint32_t nexthop_id(void) const { return nexthop_id_; }

private:
    /// \brief constructor
    mapping_impl() {
        to_public_ip_nat_idx_ = PDS_IMPL_RSVD_NAT_HW_ID;
        to_overlay_ip_nat_idx_ = PDS_IMPL_RSVD_NAT_HW_ID;
        local_mapping_overlay_ip_hdl_ = handle_t::null();
        local_mapping_public_ip_hdl_ = handle_t::null();
        mapping_hdl_ = handle_t::null();
        mapping_public_ip_hdl_ = handle_t::null();
        nexthop_type_ = NEXTHOP_TYPE_MAX;
        nexthop_id_ = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
    }

    /// \brief destructor
    ~mapping_impl() {}

    /// \brief     add necessary entries to NAT table
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec    mapping configurtion
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_nat_entries_(mapping_entry *mapping,
                               pds_mapping_spec_t *spec);

    /// \brief     reserve necessary entries in local mapping table
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_local_mapping_resources_(mapping_entry *mapping,
                                               vpc_entry *vpc,
                                               pds_mapping_spec_t *spec);

    /// \brief     reserve necessary entries in remote mapping table
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC of this mapping
    /// \param[in] subnet  subnet of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_remote_mapping_resources_(mapping_entry *mapping,
                                                vpc_entry *vpc,
                                                subnet_entry *subnet,
                                                pds_mapping_spec_t *spec);

    /// \brief     add necessary entries for local mappings
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_local_mapping_entries_(vpc_entry *vpc,
                                         pds_mapping_spec_t *spec);

    /// \brief     add necessary entries for remote mappings
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_remote_mapping_entries_(vpc_entry *vpc,
                                          subnet_entry *subnet,
                                          pds_mapping_spec_t *spec);

    /// \brief     program and activate mapping related tables during create
    ///            by enabling stage0 tables corresponding to the new epoch
    /// \param[in] epoch       epoch being activated
    /// \param[in] mapping     mapping instance
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \param[in] spec        mapping configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_create_(pds_epoch_t epoch,
                                       mapping_entry *mapping,
                                       api_obj_ctxt_t *obj_ctxt,
                                       pds_mapping_spec_t *spec);

    /// \brief     program and activate mapping related tables during delete
    ///            by enabling stage0 tables corresponding to the new epoch
    /// \param[in] epoch       epoch being activated
    /// \param[in] mapping     mapping instance
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_delete_(pds_epoch_t epoch,
                                       mapping_entry *mapping);

    /// \brief         read the configured values from the local mapping tables
    /// \param[in]     vpc  pointer to the vpc entry
    /// \param[in]     subnet    subnet of the mapping, if mapping is L2 or
    //                           else NULL
    /// \param[in/out] info pointer to the info
    /// \return        SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_local_mapping_(vpc_entry *vpc, subnet_entry *subnet,
                                  pds_mapping_info_t *info);

    /// \brief         read the configured values from the local mapping tables
    /// \param[in]     vpc       pointer to the vpc entry
    /// \param[in]     subnet    subnet of the mapping, if mapping is L2 or
    ///                          else NULL
    /// \param[in/out] info pointer to the info
    /// \return        SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_remote_mapping_(vpc_entry *vpc, subnet_entry *subnet,
                                   pds_mapping_info_t *info);

    /// \brief     release all the resources reserved for local mapping
    /// \param[in] api_obj mapping object
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t release_local_mapping_resources_(api_base *api_obj);

    /// \brief     release all the resources reserved for remote mapping
    /// \param[in] api_obj mapping object
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t release_remote_mapping_resources_(api_base *api_obj);

    /// \brief     deactivate L2 mapping entry for a given MAC
    /// \param[in] subnet    subnet id of the mapping
    /// \param[in] mac_addr    MAC address of the mapping
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t deactivate_l2_mapping_entry_(pds_obj_key_t subnet,
                                           mac_addr_t mac_addr);

    /// \brief     deactivate mapping entry for a given overlay or public ip
    /// \param[in] vpc    vpc id of the mapping
    /// \param[in] ip     pointer to the (overlay/public) IP address
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t deactivate_ip_mapping_entry_(pds_obj_key_t vpc, ip_addr_t *ip);

    /// \brief     deactivate local mapping and mapping entries for a given
    ///            overlay or public ip
    /// \param[in] vpc    vpc id of the mapping
    /// \param[in] ip     pointer to the (overlay/public) IP address
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t deactivate_ip_local_mapping_entry_(pds_obj_key_t vpc,
                                                 ip_addr_t *ip);

private:
    uint32_t    vnic_hw_id_;
    uint32_t    vpc_hw_id_;
    uint32_t    subnet_hw_id_;
    uint8_t     nexthop_type_;
    uint32_t    nexthop_id_;

    // handles or indices for NAT table
    uint32_t    to_public_ip_nat_idx_;
    uint32_t    to_overlay_ip_nat_idx_;

    // handles for LOCAL_MAPPING table
    handle_t    local_mapping_overlay_ip_hdl_;
    handle_t    local_mapping_public_ip_hdl_;
    // handles for MAPPING table
    handle_t    mapping_hdl_;    // could be L2 or IP handle
    handle_t    mapping_public_ip_hdl_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MAPPING_IMPL_HPP__
