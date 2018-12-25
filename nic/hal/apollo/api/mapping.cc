/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping.cc
 *
 * @brief   This file deals with mapping api handling
 */

#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/mapping.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_MAPPING_API - first level of mapping API handling
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @defgroup OCI_MAPPING_ENTRY - mapping entry functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**< @brief    constructor */
mapping_entry::mapping_entry() {
    stateless_ = true;
}

/**
 * @brief    factory method to allocate and initialize a mapping entry
 * @param[in] oci_mapping    mapping information
 * @return    new instance of mapping or NULL, in case of error
 */
mapping_entry *
mapping_entry::factory(oci_mapping_t *oci_mapping) {
    mapping_entry *mapping;

    /**< create mapping entry with defaults, if any */
    mapping = mapping_db()->mapping_alloc();
    if (mapping) {
        new (mapping) mapping_entry();
    }
    return mapping;
}

/**< @brief    destructor */
mapping_entry::~mapping_entry() {
}

/**
 * @brief    release all the s/w & h/w resources associated with this object
 *           and free the memory
 * @param[in] mapping     mapping to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       imp->cleanup_hw() before calling this
 */
void
mapping_entry::destroy(mapping_entry *mapping) {
    mapping->free_resources_();
    mapping->~mapping_entry();
}

/**
 * @brief     initialize mapping entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_entry::init_config(api_ctxt_t *api_ctxt) {
    // TODO: come back and fill this up
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: this should ideally go to impl class
sdk_ret_t
mapping_entry::alloc_resources_(void) {
    // TODO: need hbmhash->reserve() given a key
    // TODO: impl->alloc_resources_();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any, during creation of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_entry::program_config(obj_ctxt_t *obj_ctxt) {
    //alloc_resources_();
    // impl->program_hw();
    return sdk::SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_entry::free_resources_(void) {
    /**
      * we are not tracking indices in s/w for mappings, and h/w entries are
      * cleaned up in cleanup_config already, so this is a no-op
      */
    return sdk::SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * NOTE:     we shouldn't release h/w entries here !!
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    // impl->cleanup_hw();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // update operation is not supported on mapping
    // impl->update_hw();
    return sdk::SDK_RET_OK;
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
mapping_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 h/w programming for mapping , so nothing to activate
    return sdk::SDK_RET_OK;
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] orig_obj    old version of the object being swapped out
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    /**< mappings are not added to s/w db, so its a no-op */
    return sdk::SDK_RET_OK;
}

/**
 * @brief add mapping to database
 *
 * @param[in] mapping mapping
 */
sdk_ret_t
mapping_entry::add_to_db(void) {
    /**< mappings are not added to s/w db, so its a no-op */
    return sdk::SDK_RET_OK;
}

/**
 * @brief delete mapping from database
 * @param[in] mapping_key mapping key
 */
sdk_ret_t
mapping_entry::del_from_db(void) {
    /**< mappings are not added to s/w db, so its a no-op */
    return sdk::SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
mapping_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_MAPPING, this);
}

/** @} */    // end of OCI_MAPPING_ENTRY

/**
 * @defgroup OCI_MAPPING_STATE - mapping database functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    constructor
 */
mapping_state::mapping_state() {
    mapping_slab_ = slab::factory("mapping", OCI_SLAB_MAPPING,
                                  sizeof(mapping_entry),
                                  16, true, true, NULL);
    SDK_ASSERT(mapping_slab_ != NULL);
}

/**
 * @brief    destructor
 */
mapping_state::~mapping_state() {
    slab::destroy(mapping_slab_);
}

/**
 * @brief     allocate mapping instance
 * @return    pointer to the allocated mapping , NULL if no memory
 */
mapping_entry *
mapping_state::mapping_alloc(void) {
    return ((mapping_entry *)mapping_slab_->alloc());
}

/**
 * @brief      free mapping instance back to slab
 * @param[in]  mapping   pointer to the allocated mapping
 */
void
mapping_state::mapping_free(mapping_entry *mapping) {
    mapping_slab_->free(mapping);
}

#if 0
/**
 * @brief        lookup mapping in database with given key
 * @param[in]    mapping_key mapping key
 * @return       pointer to the mapping instance found or NULL
 */
mapping_entry *
mapping_state::mapping_find(oci_mapping_key_t *mapping_key) const {
    // TODO: if we really need to support this, do
    //       1. impl->find(key)
    //       2. decode to oci_mapping_key_t
    //       3. return the info
    return NULL;
}
#endif

/** @} */    // end of OCI_MAPPING_STATE

/**
 * @brief create mapping
 *
 * @param[in] mapping mapping information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_mapping_create (_In_ oci_mapping_t *mapping)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_MAPPING;
        api_ctxt.api_params->mapping_info = *mapping;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete mapping
 *
 * @param[in] mapping_key mapping key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_mapping_delete (_In_ oci_mapping_key_t *mapping_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_MAPPING;
        api_ctxt.api_params->mapping_key = *mapping_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */    // end of OCI_MAPPING_API

}    // namespace api
