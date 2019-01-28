/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep.cc
 *
 * @brief   Tunnel EndPoint (TEP) entry handling
 */

#include <stdio.h>
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

using sdk::lib::ht;

namespace api {

/**
 * @defgroup OCI_TEP_ENTRY - tep entry functionality
 * @ingroup OCI_TEP
 * @{
 */

/**< @brief    constructor */
tep_entry::tep_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

/**
 * @brief    factory method to allocate and initialize a tep entry
 * @param[in] oci_tep    tep information
 * @return    new instance of tep or NULL, in case of error
 */
tep_entry *
tep_entry::factory(oci_tep_t *oci_tep) {
    tep_entry *tep;

    /**< create tep entry with defaults, if any */
    tep = tep_db()->tep_alloc();
    if (tep) {
        new (tep) tep_entry();
        tep->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_TEP, oci_tep);
        if (tep->impl_ == NULL) {
            tep_entry::destroy(tep);
            tep_db()->tep_free(tep);
            return NULL;
        }
    }
    return tep;

}

/**< @brief    destructor */
tep_entry::~tep_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w resources associated with this object,
 *           if any, and free the memory
 * @param[in] tep     tep to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
tep_entry::destroy(tep_entry *tep) {
    if (tep->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_TEP, tep->impl_);
    }
    tep->release_resources_();
    tep->~tep_entry();
}

/**
 * @brief     initialize tep entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_tep_t *oci_tep = &api_ctxt->api_params->tep_info;

    memcpy(&this->key_, &oci_tep->key, sizeof(oci_tep_key_t));
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: 1. this should ideally go to impl class
//       2. we don't need an indexer here if we can use directmap here to
//          "reserve" an index
sdk_ret_t
tep_entry::reserve_resources_(void) {
    return impl_->reserve_resources(this);
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
// TODO: we should simply be generating ARP request in the substrate in this API
//       and come back and do this h/w programming later, but until that control
//       plane & PMD APIs are ready, we will directly write to hw with fixed MAC
sdk_ret_t
tep_entry::program_config(obj_ctxt_t *obj_ctxt) {
    sdk_ret_t    ret;

    ret = reserve_resources_();
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    OCI_TRACE_DEBUG("Programming TEP %s", ipv4addr2str(key_.ip_addr));
    return impl_->program_hw(this, obj_ctxt);
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_entry::release_resources_(void) {
    return impl_->release_resources(this);
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
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
tep_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
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
tep_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    OCI_TRACE_DEBUG("Created TEP %s", ipv4addr2str(key_.ip_addr));
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
tep_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add tep to database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_entry::add_to_db(void) {
    return tep_db()->tep_ht()->insert_with_key(&key_, this,
                                               &ht_ctxt_);
}

/**
 * @brief delete tep from database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_entry::del_from_db(void) {
    tep_db()->tep_ht()->remove(&key_);
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
tep_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_TEP, this);
}

/** @} */    // end of OCI_TEP_ENTRY

}    // namespace api
