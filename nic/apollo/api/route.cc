/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route.cc
 *
 * @brief   route table handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_ROUTE_TABLE - route table functionality
 * @ingroup OCI_ROUTE
 * @{
 */

/**< @brief    constructor */
route_table::route_table() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

/**
 * @brief    factory method to allocate & initialize a route table instance
 * @param[in] oci_route_table    route table information
 * @return    new instance of route table or NULL, in case of error
 */
route_table *
route_table::factory(oci_route_table_t *oci_route_table) {
    route_table    *rtable;

    /**< create route table instance with defaults, if any */
    rtable = route_table_db()->route_table_alloc();
    if (rtable) {
        new (rtable) route_table();
        rtable->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_ROUTE_TABLE,
                                           oci_route_table);
        if (rtable->impl_ == NULL) {
            route_table::destroy(rtable);
            return NULL;
        }
    }
    return rtable;
}

/**< @brief    destructor */
route_table::~route_table() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w resources associated with this object,
 *           if any, and free the memory
 * @param[in] rtable     route table to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
route_table::destroy(route_table *rtable) {
    if (rtable->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_ROUTE_TABLE, rtable->impl_);
    }
    rtable->release_resources();
    rtable->~route_table();
    route_table_db()->route_table_free(rtable);
}

/**
 * @brief     initialize route table instance with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::init_config(api_ctxt_t *api_ctxt) {
    oci_route_table_t    *oci_route_table;
    
    oci_route_table = &api_ctxt->api_params->route_table_info;
    memcpy(&this->key_, &oci_route_table->key, sizeof(oci_route_table_key_t));
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::reserve_resources(void) {
    return impl_->reserve_resources(this);
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::program_config(obj_ctxt_t *obj_ctxt) {
    sdk_ret_t            ret;
 
    ret = reserve_resources();
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    OCI_TRACE_DEBUG("Programming route table %u", key_.id);
    return impl_->program_hw(this, obj_ctxt);
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::release_resources(void) {
    return impl_->release_resources(this);
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    //return impl_->update_hw();
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
route_table::activate_config(oci_epoch_t epoch, api_op_t api_op,
                            obj_ctxt_t *obj_ctxt) {
    OCI_TRACE_DEBUG("Activating route table %u config", key_.id);
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] orig_obj    old version of the object being swapped out
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add route table to database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::add_to_db(void) {
    return route_table_db()->route_table_ht()->insert_with_key(&key_, this,
                                                               &ht_ctxt_);
}

/**
 * @brief delete route table from database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
route_table::del_from_db(void) {
    route_table_db()->route_table_ht()->remove(&key_);
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
route_table::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_ROUTE_TABLE, this);
}

/** @} */    // end of OCI_ROUTE_TABLE

}    // namespace api

/** @} */ // end of OCI_ROUTE_TABLE_API
