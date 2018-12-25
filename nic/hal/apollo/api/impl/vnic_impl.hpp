/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic_impl.hpp
 *
 * @brief   VNIC implementation in the p4/hw
 */
#if !defined (__VNIC_IMPL_HPP__)
#define __VNIC_IMPL_HPP__

#include "nic/hal/apollo/framework/api.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/framework/impl_base.hpp"
#include "nic/hal/apollo/include/api/oci_vnic.hpp"

namespace impl {

/**
 * @defgroup OCI_VNIC_IMPL - vnic functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    VNIC implementation
 */
class vnic_impl : public impl_base {
public:
    /**
     * @brief    factory method to allocate & initialize vnic impl instance
     * @param[in] oci_vnic    vnic information
     * @return    new instance of vnic or NULL, in case of error
     */
    static vnic_impl *factory(oci_vnic_t *oci_vnic);

    /**
     * @brief    release all the s/w state associated with the given vnic,
     *           if any, and free the memory
     * @param[in] vnic     vnic to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(vnic_impl *impl);

    /**
     * @brief    allocate/reserve h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t alloc_resources(api_base *api_obj) override;

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t free_resources(api_base *api_obj) override;

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
    vnic_impl() {
        hw_id_ = 0xFFFF;
        vnic_by_slot_hash_idx_ = 0xFFFF;
    }

    /**< @brief    destructor */
    ~vnic_impl() {}

private:
    /**< P4 datapath specific state */
    uint16_t          hw_id_;      /**< hardware id */
    // TODO: if we have remove(key) API, we don't need to store this
    uint16_t          vnic_by_slot_hash_idx_;
} __PACK__;

/** @} */    // end of OCI_VNIC_IMPL

}    // namespace impl

#endif    /** __VNIC_IMPL_HPP__ */
