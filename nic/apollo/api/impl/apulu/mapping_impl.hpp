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
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

using sdk::table::handle_t;
using sdk::table::sdk_table_api_params_t;

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
    /// \param[in] api_obj  object for which resources are being reserved
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj, api_base *orig_obj,
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
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] api_obj  API object being cleaned up
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

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
        rxdma_mapping_hdl_ = handle_t::null();
        rxdma_mapping_public_ip_hdl_ = handle_t::null();
        rxdma_local_mapping_tag_idx_ = PDS_IMPL_RSVD_TAG_HW_ID;
        rxdma_mapping_tag_idx_ = PDS_IMPL_RSVD_TAG_HW_ID;
        nexthop_type_ = NEXTHOP_TYPE_MAX;
        nexthop_id_ = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
        num_class_id_ = 0;
        for (uint32_t i = 0; i < PDS_MAX_TAGS_PER_MAPPING; i++) {
            class_id_[i] = PDS_IMPL_RSVD_MAPPING_CLASS_ID;
        }
    }

    /// \brief destructor
    ~mapping_impl() {}

    /// \brief     reserve necessary mapping table entries for local mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC impl instance of this mapping
    /// \param[in] vnic    vnic of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_public_ip_mapping_resources_(mapping_entry *mapping,
                                                   vpc_impl *vpc,
                                                   vnic_entry *vnic,
                                                   pds_mapping_spec_t *spec);

    /// \brief     reserve necessary NAT table entries for local mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] vnic    vnic of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_nat_resources_(mapping_entry *mapping, vnic_entry *vnic,
                                     pds_mapping_spec_t *spec);

    /// \brief     reserve necessary NAT table entries for local mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC impl instance of this mapping
    /// \param[in] vnic    vnic of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_public_ip_rxdma_mapping_resources_(mapping_entry *mapping,
                  vpc_impl *vpc, vnic_entry *vnic, pds_mapping_spec_t *spec);

    /// \brief     reserve all resources needed for local mapping's
    ///            public IP
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC impl instance of this mapping
    /// \param[in] vnic    vnic of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_public_ip_resources_(mapping_entry *mapping,
                                           vpc_impl *vpc, vnic_entry *vnic,
                                           pds_mapping_spec_t *spec);

    /// \brief     allocate all classids corresponding to the tags
    ///            configured on this mapping
    ///            in the rxdma to support tag derivation
    /// \param[in] vpc     VPC impl instance for this mapping
    /// \param[in] local   true if the mapping is local
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec    IP mapping configuration details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t allocate_tag_classes_(vpc_impl *vpc, bool local,
                                    mapping_entry *mapping,
                                    pds_mapping_spec_t *spec);

    /// \brief     reserve all resources needed for a mapping in the rxdma to
    ///            support tag derivation
    /// \param[in] vpc     VPC entry of this mapping
    /// \param[in] local   true if the mapping is local
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec    IP mapping configuration details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_rxdma_mapping_tag_resources_(vpc_entry *vpc, bool local,
                                                   mapping_entry *mapping,
                                                   pds_mapping_spec_t *spec);

    /// \brief     reserve necessary resources for local mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC of this mapping
    /// \param[in] vnic    vnic instance of this local mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_local_mapping_resources_(mapping_entry *mapping,
                                               vpc_entry *vpc,
                                               vnic_entry *vnic,
                                               pds_mapping_spec_t *spec);

    /// \brief     reserve necessary entries in remote mapping table
    ///            during the creation of remote mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] vpc     VPC of this mapping
    /// \param[in] subnet  subnet of this mapping
    /// \param[in] spec    IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_remote_mapping_resources_(mapping_entry *mapping,
                                                vpc_entry *vpc,
                                                subnet_entry *subnet,
                                                pds_mapping_spec_t *spec);

    /// \brief     add necessary entries to NAT table
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec    mapping configurtion
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_nat_entries_(mapping_entry *mapping,
                               pds_mapping_spec_t *spec);

    /// \brief add rxdma MAPPING table entry for (local/remote) overlay IP
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] spec    mapping configurtion
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_overlay_ip_rxdma_mapping_entry_(vpc_impl *vpc,
                                                  pds_mapping_spec_t *spec);

    /// \brief deactivate rxdma MAPPING table entry for (local/remote)
    ///        overlay IP
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] mapping   mapping instance
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t deactivate_overlay_ip_rxdma_mapping_entry_(vpc_impl *vpc,
                  mapping_entry *mapping);

    /// \brief add rxdma MAPPING table entry for public IP
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] spec    mapping configurtion
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_public_ip_rxdma_mapping_entry_(vpc_impl *vpc,
                                                 pds_mapping_spec_t *spec);

    /// \brief deactivate rxdma MAPPING table entry for public IP
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] mapping   mapping instance
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t deactivate_public_ip_rxdma_mapping_entry_(vpc_impl *vpc,
                                                        mapping_entry *mapping);

    /// \brief     fill key and data information for local mappping's public IP
    ///            P4 table entries
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[in] vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] local_mapping_key pointer to the key structure of
    ///            LOCAL_MAPPING table
    /// \param[in] local_mapping_data pointer to the data structure of
    ///            LOCAL_MAPPING table
    /// \param[in] local_mapping_public_ip_hdl LOCAL_MPPING table entry handle
    /// \param[in] local_mapping_tbl_params table params for LOCAL_MAPPING table
    /// \param[in] mapping_key pointer to the key structure of MAPPING table
    /// \param[in] mapping_data pointer to the data structure of MAPPING table
    /// \param[in] mapping_hdl MAPPING table entry handle
    /// \param[in] mapping_tbl_params  table params for MAPPING table
    /// \param[in] rxdma_mapping_key pointer to the key structure of rxdma MAPPING table
    /// \param[in] rxdma_mapping_data pointer to the data structure of rxdma MAPPING table
    /// \param[in] rxdma_mapping_public_hdl rxdma MAPPING table entry handle for
    ///                                     public IP
    /// \param[in] rxdma_mapping_tbl_params  table params for rxdma MAPPING table
    /// \param[in] spec IP mapping details
    void fill_public_ip_mapping_key_data_(
             vpc_impl *vpc, subnet_entry *subnet, vnic_entry *vnic,
             vnic_impl *vnic_impl_obj, local_mapping_swkey_t *local_mapping_key,
             local_mapping_appdata_t *local_mapping_data,
             sdk::table::handle_t local_mapping_overlay_ip_hdl,
             sdk_table_api_params_t *local_mapping_tbl_params,
             mapping_swkey_t *mapping_key, mapping_appdata_t *mapping_data,
             sdk::table::handle_t mapping_hdl,
             sdk_table_api_params_t *mapping_tbl_params,
             rxdma_mapping_swkey_t *rxdma_mapping_key,
             rxdma_mapping_appdata_t *rxdma_mapping_data,
             sdk::table::handle_t rxdma_mapping_public_ip_hdl,
             sdk_table_api_params_t *rxdma_mapping_tbl_params,
             pds_mapping_spec_t *spec);

    /// \brief     add necessary entries for local mapping's public IP
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[in] vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_public_ip_entries_(vpc_impl *vpc, subnet_entry *subnet,
                                     vnic_entry *vnic, vnic_impl *vnic_impl_obj,
                                     mapping_entry *mapping,
                                     pds_mapping_spec_t *spec);

    /// \brief     fill key and data information for local overlay IP mappping's
    ///            P4 table entries
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[in] vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] local_mapping_key pointer to the key structure of
    ///            LOCAL_MAPPING table
    /// \param[in] local_mapping_data pointer to the data structure of
    ///            LOCAL_MAPPING table
    /// \param[in] local_mapping_overlay_ip_hdl LOCAL_MPPING table entry handle
    /// \param[in] local_mapping_tbl_params table params for LOCAL_MAPPING table
    /// \param[in] mapping_key pointer to the key structure of MAPPING table
    /// \param[in] mapping_data pointer to the data structure of MAPPING table
    /// \param[in] mapping_hdl MAPPING table entry handle
    /// \param[in] mapping_tbl_params  table params for MAPPING table
    /// \param[in] rxdma_mapping_key pointer to the key structure of rxdma MAPPING table
    /// \param[in] rxdma_mapping_data pointer to the data structure of rxdma MAPPING table
    /// \param[in] rxdma_mapping_hdl rxdma MAPPING table entry handle
    /// \param[in] rxdma_mapping_tbl_params  table params for rxdma MAPPING table
    /// \param[in] spec IP mapping details
    void fill_local_overlay_ip_mapping_key_data_(
             vpc_impl *vpc, subnet_entry *subnet, vnic_entry *vnic,
             vnic_impl *vnic_impl_obj, local_mapping_swkey_t *local_mapping_key,
             local_mapping_appdata_t *local_mapping_data,
             sdk::table::handle_t local_mapping_overlay_ip_hdl,
             sdk_table_api_params_t *local_mapping_tbl_params,
             mapping_swkey_t *mapping_key, mapping_appdata_t *mapping_data,
             sdk::table::handle_t mapping_hdl,
             sdk_table_api_params_t *mapping_tbl_params,
             rxdma_mapping_swkey_t *rxdma_mapping_key,
             rxdma_mapping_appdata_t *rxdma_mapping_data,
             sdk::table::handle_t rxdma_mapping_hdl,
             sdk_table_api_params_t *rxdma_mapping_tbl_params,
             pds_mapping_spec_t *spec);

    /// \brief     add necessary entries for overlay IP of local mapping
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[n]  vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_overlay_ip_mapping_entries_(vpc_impl *vpc,
                                              subnet_entry *subnet,
                                              vnic_entry *vnic,
                                              vnic_impl *vnic_impl_obj,
                                              pds_mapping_spec_t *spec);

    /// \brief     program MAPPING_TAG and LOCAL_MAPPING_TAG tables for
    ///            local mapping
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t program_local_mapping_tag_entries_(void);

    /// \brief     add necessary entries for local mappings
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_local_mapping_entries_(vpc_entry *vpc, subnet_entry *subnet,
                                         mapping_entry *mapping,
                                         pds_mapping_spec_t *spec);

    /// \brief     fill key and data information for remote IP/MAC mapping's
    ///            P4 table entries
    /// \param[in] vpc vpc corresponding to this mapping
    /// \param[in] subnet subnet imple instance of this mapping
    /// \param[in] mapping_key pointer to the key structure of MAPPING table
    /// \param[in] mapping_data pointer to the data structure of MAPPING table
    /// \param[in] mapping_hdl MAPPING table entry handle
    /// \param[in] mapping_tbl_params  table params for MAPPING table
    /// \param[in] rxdma_mapping_key pointer to the key structure of rxdma MAPPING table
    /// \param[in] rxdma_mapping_data pointer to the data structure of rxdma MAPPING table
    /// \param[in] rxdma_mapping_hdl rxdma MAPPING table entry handle
    /// \param[in] rxdma_mapping_tbl_params  table params for rxdma MAPPING table
    /// \param[in] spec IP mapping details
    sdk_ret_t fill_remote_mapping_key_data_(
                  vpc_entry *vpc, subnet_impl *subnet,
                  mapping_swkey_t *mapping_key, mapping_appdata_t *mapping_data,
                  sdk::table::handle_t mapping_hdl,
                  sdk_table_api_params_t *mapping_tbl_params,
                  rxdma_mapping_swkey_t *rxdma_mapping_key,
                  rxdma_mapping_appdata_t *rxdma_mapping_data,
                  sdk::table::handle_t rxdma_mapping_hdl,
                  sdk_table_api_params_t *rxdma_mapping_tbl_params,
                  pds_mapping_spec_t *spec);

    /// \brief     add necessary entries for remote mappings
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] mapping mapping object being processed
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_remote_mapping_entries_(vpc_entry *vpc, subnet_entry *subnet,
                                          mapping_entry *mapping,
                                          pds_mapping_spec_t *spec);

    /// \brief     add necessary entries for local mapping's public IP
    /// \param[in] vpc  VPC impl instance corresponding to this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[n]  vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] new_mapping cloned mapping object being processed
    /// \param[in] orig_mapping original/current mapping object
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t upd_public_ip_entries_(vpc_impl *vpc, subnet_entry *subnet,
                                     vnic_entry *vnic, vnic_impl *vnic_impl_obj,
                                     mapping_entry *new_mapping,
                                     mapping_entry *orig_mapping,
                                     pds_mapping_spec_t *spec);

    /// \brief     update necessary entries overlay IP for local mappings
    /// \param[in] vpc  VPC impl instance of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[n]  vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] new_mapping cloned mapping object being processed
    /// \param[in] orig_mapping original/current mapping object
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t upd_overlay_ip_mapping_entries_(vpc_impl *vpc,
                                              subnet_entry *subnet,
                                              vnic_entry *vnic,
                                              vnic_impl *vnic_impl_obj,
                                              mapping_entry *new_mapping,
                                              mapping_entry *orig_mapping,
                                              pds_mapping_spec_t *spec);

    /// \brief     handle tag(s) update of local mapping
    /// \param[in] vpc  VPC impl instance of this mapping
    /// \param[in] new_mapping cloned mapping object being processed
    /// \param[in] orig_mapping original/current mapping object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t handle_tag_update_(vpc_impl *vpc,
                                 mapping_entry *new_mapping,
                                 mapping_entry *orig_mapping,
                                 api_obj_ctxt_t *obj_ctxt,
                                 pds_mapping_spec_t *spec);

    /// \brief    handle public IP attribute updates of local mapping
    /// \param[in] vpc  VPC impl instance of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] vnic vnic this mapping is associated with
    /// \param[n]  vnic_impl_obj vnic impl instance of vnic corresponding to
    ///                          mapping
    /// \param[in] new_mapping cloned mapping object being processed
    /// \param[in] orig_mapping original/current mapping object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \param[in] spec IP mapping details
    /// \param[in,out] upd_pub_ip_mappings if upd_pub_ip_mappings is set to
    ///                false by this method, it means all public IP mapping
    ///                related updates are taken care of and no need to update
    ///                the entries further. this out parameter indicates whether
    ///                to update those entries or not in the caller
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t handle_public_ip_update_(vpc_impl *vpc, subnet_entry *subnet,
                                       vnic_entry *vnic,
                                       vnic_impl *vnic_impl_obj,
                                       mapping_entry *new_mapping,
                                       mapping_entry *orig_mapping,
                                       api_obj_ctxt_t *obj_ctxt,
                                       pds_mapping_spec_t *spec,
                                       bool& upd_pub_ip_mappings);

    /// \brief     update necessary entries for local mappings
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] new_mapping cloned mapping object being processed
    /// \param[in] orig_mapping original/current mapping object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \param[in] spec IP mapping details
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_local_mapping_update_(vpc_entry *vpc,
                                             subnet_entry *subnet,
                                             mapping_entry *new_mapping,
                                             mapping_entry *orig_mapping,
                                             api_obj_ctxt_t *obj_ctxt,
                                             pds_mapping_spec_t *spec);

    /// \brief     program MAPPING_TAG table entry for the remote mapping
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t program_remote_mapping_tag_tables_(void);

    /// \brief     update necessary entries for remote mappings
    /// \param[in] vpc  VPC of this mapping
    /// \param[in] subnet subnet of this mapping
    /// \param[in] new_mapping cloned mapping object being processed
    /// \param[in] orig_mapping original/current mapping object
    /// \param[in] spec IP mapping details
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_remote_mapping_update_(vpc_entry *vpc,
                                              subnet_entry *subnet,
                                              mapping_entry *new_mapping,
                                              mapping_entry *orig_mapping,
                                              pds_mapping_spec_t *spec,
                                              api_obj_ctxt_t *obj_ctxt);

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

    /// \brief     program and activate mapping related tables during update
    ///            by enabling stage0 tables corresponding to the new epoch
    /// \param[in] epoch       epoch being activated
    /// \param[in] mapping     mapping instance
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \param[in] spec        mapping configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_update_(pds_epoch_t epoch,
                                       mapping_entry *new_mapping,
                                       mapping_entry *orig_mapping,
                                       api_obj_ctxt_t *obj_ctxt,
                                       pds_mapping_spec_t *spec);

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
    // indices into rxdma LOCAL_MAPPING_TAG and MAPPING_TAG tables
    uint32_t    rxdma_local_mapping_tag_idx_;
    uint32_t    rxdma_mapping_tag_idx_;
    // handles for RxDMA MAPPING table
    handle_t    rxdma_mapping_hdl_;
    handle_t    rxdma_mapping_public_ip_hdl_;
    // number of class id(s) allocated for this mapping
    uint32_t    num_class_id_;
    // class id(s) of this mapping
    uint32_t    class_id_[PDS_MAX_TAGS_PER_MAPPING];
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MAPPING_IMPL_HPP__
