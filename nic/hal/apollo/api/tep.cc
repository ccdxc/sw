/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep.cc
 *
 * @brief   This file deals with Tunnel EndPoint (TEP) api handling
 */

#include <stdio.h>
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/tep.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

using sdk::lib::ht;
using sdk::lib::indexer;

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
    hw_id_ = 0xFFFF;
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
 *       cleanup_hw() before calling this
 */
void
tep_entry::destroy(tep_entry *tep) {
    tep->free_resources_();
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
    return sdk::SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: 1. this should ideally go to impl class
//       2. we don't need an indexer here if we can use directmap here to
//          "reserve" an index
sdk_ret_t
tep_entry::alloc_resources_(void) {
    /**
     * allocate hw id for this tep, tep specific index tables in the p4
     * datapath are indexed by this
     */
    if (tep_db()->tep_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
// TODO: this should ideally go to impl class (other than subnet_db lookup
// TODO: we should simply be generating ARP request in the substrate in this API
//       and come back and do this h/w programming later, but until that control
//       plane & PMD APIs are ready, we will directly write to hw with fixed MAC
#define tep_tx_udp_action    action_u.tep_tx_udp_tep_tx
sdk_ret_t
tep_entry::program_hw(obj_ctxt_t *obj_ctxt) {
    sdk_ret_t              ret;
    oci_tep_t              *tep_info;
    tep_tx_actiondata_t    tep_tx_data = { 0 };

    tep_info = &obj_ctxt->api_params->tep_info;
    tep_tx_data.action_id = TEP_TX_UDP_TEP_TX_ID;
    // TODO: take care of byte ordering issue, if any, here !!
    tep_tx_data.tep_tx_udp_action.dipo = tep_info->key.ip_addr;
    MAC_UINT64_TO_ADDR(tep_tx_data.tep_tx_udp_action.dmac,
                       0x00020B0A0D0E);
    ret = tep_db()->tep_tx_tbl()->insert_withid(&tep_tx_data, hw_id_);
    return ret;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: release the directmap entry as well
sdk_ret_t
tep_entry::free_resources_(void) {
    if (hw_id_ != 0xFF) {
        tep_db()->tep_idxr()->free(hw_id_);
        // TODO: how do we know whether we actually programmed h/w or not ? need
        // a bit in api_base ?
        tep_db()->tep_tx_tbl()->remove(hw_id_);
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_entry::cleanup_hw(obj_ctxt_t *obj_ctxt) {
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
tep_entry::update_hw(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
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
tep_entry::activate_epoch(oci_epoch_t epoch, api_op_t api_op,
                          obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
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
 *
 * @param[in] tep tep
 */
sdk_ret_t
tep_entry::add_to_db(void) {
    return tep_db()->tep_ht()->insert_with_key(&key_, this,
                                               &ht_ctxt_);
}

/**
 * @brief delete tep from database
 *
 * @param[in] tep_key tep key
 */
sdk_ret_t
tep_entry::del_from_db(void) {
    tep_db()->tep_ht()->remove(&key_);
    return sdk::SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
tep_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_TEP, this);
}

/** @} */    // end of OCI_TEP_ENTRY

/**
 * @defgroup OCI_TEP_STATE - tep database functionality
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    constructor
 */
tep_state::tep_state() {
    p4pd_table_properties_t    tinfo;

    // TODO: need to tune multi-threading related params later
    tep_ht_ = ht::factory(OCI_MAX_TEP >> 2,
                          tep_entry::tep_key_func_get,
                          tep_entry::tep_hash_func_compute,
                          tep_entry::tep_key_func_compare);
    SDK_ASSERT(tep_ht_ != NULL);

    tep_idxr_ = indexer::factory(OCI_MAX_TEP);
    SDK_ASSERT(tep_idxr_ != NULL);

    tep_slab_ = slab::factory("tep", OCI_SLAB_TEP, sizeof(tep_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(tep_slab_ != NULL);

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_TEP_TX, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    tep_tx_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_TEP_TX,
                                     tinfo.tabledepth,
                                     tinfo.actiondata_struct_size,
                                     false, true, NULL);
    SDK_ASSERT(tep_tx_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
tep_state::~tep_state() {
    ht::destroy(tep_ht_);
    indexer::destroy(tep_idxr_);
    slab::destroy(tep_slab_);
    directmap::destroy(tep_tx_tbl_);
}

/**
 * @brief     allocate tep instance
 * @return    pointer to the allocated tep , NULL if no memory
 */
tep_entry *
tep_state::tep_alloc(void) {
    return ((tep_entry *)tep_slab_->alloc());
}

/**
 * @brief        lookup tep in database with given key
 * @param[in]    tep_key tep key
 * @return       pointer to the tep instance found or NULL
 */
tep_entry *
tep_state::tep_find(oci_tep_key_t *tep_key) const {
    return (tep_entry *)(tep_ht_->lookup(tep_key));
}

/**
 * @brief free tep instance
 * @param[in] tep tep instance
 */
void
tep_state::tep_free(tep_entry *tep) {
    api::delay_delete_to_slab(OCI_SLAB_TEP, tep);
}

/** @} */    // end of OCI_TEP_STATE

}    // namespace api

/**
 * @brief Create TEP
 *
 * @param[in] tep TEP information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_tep_create (_In_ oci_tep_t *tep)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_TEP;
        api_ctxt.api_params->tep_info = *tep;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief Delete TEP
 *
 * @param[in] tep_key TEP key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_tep_delete (_In_ oci_tep_key_t *tep_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_TEP;
        api_ctxt.api_params->tep_key = *tep_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of OCI_TEP_API
