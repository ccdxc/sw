/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.cc
 *
 * @brief   vcn entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

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
    vcn->release_resources();
    vcn->~vcn_entry();
    vcn_db()->vcn_free(vcn);
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
vcn_entry::reserve_resources(void) {
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
    oci_vcn_t *oci_vcn = &obj_ctxt->api_params->vcn_info;
    OCI_TRACE_DEBUG("Programming vcn %u, type %u, pfx %s", key_.id,
                    oci_vcn->type, ippfx2str(&oci_vcn->pfx));
    return reserve_resources();
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::release_resources(void) {
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
    /**< there is no h/w programming for vcn config, so nothing to activate */
    OCI_TRACE_DEBUG("Created vcn %u", key_.id);
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
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_entry::add_to_db(void) {
    return vcn_db()->vcn_ht()->insert_with_key(&key_, this,
                                               &ht_ctxt_);
}

/**
 * @brief delete vcn from database
 * @return   SDK_RET_OK on success, failure status code on error
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

}    // namespace api
