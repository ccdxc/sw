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
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup PDS_VNIC_ENTRY - vnic entry functionality
 * @ingroup PDS_VNIC
 * @{
 */

/**< @brief    constructor */
vnic_entry::vnic_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

/**
 * @brief    factory method to allocate and initialize a vnic entry
 * @param[in] spec    vnic spec
 * @return    new instance of vnic or NULL, in case of error
 */
vnic_entry *
vnic_entry::factory(pds_vnic_spec_t *spec) {
    vnic_entry *vnic;

    /**< create vnic entry with defaults, if any */
    vnic = vnic_db()->vnic_alloc();
    if (vnic) {
        new (vnic) vnic_entry();
        vnic->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_VNIC, spec);
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
    vnic->release_resources();
    if (vnic->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_VNIC, vnic->impl_);
    }
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
    pds_vnic_spec_t *spec = &api_ctxt->api_params->vnic_spec;

    memcpy(&this->key_, &spec->key, sizeof(pds_vnic_key_t));
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::program_config(obj_ctxt_t *obj_ctxt) {
    pds_vnic_spec_t    *spec = &obj_ctxt->api_params->vnic_spec;

    PDS_TRACE_DEBUG("Programming vnic %u, vcn %u, subnet %u, mac %s, vlan %u, "
                    "encap type %u, encap value %u", key_.id, spec->vcn.id,
                    spec->subnet.id, macaddr2str(spec->mac_addr),
                    spec->wire_vlan, spec->fabric_encap.type,
                    spec->fabric_encap.val.value);
    return impl_->program_hw(this, obj_ctxt);
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::release_resources(void) {
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
vnic_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating vnic %u config", key_.id);
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
    return delay_delete_to_slab(PDS_SLAB_ID_VNIC, this);
}

/** @} */    // end of PDS_VNIC_ENTRY

}    // namespace api
