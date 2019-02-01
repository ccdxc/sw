/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_impl.hpp
 *
 * @brief   security policy implementation in the p4/hw
 */
#if !defined (__SECURITY_POLICY_IMPL_HPP__)
#define __SECURITY_POLICY_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/oci_security_policy.hpp"

namespace impl {

/**
 * @defgroup OCI_SECURITY_POLICY_IMPL - security policy functionality
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

/**
 * @brief    security policy implementation
 */
class security_policy_impl : public impl_base {
public:
    /**
     * @brief    factory method to allocate & initialize
     *           security policy impl instance
     * @param[in] oci_security_policy    security policy information
     * @return    new instance of security policy or NULL, in case of error
     */
    static security_policy_impl *factory(oci_security_policy_t *oci_security_policy);

    /**
     * @brief    release all the s/w state associated with the given
     *           security policy instance, if any, and free the memory
     * @param[in] impl security policy impl instance to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(security_policy_impl *impl);

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

    mem_addr_t security_policy_root_addr(void) {
        return security_policy_root_addr_;
    }

private:
    /**< @brief    constructor */
    security_policy_impl() {
        security_policy_root_addr_ = 0xFFFFFFFFFFFFFFFFUL;
    }

    /**< @brief    destructor */
    ~security_policy_impl() {}

private:
    /**< P4 datapath specific state */
    mem_addr_t    security_policy_root_addr_;   /**< policy root node address */
} __PACK__;

/** @} */    // end of OCI_SECURITY_POLICY_IMPL

}    // namespace impl

#endif    /** __SECURITY_POLICY_IMPL_HPP__ */
