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
#include "gen/p4gen/apulu/include/p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"

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

    /// \brief    clone this object by copying all the h/w resources
    ///           allocated for this object into new object and return the
    ///           cloned object
    /// \return    cloned impl instance
    virtual impl_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] impl impl instance to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(vnic_impl *impl);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] api_obj API object for which resources are being reserved
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj,
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
    /// \NOTE      this method is called when an object is in the
    ///            dependent/puppet object list
    virtual sdk_ret_t reprogram_hw(api_base *api_obj, api_op_t api_op) override;

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] api_obj  API object holding this resource
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] curr_obj current version of the unmodified object
    /// \param[in] prev_obj previous version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

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

    /// \brief     re-activate config in the hardware stage 0 tables relevant to
    ///            this object, if any, this reactivation must be based on
    ///            existing state and any of the state present in the dirty
    ///            object list
    ///            (like cloned objects etc.) only and not directly on db
    ///            objects
    /// \param[in] api_obj API object being activated
    /// \param[in] api_op  API operation
    /// \return    #SDK_RET_OK on success, failure status code on error
    /// \NOTE      this method is called when an object is in the
    ///            dependent/puppet object list
    virtual sdk_ret_t reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                                    api_op_t api_op) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to VNIC key
    /// \param[out] info pointer to VNIC info
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief     return vnic's h/w id
    /// \return    h/w id assigned to the vnic
    uint16_t hw_id(void) { return hw_id_; }

    /// \brief     return nexthop idx allocated for this vnic
    /// \return    nexthop index corresponding to this vnic
    uint16_t nh_idx(void) { return nh_idx_; }

private:
    /// \brief constructor
    vnic_impl() {
        hw_id_ = 0xFFFF;
        nh_idx_ = 0xFFFF;
        local_mapping_hdl_ = handle_t::null();
        mapping_hdl_ = handle_t::null();
    }

    /// \brief destructor
    ~vnic_impl() {}

    /// \brief      populate rxdma vnic info table entry's policy tree root
    ///             address
    /// \param[in]  vnic_info_data    vnic info data to be programmed
    /// \param[in]  idx               policy LPM root entry index
    /// \param[in]  addr              policy tree root address
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t populate_rxdma_vnic_info_policy_root_(
                  vnic_info_rxdma_actiondata_t *vnic_info_data,
                  uint32_t idx, mem_addr_t addr);

    /// \brief      program vnic info tables in rxdma and txdma
    /// \param[in]  vpc       vpc of the vnic
    /// \param[in]  subnet    subnet of the vnic
    /// \param[in]  spec      vnic configuration
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t program_vnic_info_(vpc_entry *vpc, subnet_entry *subnet,
                                 pds_vnic_spec_t *spec);

    /// \brief     add an entry to LOCAL_MAPPING table
    /// \param[in] epoch epoch being activated
    /// \param[in] vpc vpc of this vnic
    /// \param[in] subnet subnet of this vnic
    /// \param[in] vnic  VNIC obj being programmed
    /// \param[in] spec  VNIC configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_local_mapping_entry_(pds_epoch_t epoch, vpc_entry *vpc,
                                       subnet_entry *subnet, vnic_entry *vnic,
                                       pds_vnic_spec_t *spec);

    /// \brief     add an entry to VLAN table
    /// \param[in] epoch epoch being activated
    /// \param[in] vpc vpc of this vnic
    /// \param[in] subnet subnet of this vnic
    /// \param[in] vnic  VNIC obj being programmed
    /// \param[in] spec  VNIC configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_vlan_entry_(pds_epoch_t epoch, vpc_entry *vpc,
                              subnet_entry *subnet, vnic_entry *vnic,
                              pds_vnic_spec_t *spec);

    /// \brief     add an entry to MAPPING table
    /// \param[in] epoch epoch being activated
    /// \param[in] vpc vpc of this vnic
    /// \param[in] subnet subnet of this vnic
    /// \param[in] vnic  VNIC obj being programmed
    /// \param[in] spec  VNIC configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_mapping_entry_(pds_epoch_t epoch, vpc_entry *vpc,
                                 subnet_entry *subnet, vnic_entry *vnic,
                                 pds_vnic_spec_t *spec);

    /// \brief     program VNIC related tables during VNIC create by enabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] vnic  VNIC obj being programmed
    /// \param[in] spec  VNIC configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_create_(pds_epoch_t epoch, vnic_entry *vnic,
                               pds_vnic_spec_t *spec);

    /// \brief     program VNIC related tables during VNIC delete by disabling
    ///            stage0 tables corresponding to the new epoch
    /// \param[in] epoch epoch being activated
    /// \param[in] vnic  VNIC obj being programmed
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_delete_(pds_epoch_t epoch, vnic_entry *vnic);

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
    uint16_t hw_id_;                   ///< hardware id
    uint16_t nh_idx_;                  ///< nexthop table index for this vnic
    ///< handle for LOCAL_MAPPING and MAPPING table entries (note that handles
    ///< are valid only in a transaction)
    handle_t local_mapping_hdl_;
    handle_t mapping_hdl_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __VNIC_IMPL_HPP__
