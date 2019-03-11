//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements Device API
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"

sdk_ret_t
pds_device_create (pds_device_spec_t *device)
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params =
        api::api_params_alloc(api::OBJ_ID_DEVICE, api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_DEVICE;
        api_ctxt.api_params->device_spec = *device;
        return api::g_api_engine.process_api(&api_ctxt);
    }
    return sdk::SDK_RET_OOM;
}

static inline void
pds_device_spec_fill (device_entry *entry, pds_device_spec_t *spec)
{
    spec->gateway_ip_addr = entry->gw_ip_addr();
}

sdk_ret_t
pds_device_read (pds_device_info_t *info)
{
    device_entry *entry;
    api::impl::device_impl *impl;

    if (info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    entry = device_db()->find();
    if (entry == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    // fill from software state
    pds_device_spec_fill(entry, &info->spec);

    impl = dynamic_cast<api::impl::device_impl*>(entry->impl());
    return impl->read_hw(info);
}

sdk_ret_t
pds_device_delete (void)
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params =
        api::api_params_alloc(api::OBJ_ID_DEVICE, api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_DEVICE;
        return api::g_api_engine.process_api(&api_ctxt);
    }
    return sdk::SDK_RET_OOM;
}
