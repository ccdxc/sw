/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic_impl.hpp
 *
 * @brief   VNIC implementation in the p4/hw
 */
#if !defined (__VNIC_IMPL_HPP__)
#define __VNIC_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/pds_vnic.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/api/policy.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {
namespace impl {

/**
 * @defgroup PDS_VNIC_IMPL - vnic functionality
 * @ingroup PDS_VNIC
 * @{
 */

/**
 * @brief    VNIC implementation
 */
class vnic_impl : public impl_base {
public:
    /**
     * @brief    factory method to allocate & initialize vnic impl instance
     * @param[in] pds_vnic    vnic information
     * @return    new instance of vnic or NULL, in case of error
     */
    static vnic_impl *factory(pds_vnic_spec_t *pds_vnic);

    /**
     * @brief    release all the s/w state associated with the given vnic,
     *           if any, and free the memory
     * @param[in] impl     vnic impl instance to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(vnic_impl *impl);

    /**
     * @brief    allocate/reserve h/w resources for this object
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_hw(api_base *api_obj,
                                  pds_epoch_t epoch,
                                  api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief read spec, statistics and status from hw tables
     * @param[in]  key  pointer to vnic key
     * @param[out] info pointer to vnic info
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t read_hw(pds_vnic_key_t *key, pds_vnic_info_t *info);

    uint16_t hw_id(void) { return hw_id_; }

private:
    /**< @brief    constructor */
    vnic_impl() {
        hw_id_ = 0xFFFF;
        vnic_by_slot_hash_idx_ = 0xFFFF;
    }

    /**< @brief    destructor */
    ~vnic_impl() {}


    /**
     * @brief    program LOCAL_VNIC_BY_VLAN_TX table and activate the epoch in
     *           the Tx direction
     * @param[in] api_op         api operation
     * @param[in] api_obj        vnic entry object
     * @param[in] epoch          epoch being activated
     * @param[in] vcn            vcn entry
     * @param[in] subnet         subnet entry
     * @param[in] spec           vnic configuration
     * @param[in] v4_route_table IPv4 routing table entry
     * @param[in] v6_route_table IPv6 routing table entry
     * @param[in] v4_policy      egress IPv4 security policy
     * @param[in] v6_policy      egress IPv6 security policy
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t activate_vnic_by_vlan_tx_table_(api_op_t api_op,
                                              api_base *api_obj,
                                              pds_epoch_t epoch,
                                              vcn_entry *vcn,
                                              subnet_entry *subnet,
                                              pds_vnic_spec_t *spec,
                                              route_table *v4_route_table,
                                              route_table *v6_route_table,
                                              policy *v4_policy,
                                              policy *v6_policy);
    /**
     * @brief    program LOCAL_VNIC_BY_SLOT_RX table and activate the epoch in
     *           the Rx direction
     * @param[in] api_op         api operation
     * @param[in] api_obj        vnic entry object
     * @param[in] epoch          epoch being activated
     * @param[in] vcn            vcn entry
     * @param[in] subnet         subnet entry
     * @param[in] spec           vnic configuration
     * @param[in] v4_policy      ingress IPv4 security policy
     * @param[in] v6_policy      ingress IPv6 security policy
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t activate_vnic_by_slot_rx_table_(api_op_t api_op,
                                              api_base *api_obj,
                                              pds_epoch_t epoch, vcn_entry *vcn,
                                              subnet_entry *subnet,
                                              pds_vnic_spec_t *spec,
                                              policy *v4_policy,
                                              policy *v6_policy);
    /**
     * @brief Read statistics from the hw table and fill the vnic stats
     * @param[in] tx_stats Transmit statistics table data
     * @param[in] rx_stats Receive statistics table data
     */
    void fill_vnic_stats_(vnic_tx_stats_actiondata_t *tx_stats,
                         vnic_rx_stats_actiondata_t *rx_stats,
                         pds_vnic_stats_t *stats);
private:
    /**< P4 datapath specific state */
    uint16_t          hw_id_;      /**< hardware id */
    // TODO: if we have remove(key) API, we don't need to store this
    uint16_t          vnic_by_slot_hash_idx_;
} __PACK__;

/** @} */    // end of PDS_VNIC_IMPL

}    // namespace impl
}    // namespace api

#endif    /** __VNIC_IMPL_HPP__ */
