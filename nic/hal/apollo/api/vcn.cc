/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.cc
 *
 * @brief   This file deals with vcn api handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_VCN_ENTRY - vcn entry functionality
 * @ingroup OCI_VCN
 * @{
 */

/**< @brief    constructor */
vcn_entry::vcn_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
}

/**
 * @brief    factory method to allocate and initialize a vcn entry
 * @param[in] oci_vcn    vcn information
 * @return    new instance of vcn or NULL, in case of error
 */
vcn_entry *
vcn_entry::factory(oci_vcn_t *oci_vcn) {
    vcn_entry *vcn;

    /**< create vcn entry with defaults, if any */
    vcn = vcn_db()->vcn_alloc();
    if (vcn) {
        new (vcn) vcn_entry();
    }
    return vcn;
}

/**< @brief    destructor */
vcn_entry::~vcn_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w resources associated with this object
 *           and free the memory
 * @param[in] vcn     vcn to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       imp->cleanup_hw() before calling this
 */
void
vcn_entry::destroy(vcn_entry *vcn) {
    vcn->free_resources_();
    vcn->~vcn_entry();
}

/**
 * @brief     initialize vcn entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_vcn_t *oci_vcn = &api_ctxt->api_params->vcn_info;

    memcpy(&this->key_, &oci_vcn->key, sizeof(oci_vcn_key_t));
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::alloc_resources_(void) {
    if (vcn_db()->vcn_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any, during creation of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::program_config(obj_ctxt_t *obj_ctxt) {
    /**
     * there is no h/w programming for VCN config but a h/w id is needed so we
     * can use while programming vnics, routes etc.
     */
    return alloc_resources_();
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::free_resources_(void) {
    if (hw_id_ != 0xFF) {
        vcn_db()->vcn_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * NOTE:     we shouldn't release h/w entries here !!
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    /**< there is no h/w programming for VCN config, so nothing to cleanup */
    return SDK_RET_OK;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    /**< there is no h/w programming for VCN config, so nothing to update */
    return SDK_RET_OK;
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
vcn_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    /**< there is no h/w programming for VCN config, so nothing to activate */
    return SDK_RET_OK;
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] orig_obj    old version of the object being swapped out
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    /**< nothing to update on vcn, so no updates supported */
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add vcn to database
 *
 * @param[in] vcn vcn
 */
sdk_ret_t
vcn_entry::add_to_db(void) {
    return vcn_db()->vcn_ht()->insert_with_key(&key_, this,
                                               &ht_ctxt_);
}

/**
 * @brief delete vcn from database
 *
 * @param[in] vcn_key vcn key
 */
sdk_ret_t
vcn_entry::del_from_db(void) {
    vcn_db()->vcn_ht()->remove(&key_);
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
vcn_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_VCN, this);
}

/** @} */    // end of OCI_VCN_ENTRY

/**
 * @defgroup OCI_VCN_STATE - vcn database functionality
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief    constructor
 */
vcn_state::vcn_state() {
    // TODO: need to tune multi-threading related params later
    vcn_ht_ = ht::factory(OCI_MAX_VCN >> 1,
                          vcn_entry::vcn_key_func_get,
                          vcn_entry::vcn_hash_func_compute,
                          vcn_entry::vcn_key_func_compare);
    SDK_ASSERT(vcn_ht_!= NULL);
    vcn_idxr_ = indexer::factory(OCI_MAX_VCN);
    SDK_ASSERT(vcn_idxr_ != NULL);
    vcn_slab_ = slab::factory("vcn", OCI_SLAB_ID_VCN, sizeof(vcn_entry),
                              16, true, true, NULL);
    SDK_ASSERT(vcn_slab_ != NULL);
}

/**
 * @brief    destructor
 */
vcn_state::~vcn_state() {
    ht::destroy(vcn_ht_);
    indexer::destroy(vcn_idxr_);
    slab::destroy(vcn_slab_);
}

/**
 * @brief     allocate vcn instance
 * @return    pointer to the allocated vcn , NULL if no memory
 */
vcn_entry *
vcn_state::vcn_alloc(void) {
    return ((vcn_entry *)vcn_slab_->alloc());
}

/**
 * @brief      free vcn instance back to slab
 * @param[in]  vcn   pointer to the allocated vcn
 */
void
vcn_state::vcn_free(vcn_entry *vcn) {
    vcn_slab_->free(vcn);
}

/**
 * @brief        lookup vcn in database with given key
 * @param[in]    vcn_key vcn key
 * @return       pointer to the vcn instance found or NULL
 */
vcn_entry *
vcn_state::vcn_find(oci_vcn_key_t *vcn_key) const {
    return (vcn_entry *)(vcn_ht_->lookup(vcn_key));
}

/** @} */    // end of OCI_VCN_STATE

}    // namespace api

/**
 * @defgroup OCI_VCN_API - first level of vcn API handling
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief create vcn
 *
 * @param[in] vcn vcn information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vcn_create (_In_ oci_vcn_t *vcn)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_VCN;
        api_ctxt.api_params->vcn_info = *vcn;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete vcn
 *
 * @param[in] vcn_key vcn key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vcn_delete (_In_ oci_vcn_key_t *vcn_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_VCN;
        api_ctxt.api_params->vcn_key = *vcn_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */    // end of OCI_VCN_API
