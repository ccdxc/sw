/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl.cc
 *
 * @brief   datapath implementation of mapping
 */

#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/mapping.hpp"
#include "nic/hal/apollo/api/impl/mapping_impl.hpp"

namespace impl {

/**
 * @defgroup OCI_MAPPING_IMPL - mapping entry datapath implementation
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    factory method to allocate & initialize mapping impl instance
 * @param[in] oci_mapping    mapping information
 * @return    new instance of mapping or NULL, in case of error
 */
mapping_impl *
mapping_impl::factory(oci_mapping_t *oci_mapping) {
    return NULL;
}

/**
 * @brief    release all the s/w state associated with the given mapping,
 *           if any, and free the memory
 * @param[in] mapping     mapping to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
mapping_impl::destroy(mapping_impl *impl) {
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::alloc_resources(api_base *api_obj) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::free_resources(api_base *api_obj) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
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
mapping_impl::activate_hw(api_base *api_obj, oci_epoch_t epoch,
                          api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/** @} */    // end of OCI_MAPPING_IMPL

}    // namespace impl
