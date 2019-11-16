//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// device entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/impl.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/// \defgroup PDS_DEVICE_ENTRY - device entry functionality
/// \ingroup PDS_DEVICE
/// \@{

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

void
device_entry::destroy(device_entry *device) {
    device->nuke_resources_();
    if (device->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_DEVICE, device->impl_);
    }
    device->~device_entry();
    device_db()->free(device);
}

sdk_ret_t
device_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_device_spec_t *spec = &api_ctxt->api_params->device_spec;

    memcpy(&ip_addr_, &spec->device_ip_addr, sizeof(ip_addr_));
    memcpy(mac_addr_, spec->device_mac_addr, ETH_ADDR_LEN);
    memcpy(&gw_ip_addr_, &spec->gateway_ip_addr, sizeof(gw_ip_addr_));
    oper_mode_ = spec->dev_oper_mode;
    bridging_en_ = spec->bridging_en;
    learning_en_ = spec->learning_en;
    return SDK_RET_OK;
}

sdk_ret_t
device_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->update_hw(orig_obj, this, obj_ctxt);
}

sdk_ret_t
device_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                              obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 programming for device cfg, so this is a no-op
    PDS_TRACE_DEBUG("Activating device config");
    impl_->activate_hw(this, epoch, api_op, obj_ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
device_entry::read(pds_device_info_t *info) {
    return impl_->read_hw(this, NULL, (impl::obj_info_t *)info);
}

sdk_ret_t
device_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
device_entry::add_to_db(void) {
    return device_db()->insert(this);
}

sdk_ret_t
device_entry::del_from_db(void) {
    if (device_db()->remove()) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
device_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_DEVICE, this);
}

/// \@}

}    // namespace api
