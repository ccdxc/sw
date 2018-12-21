/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    switchport.cc
 *
 * @brief   This file deals with switchport api handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/switchport.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

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
    return switchport_db()->switchport_alloc();
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
    return sdk::SDK_RET_OK;
}

#if 0
/**
 * @brief     update/override the switchport object with given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::update_config(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::alloc_resources_(void) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any, during creation of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::program_config(obj_ctxt_t *obj_ctxt) {
    // TODO: need to progam table constants here !!!
    return sdk::SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_entry::free_resources_(void) {
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
switchport_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // TODO: need to progam table constants here !!!
    // impl->update_hw()
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
switchport_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 programming for switchport cfg, so this is a no-op
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
switchport_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add switchport to database
 *
 * @param[in] switchport switchport
 */
sdk_ret_t
switchport_entry::add_to_db(void) {
    return sdk::SDK_RET_OK;
}

/** @} */    // end of OCI_SWITCHPORT_ENTRY

/**
 * @defgroup OCI_SWITCHPORT_API - first level of switchport API handling
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief create switchport
 *
 * @param[in] switchport switchport information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_switchport_create (_In_ oci_switchport_t *switchport)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_SWITCHPORT;
        api_ctxt.api_params->switchport_info = *switchport;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */    // end of OCI_SWITCHPORT_API

}    // namespace api
