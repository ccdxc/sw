/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   vnic entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

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
    vnic_db()->vnic_free(vnic);
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
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::add_to_db(void) {
    return vnic_db()->vnic_ht()->insert_with_key(&key_, this,
                                                 &ht_ctxt_);
}

/**
 * @brief delete vnic from database
 * @return   SDK_RET_OK on success, failure status code on error
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

}    // namespace api
