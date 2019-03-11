/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    device.cc
 *
 * @brief   device entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/impl.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup PDS_DEVICE_ENTRY - device entry functionality
 * @ingroup PDS_DEVICE
 * @{
 */

/**
 * @brief    factory method to allocate and initialize a device entry
 * @param[in] pds_device device information
 * @return    new instance of device or NULL, in case of error
 */
device_entry *
device_entry::factory(pds_device_spec_t *pds_device) {
    device_entry *device;

    device = device_db()->alloc();
    if (device) {
        new (device) device_entry();
        device->impl_ =
            impl_base::factory(impl::IMPL_OBJ_ID_DEVICE, pds_device);
        if (device->impl_ == NULL) {
            device_entry::destroy(device);
            return NULL;
        }
    }
    return device;
}

/**
 * @brief    release all the s/w state associate with the given device,
 *           if any, and free the memory
 * @param[in] device device to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
device_entry::destroy(device_entry *device) {
    device->release_resources();
    if (device->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_DEVICE, device->impl_);
    }
    device->~device_entry();
    device_db()->free(device);
}

/**
 * @brief     initialize device entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
device_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_device_spec_t *pds_device = &api_ctxt->api_params->device_spec;

    ip_addr_ = pds_device->switch_ip_addr;
    memcpy(mac_addr_, pds_device->switch_mac_addr, ETH_ADDR_LEN);
    gw_ip_addr_ = pds_device->gateway_ip_addr;
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
device_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
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
device_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                              obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 programming for device cfg, so this is a no-op
    PDS_TRACE_DEBUG("Activating device config");
    //impl_->activate_hw(this, epoch, api_op, obj_ctxt);
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
device_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // TODO: will address this later !!
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief add device to database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
device_entry::add_to_db(void) {
    return device_db()->insert(this);
}

/**
 * @brief delete device instance from database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
device_entry::del_from_db(void) {
    if (device_db()->remove()) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
device_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_DEVICE, this);
}

/** @} */    // end of PDS_DEVICE_ENTRY

}    // namespace api
