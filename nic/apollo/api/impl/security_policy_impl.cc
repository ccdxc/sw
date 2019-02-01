/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_impl.cc
 *
 * @brief   datapath implementation of security policy
 */

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/security_policy.hpp"
#include "nic/apollo/api/impl/security_policy_impl.hpp"
#include "nic/apollo/api/impl/oci_impl_state.hpp"

namespace impl {

/**
 * @defgroup OCI_SECURITY_POLICY_IMPL - security policy datapath implementation
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

/**
 * @brief    factory method to allocate & initialize security policy impl instance
 * @param[in] oci_security_policy    security policy information
 * @return    new instance of security policy or NULL, in case of error
 */
security_policy_impl *
security_policy_impl::factory(oci_security_policy_t *oci_security_policy) {
    security_policy_impl    *impl;

    // TODO: move to slab later
    impl = (security_policy_impl *)
               SDK_CALLOC(SDK_MEM_ALLOC_OCI_SECURITY_POLICY_IMPL,
                          sizeof(security_policy_impl));
    new (impl) security_policy_impl();
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given
 *           security policy impl instance, if any, and free the memory
 * @param[in] impl security policy impl instance to be freed
 */
void
security_policy_impl::destroy(security_policy_impl *impl) {
    impl->~security_policy_impl();
    SDK_FREE(SDK_MEM_ALLOC_OCI_SECURITY_POLICY_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl::reserve_resources(api_base *api_obj) {
    uint32_t    policy_block_id;

    /**< allocate available block for this security policy */
    if (security_policy_impl_db()->security_policy_idxr()->alloc(&policy_block_id) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    security_policy_root_addr_ =
        security_policy_impl_db()->security_policy_region_addr() +
            (security_policy_impl_db()->security_policy_table_size() * policy_block_id);
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl::release_resources(api_base *api_obj) {
    uint32_t    policy_block_id;

    if (security_policy_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        policy_block_id =
            (security_policy_root_addr_ -
                 security_policy_impl_db()->security_policy_region_addr())/
                security_policy_impl_db()->security_policy_table_size();
        security_policy_impl_db()->security_policy_idxr()->free(policy_block_id);
    }
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] curr_obj    cloned and updated version of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                            obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    activate the epoch in the dataplane by programming stage 0
 *           tables, if any
 * @param[in] epoch       epoch being activated
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl::activate_hw(api_base *api_obj, oci_epoch_t epoch,
                                  api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case api::API_OP_CREATE:
        /**< for security policy create, there is no stage 0 programming */
        break;

    case api::API_OP_UPDATE:
        /**
         * need to walk all vnics AND subnets to see which of them are using
         * this policy table and then walk all the vnics that are part of the
         * vcns and subnets and write new epoch data
         */
        return SDK_RET_ERR;
        break;

    case api::API_OP_DELETE:
        /**< same as update but every entry written will have invalid bit set */
        return SDK_RET_ERR;
        break;

    default:
        break;
    }
    return SDK_RET_OK;
}

/** @} */    // end of OCI_SECURITY_POLICY_IMPL

}    // namespace impl
