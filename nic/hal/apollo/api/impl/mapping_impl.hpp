/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl.hpp
 *
 * @brief   mapping implementation in the p4/hw
 */
#if !defined (__MAPPING_IMPL_HPP__)
#define __MAPPING_IMPL_HPP__

#include "nic/hal/apollo/framework/api.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/framework/impl_base.hpp"
#include "nic/hal/apollo/include/api/oci_mapping.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"

namespace impl {

/**
 * @defgroup OCI_MAPPING_IMPL - mapping functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    mapping implementation
 */
class mapping_impl : public impl_base {
public:
    /**
     * @brief    factory method to allocate & initialize mapping impl instance
     * @param[in] oci_mapping    mapping information
     * @return    new instance of mapping or NULL, in case of error
     */
    static mapping_impl *factory(oci_mapping_t *oci_mapping);

    /**
     * @brief    release all the s/w state associated with the given mapping,
     *           if any, and free the memory
     * @param[in] mapping     mapping to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(mapping_impl *impl);

    /**
     * @brief    allocate/reserve h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *api_obj) override;

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
                                  oci_epoch_t epoch,
                                  api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) override;

private:
    /**< @brief    constructor */
    mapping_impl() {
        nat_idx1_ = nat_idx2_ = 0xFFFFFFFF;
        local_ip_mapping_data_idx1_ = 0xFFFFFFFF;
        local_ip_mapping_data_idx2_ = 0xFFFFFFFF;
        remote_vnic_mapping_rx_idx_ = 0xFFFFFFFF;
        remote_vnic_mapping_tx_idx_ = 0xFFFFFFFF;
    }

    /**< @brief    destructor */
    ~mapping_impl() {}

    /**
     * @brief     add necessary entries to NAT table
     * @param[in] mapping_info    IP mapping details
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t add_nat_entries_(oci_mapping_t *mapping_info);

    /**
     * @brief     add necessary entries to LOCAL_IP_MAPPING table
     * @param[in] vcn             VCN of this IP
     * @param[in] mapping_info    IP mapping details
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t add_local_ip_mapping_entries_(vcn_entry *vcn,
                                            oci_mapping_t *mapping_info);

    /**
     * @brief     add necessary entries to REMOTE_VNIC_MAPPING_RX table
     * @param[in] vcn             VCN of this IP
     * @param[in] subnet          subnet of this IP
     * @param[in] mapping_info    IP mapping details
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t add_remote_vnic_mapping_rx_entries_(vcn_entry *vcn,
                                                  subnet_entry *subnet,
                                                  oci_mapping_t *mapping_info);

    /**
     * @brief     add necessary entries to REMOTE_VNIC_MAPPING_TX table
     * @param[in] vcn             VCN of this IP
     * @param[in] subnet          subnet of this IP
     * @param[in] mapping_info    IP mapping details
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t add_remote_vnic_mapping_tx_entries_(vcn_entry *vcn,
                                                  oci_mapping_t *mapping_info);
private:
    bool        is_local_;
    uint32_t    nat_idx1_, nat_idx2_;
    uint32_t    local_ip_mapping_data_idx1_;
    uint32_t    local_ip_mapping_data_idx2_;
    uint32_t    remote_vnic_mapping_rx_idx_;
    uint32_t    remote_vnic_mapping_tx_idx_;
};

/** @} */    // end of OCI_MAPPING_IMPL

}    // namespace impl

#endif    /** __MAPPING_IMPL_HPP__ */
