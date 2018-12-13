/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.cc
 *
 * @brief   This file deals with OCI vcn API handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/oci_state.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_VCN_ENTRY - vcn entry functionality
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief    cleanup state maintained for this vcn including any hw entries we
 *           are holding on to
 */
void
vcn_entry::cleanup(void) {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
    // TODO: check if indices are valid before calling tbl mgmt. APIs
    vcn_db()->vcn_idxr()->free(hw_id_);
}

/**
 * @brief    release all the s/w state associate with the given vcn, if any,
 *           and free the memory
 * @param[in] vcn     vcn to be freed
 * NOTE: h/w entries should have been cleaned up (by calling cleanup_hw()
 * before calling this
 */
void
vcn_entry::destroy(vcn_entry *vcn) {
    vcn->cleanup();
    vcn->~vcn_entry();
}

/**
 * @brief     initialize vcn entry with the given config
 * @param[in] oci_vcn    vcn information
 * @return    SDK_RET_OK on success, failure status code on error
 *
 * NOTE:     allocate all h/w resources (i.e., table indices as well here, we
 *           can always release them in abort phase if something goes wrong
 */
sdk_ret_t
vcn_entry::init(oci_vcn_t *oci_vcn) {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_SHARED);
    memcpy(&this->key_, &oci_vcn->key, sizeof(oci_vcn_key_t));
    this->ht_ctxt_.reset();
    if (vcn_db()->vcn_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
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

/**
 * @brief     handle a vcn create by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::process_create(api_ctxt_t *api_ctxt) {
    return init(&api_ctxt->vcn_info);
}

/**
 * @brief     handle a vcn update by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::process_update(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_ERR;
}

/**
 * @brief     handle a vcn delete by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::process_delete(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_ERR;
}

/**
 * @brief     handle a vcn get by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::process_get(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::program_hw(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::cleanup_hw(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::update_hw(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    activate the epoch in the dataplane
 * @param[in] api_op      api operation
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::activate_epoch(api_op_t api_op, api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] old         old version of the object being swapped out
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::update_db(api_base *old_obj, api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#if 0
/**
 * @brief    commit() is invokved during commit phase of the API processing and
 *           is not expected to fail as all required resources are already
 *           allocated by now. Based on the API operation, this API is expected
 *           to process either create/retrieve/update/delete. If any temporary
 *           state was stashed in the api_ctxt while processing this API, it
 *           should be freed here
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 *
 * NOTE:     commit() is not expected to fail
 */
sdk_ret_t
vcn_entry::commit(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief     abort() is invoked during abort phase of the API processing and is
 *            not expected to fail. During this phase, all associated resources
 *            must be freed and global DBs need to be restored back to their
 *            original state and any transient state stashed in api_ctxt while
 *            processing this API should also be freed here
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::abort(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

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
    return sdk::SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
vcn_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_VCN, this);
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
    vcn_slab_ = slab::factory("vcn", OCI_SLAB_VCN, sizeof(vcn_entry),
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
 * @brief        lookup vcn in database with given key
 * @param[in]    vcn_key vcn key
 * @return       pointer to the vcn instance found or NULL
 */
vcn_entry *
vcn_state::vcn_find(oci_vcn_key_t *vcn_key) {
    return (vcn_entry *)(vcn_ht_->lookup(vcn_key));
}

/** @} */    // end of OCI_VCN_STATE

#if 0
/**
 * @brief handle vcn create message
 *
 * @param[in] vcn vcn information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_db::vcn_create(_In_ oci_vcn_t *oci_vcn) {
    vcn_t *vcn;

    if ((vcn = vcn_alloc_init(oci_vcn)) == NULL) {
        return sdk::SDK_RET_OOM;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief handle vcn delete API
 *
 * @param[in] vcn_key vcn key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_db::vcn_delete(_In_ oci_vcn_key_t *vcn_key) {
    vcn_t *vcn;

    if ((vcn = vcn_del_from_db(vcn_key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    vcn_delete(vcn);

    return sdk::SDK_RET_OK;
}

/**
 * @brief Uninitialize and free internal vcn structure
 *
 * @param[in] vcn vcn
 */
void
vcn_db::vcn_delete(_In_ vcn_t *vcn) {
    if (vcn) {
        vcn_cleanup(vcn);
        vcn_free(vcn);
    }
}
#endif

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
    api_ctxt_t    api_ctxt;   // TODO: get this from slab
    sdk_ret_t     rv;

    memset(&api_ctxt, 0, sizeof(api_ctxt));
    api_ctxt.api_op = api::API_OP_CREATE;
    api_ctxt.obj_id = api::OBJ_ID_VCN;
    api_ctxt.vcn_info = *vcn;
    rv = api::g_api_engine.process_api(&api_ctxt);
    return rv;
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
    api_ctxt_t    api_ctxt;   // TODO: get this from slab ??
    sdk_ret_t     rv;

    memset(&api_ctxt, 0, sizeof(api_ctxt));
    api_ctxt.api_op = api::API_OP_DELETE;
    api_ctxt.obj_id = api::OBJ_ID_VCN;
    api_ctxt.vcn_key = *vcn_key;
    rv = api::g_api_engine.process_api(&api_ctxt);
    return rv;
}

/** @} */    // end of OCI_VCN_API
