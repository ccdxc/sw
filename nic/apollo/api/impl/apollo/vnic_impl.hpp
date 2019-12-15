//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// VNIC implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __VNIC_IMPL_HPP__
#define __VNIC_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/api/policy.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

using sdk::table::handle_t;

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL - VNIC functionality
/// \ingroup PDS_VNIC
/// @{

/// \brief VNIC implementation
class vnic_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize VNIC impl instance
    /// \param[in] spec VNIC specification
    /// \return    new instance of VNIC or NULL, in case of error
    static vnic_impl *factory(pds_vnic_spec_t *spec);

    /// \brief     release all the s/w state associated with the given VNIC,
    ///            if any, and free the memory
    /// \param[in] impl VNIC impl instance to be freed
    /// \NOTE      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this)
    static void destroy(vnic_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \param[in] api_obj API object holding this resource
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \param[in] api_obj API object holding the resources
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] api_obj API object holding this resource
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     re-program config in the hardware
    ///            re-program all hardware tables relevant to this object
    ///            except stage 0 table(s), if any and this reprogramming must
    ///            be based on existing state and any of the state present in
    ///            the dirty object list (like clone objects etc.)
    /// \param[in] api_obj API object being activated
    /// \param[in] api_op  API operation
    /// \return    #SDK_RET_OK on success, failure status code on error
    /// \NOTE      this method is called when an object is in the dependent/puppet
    ///            object list
    virtual sdk_ret_t reprogram_hw(api_base *api_obj, api_op_t api_op) override;

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] api_obj  API object holding this resource
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] curr_obj current version of the unmodified object
    /// \param[in] prev_obj previous version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming stage 0
    ///            tables, if any
    /// \param[in] api_obj  (cloned) API api object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     re-activate config in the hardware stage 0 tables relevant to
    ///            this object, if any, this reactivation must be based on existing
    ///            state and any of the state present in the dirty object list
    ///            (like clone objects etc.) only and not directly on db objects
    /// \param[in] api_obj API object being activated
    /// \param[in] api_op  API operation
    /// \return    #SDK_RET_OK on success, failure status code on error
    /// \NOTE      this method is called when an object is in the dependent/puppet
    ///            object list
    virtual sdk_ret_t reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                                    api_op_t api_op) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to VNIC key
    /// \param[out] info pointer to VNIC info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    uint16_t hw_id(void) { return hw_id_; }

private:
    /// \brief constructor
    vnic_impl() {
        hw_id_ = 0xFFFF;
    }

    /// \brief destructor
    ~vnic_impl() {}

    /// \brief     program LOCAL_VNIC_BY_VLAN_TX table and activate the epoch in
    ///            the Tx direction
    /// \param[in] api_op         API operation
    /// \param[in] api_obj        VNIC entry object
    /// \param[in] epoch          epoch being activated
    /// \param[in] vpc            VPC entry
    /// \param[in] subnet         subnet entry
    /// \param[in] spec           VNIC configuration
    /// \param[in] v4_route_table IPv4 routing table entry
    /// \param[in] v6_route_table IPv6 routing table entry
    /// \param[in] v4_policy      egress IPv4 security policy
    /// \param[in] v6_policy      egress IPv6 security policy
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_vnic_by_vlan_tx_table_create_(pds_epoch_t epoch,
                                                     vpc_entry *vpc,
                                                     pds_vnic_spec_t *spec,
                                                     vnic_entry *vnic,
                                                     route_table *v4_route_table,
                                                     route_table *v6_route_table,
                                                     policy *v4_policy,
                                                     policy *v6_policy);

    /// \brief     program LOCAL_VNIC_BY_SLOT_RX table and activate the epoch in
    ///            the Rx direction
    /// \param[in] epoch     epoch being activated
    /// \param[in] vpc       VPC entry
    /// \param[in] spec      VNIC configuration
    /// \param[in] vnic      VNIC obj being programmed
    /// \param[in] v4_policy ingress IPv4 security policy
    /// \param[in] v6_policy ingress IPv6 security policy
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_vnic_by_slot_rx_table_create_(pds_epoch_t epoch,
                                                     vpc_entry *vpc,
                                                     pds_vnic_spec_t *spec,
                                                     vnic_entry *vnic,
                                                     policy *v4_policy,
                                                     policy *v6_policy);

    /// \brief     program VNIC related tables during VNIC create by enabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] vnic  VNIC obj being programmed
    /// \param[in] spec  VNIC configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_vnic_create_(pds_epoch_t epoch, vnic_entry *vnic,
                                    pds_vnic_spec_t *spec);

    /// \brief     program VNIC related tables during VNIC delete by disabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] vnic  VNIC obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_vnic_delete_(pds_epoch_t epoch, vnic_entry *vnic);

    /// \brief      fill the VNIC spec
    /// \param[out] spec specification
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_vnic_spec_t *spec);

    /// \brief      fill the VNIC stats
    /// \param[out] stats statistics
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_stats_(pds_vnic_stats_t *stats);

    /// \brief      fill the VNIC status
    /// \param[out] status status
    void fill_status_(pds_vnic_status_t *status);

private:
    // P4 datapath specific state
    uint16_t hw_id_;                           ///< hardware id
    handle_t local_vnic_by_vlan_tx_handle_;
    handle_t local_vnic_by_slot_rx_handle_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __VNIC_IMPL_HPP__
