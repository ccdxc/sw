/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    switchport.cc
 *
 * @brief   switchport entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/impl.hpp"
#include "nic/apollo/api/switchport.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_SWITCHPORT_ENTRY - switchport entry functionality
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief    factory method to allocate and initialize a switchport entry
 * @param[in] oci_switchport    switchport information
 * @return    new instance of switchport or NULL, in case of error
 */
switchport_entry *
switchport_entry::factory(oci_switchport_t *oci_switchport) {
    switchport_entry    *switchport;

    switchport = switchport_db()->switchport_alloc();
    if (switchport) {
        new (switchport) switchport_entry();
        switchport->impl_ =
            impl_base::factory(impl::IMPL_OBJ_ID_SWITCHPORT, oci_switchport);
        if (switchport->impl_ == NULL) {
            switchport_entry::destroy(switchport);
            return NULL;
        }
    }
    return switchport;
}

/**
 * @brief    release all the s/w state associate with the given switchport,
 *           if any, and free the memory
 * @param[in] switchport     switchport to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
switchport_entry::destroy(switchport_entry *switchport) {
    if (switchport->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_SWITCHPORT, switchport->impl_);
    }
    switchport->~switchport_entry();
    switchport_db()->switchport_free(switchport);
}

/**
 * @brief     initialize switchport entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_switchport_t *oci_switchport = &api_ctxt->api_params->switchport_info;

    ip_addr_ = oci_switchport->switch_ip_addr;
    memcpy(mac_addr_, oci_switchport->switch_mac_addr, ETH_ADDR_LEN);
    gw_ip_addr_ = oci_switchport->gateway_ip_addr;
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any, during creation of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::program_config(obj_ctxt_t *obj_ctxt) {
    OCI_TRACE_DEBUG("Programming switchport config");
    return impl_->program_hw(this, obj_ctxt);
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::release_resources(void) {
    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 *           and setting invalid bit (if any) in the h/w entries
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
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
switchport_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->update_hw(orig_obj, this, obj_ctxt);
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
switchport_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 programming for switchport cfg, so this is a no-op
    OCI_TRACE_DEBUG("Activated switchport config");
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
switchport_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // TODO: will address this later !!
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add switchport to database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::add_to_db(void) {
    return switchport_db()->insert(this);
}

/**
 * @brief delete switchport from database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::del_from_db(void) {
    /**< this is a singleton obj, so this is no-op */
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
switchport_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_SWITCHPORT, this);
}

/** @} */    // end of OCI_SWITCHPORT_ENTRY

}    // namespace api
