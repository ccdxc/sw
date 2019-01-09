/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl.cc
 *
 * @brief   datapath implementation of route table
 */

#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"
#include "nic/hal/apollo/api/route.hpp"
#include "nic/hal/apollo/api/impl/route_impl.hpp"
#include "nic/hal/apollo/api/impl/oci_impl_state.hpp"

namespace impl {

/**
 * @defgroup OCI_ROUTE_TABLE_IMPL - route table datapath implementation
 * @ingroup OCI_ROUTE
 * @{
 */

/**
 * @brief    factory method to allocate & initialize route table impl instance
 * @param[in] oci_route_table    route table information
 * @return    new instance of route table or NULL, in case of error
 */
route_table_impl *
route_table_impl::factory(oci_route_table_t *oci_route_table) {
    route_table_impl    *impl;

    // TODO: move to slab later
    impl = (route_table_impl *)SDK_CALLOC(SDK_MEM_ALLOC_OCI_ROUTE_TABLE_IMPL,
                                          sizeof(route_table_impl));
    new (impl) route_table_impl();
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given
 *           route table instance, if any, and free the memory
 * @param[in] impl route table impl instance to be freed
 */
void
route_table_impl::destroy(route_table_impl *impl) {
    impl->~route_table_impl();
    SDK_FREE(SDK_MEM_ALLOC_OCI_ROUTE_TABLE_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table_impl::reserve_resources(api_base *api_obj) {
    //uint32_t    lpm_block_id;

#if 0
    /**< allocate free lpm slab for this route table */
    if (route_table_impl_db()->route_table_idxr()->alloc(&lpm_block_id) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    lpm_root_addr_ = lpm_mem_base_ + (lpm_table_sz_ * lpm_block_id);
#endif
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table_impl::release_resources(api_base *api_obj) {
#if 0
    if (lpm_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        lpm_block_id = (lpm_root_addr_ - lpm_mem_base_)/lpm_table_sz_;
        route_table_impl_db()->route_table_idxr()->free(lpm_block_id);
    }
#endif
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
// TODO: undo stuff if something goes wrong here !!
sdk_ret_t
route_table_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
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
route_table_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
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
route_table_impl::activate_hw(api_base *api_obj, oci_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case api::API_OP_CREATE:
        /**< for route table create, there is no stage 0 programming */
        break;

    case api::API_OP_UPDATE:
        /**
         * need to walk all vnics AND subnets to see which of them are using
         * this routing table and then walk all the vnics that are part of the
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

/** @} */    // end of OCI_ROUTE_TABLE_IMPL

}    // namespace impl
