/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl.cc
 *
 * @brief   datapath implementation of route table
 */

#include "nic/hal/apollo/core/trace.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"
#include "nic/hal/apollo/api/route.hpp"
#include "nic/hal/apollo/api/impl/route_impl.hpp"
#include "nic/hal/apollo/api/impl/oci_impl_state.hpp"
#include "nic/hal/apollo/api/tep.hpp"
#include "nic/hal/apollo/api/impl/tep_impl.hpp"
#include "nic/hal/apollo/lpm/lpm.hpp"

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
    uint32_t      lpm_block_id;

    /**< allocate free lpm slab for this route table */
    if (route_table_impl_db()->route_table_idxr()->alloc(&lpm_block_id) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    lpm_root_addr_ =
        route_table_impl_db()->lpm_region_addr() +
            (route_table_impl_db()->lpm_table_size() * lpm_block_id);
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table_impl::release_resources(api_base *api_obj) {
    uint32_t    lpm_block_id;

    if (lpm_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        lpm_block_id =
            (lpm_root_addr_ - route_table_impl_db()->lpm_region_addr())/route_table_impl_db()->lpm_table_size();
        route_table_impl_db()->route_table_idxr()->free(lpm_block_id);
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
route_table_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t            ret;
    oci_route_table_t    *route_table_info;
    route_table_t        *rtable;
    oci_tep_key_t        tep_key;
    api::tep_entry       *tep;

    route_table_info = &obj_ctxt->api_params->route_table_info;
    SDK_ASSERT_RETURN((route_table_info->num_routes > 0),
                      sdk::SDK_RET_INVALID_ARG);

    /**< allocate memory for the library to build route table */
    rtable =
        (route_table_t *)
            SDK_MALLOC(OCI_MEM_ALLOC_ROUTE_TABLE,
                       sizeof(route_table_t) +
                           (route_table_info->num_routes * sizeof(route_t)));
    SDK_ASSERT_RETURN((rtable != NULL), sdk::SDK_RET_OOM);

    rtable->af = route_table_info->af;
    rtable->num_routes = route_table_info->num_routes;
    for (uint32_t i = 0; i < rtable->num_routes; i++) {
        rtable->routes[i].prefix = route_table_info->routes[i].prefix;
        tep_key.ip_addr = route_table_info->routes[i].nh_ip.addr.v4_addr;
        tep = tep_db()->tep_find(&tep_key);
        SDK_ASSERT(tep != NULL);
        rtable->routes[i].nhid = ((tep_impl *)(tep->impl()))->nh_id();
    }
    ret = lpm_tree_create(rtable, lpm_root_addr_,
                          route_table_impl_db()->lpm_table_size());
    if (ret != SDK_RET_OK) {
        OCI_TRACE_ERR("Failed to build LPM route table, err : %u", ret);
    }
    SDK_FREE(OCI_MEM_ALLOC_ROUTE_TABLE, rtable);
    return ret;
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
