/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   This file deals with vnic api handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/core/trace.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vnic.hpp"
#include "nic/hal/apollo/api/utils.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_VNIC_ENTRY - vnic entry functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**< @brief    constructor */
vnic_entry::vnic_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

/**
 * @brief    factory method to allocate and initialize a vnic entry
 * @param[in] oci_vnic    vnic information
 * @return    new instance of vnic or NULL, in case of error
 */
vnic_entry *
vnic_entry::factory(oci_vnic_t *oci_vnic) {
    vnic_entry *vnic;

    /**< create vnic entry with defaults, if any */
    vnic = vnic_db()->vnic_alloc();
    if (vnic) {
        new (vnic) vnic_entry();
        vnic->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_VNIC, oci_vnic);
        if (vnic->impl_ == NULL) {
            vnic_entry::destroy(vnic);
            vnic_db()->vnic_free(vnic);
            return NULL;
        }
    }
    return vnic;

}

/**< @brief    destructor */
vnic_entry::~vnic_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w resources associated with this object,
 *           if any, and free the memory
 * @param[in] vnic     vnic to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
vnic_entry::destroy(vnic_entry *vnic) {
    if (vnic->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_VNIC, vnic->impl_);
    }
    vnic->release_resources_();
    vnic->~vnic_entry();
}

/**
 * @brief     initialize vnic entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_vnic_t *oci_vnic = &api_ctxt->api_params->vnic_info;

    memcpy(&this->key_, &oci_vnic->key, sizeof(oci_vnic_key_t));
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// 1. we don't need an indexer here if we can use directmap here to
//    "reserve" an index
sdk_ret_t
vnic_entry::reserve_resources_(void) {
    return impl_->reserve_resources(this);
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::program_config(obj_ctxt_t *obj_ctxt) {
    sdk_ret_t     ret;
    oci_vnic_t    *oci_vnic = &obj_ctxt->api_params->vnic_info;

    ret = reserve_resources_();
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    OCI_TRACE_DEBUG("Programming vnic %u, vcn %u, subnet %u, mac %s, vlan %u, "
                    "slot %u", key_.id, oci_vnic->vcn.id, oci_vnic->subnet.id,
                    macaddr2str(oci_vnic->mac_addr), oci_vnic->wire_vlan,
                    oci_vnic->slot);
    return impl_->program_hw(this, obj_ctxt);
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::release_resources_(void) {
    return impl_->release_resources(this);
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
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
vnic_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
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
vnic_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    OCI_TRACE_DEBUG("Activating vnic %u config", key_.id);
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
vnic_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add vnic to database
 *
 * @param[in] vnic vnic
 */
sdk_ret_t
vnic_entry::add_to_db(void) {
    return vnic_db()->vnic_ht()->insert_with_key(&key_, this,
                                                 &ht_ctxt_);
}

/**
 * @brief delete vnic from database
 *
 * @param[in] vnic_key vnic key
 */
sdk_ret_t
vnic_entry::del_from_db(void) {
    vnic_db()->vnic_ht()->remove(&key_);
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
vnic_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_VNIC, this);
}

/** @} */    // end of OCI_VNIC_ENTRY

/**
 * @defgroup OCI_VNIC_STATE - vnic database functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    constructor
 */
vnic_state::vnic_state() {
    // TODO: need to tune multi-threading related params later
    vnic_ht_ = ht::factory(OCI_MAX_VNIC >> 2,
                           vnic_entry::vnic_key_func_get,
                           vnic_entry::vnic_hash_func_compute,
                           vnic_entry::vnic_key_func_compare);
    SDK_ASSERT(vnic_ht_ != NULL);

    vnic_slab_ = slab::factory("vnic", OCI_SLAB_ID_VNIC, sizeof(vnic_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(vnic_slab_ != NULL);
}

/**
 * @brief    destructor
 */
vnic_state::~vnic_state() {
    ht::destroy(vnic_ht_);
    slab::destroy(vnic_slab_);
}

/**
 * @brief     allocate vnic instance
 * @return    pointer to the allocated vnic , NULL if no memory
 */
vnic_entry *
vnic_state::vnic_alloc(void) {
    return ((vnic_entry *)vnic_slab_->alloc());
}

/**
 * @brief      free vnic instance back to slab
 * @param[in]  vnic   pointer to the allocated vnic
 */
void
vnic_state::vnic_free(vnic_entry *vnic) {
    vnic_slab_->free(vnic);
}

/**
 * @brief        lookup vnic in database with given key
 * @param[in]    vnic_key vnic key
 * @return       pointer to the vnic instance found or NULL
 */
vnic_entry *
vnic_state::vnic_find(oci_vnic_key_t *vnic_key) const {
    return (vnic_entry *)(vnic_ht_->lookup(vnic_key));
}

/** @} */    // end of OCI_VNIC_STATE

}    // namespace api

/**
 * @brief Create VNIC
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_create (_In_ oci_vnic_t *vnic)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        api_ctxt.api_params->vnic_info = *vnic;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_key VNIC key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_delete (_In_ oci_vnic_key_t *vnic_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        api_ctxt.api_params->vnic_key = *vnic_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of OCI_VNIC_API
