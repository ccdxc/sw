//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements Device CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/obj_api.hpp"

static inline sdk_ret_t
pds_device_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                       pds_device_spec_t *spec)
{
    api_ctxt_t *api_ctxt;

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_DEVICE, op);
    if (likely(api_ctxt != NULL)) {
        if (op != API_OP_DELETE) {
            api_ctxt->api_params->device_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline device_entry *
pds_device_find (void)
{
    return (device_db()->find());
}

sdk_ret_t
pds_device_create (_In_ pds_device_spec_t *device, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_device_api_handle(bctxt, API_OP_CREATE, device);
}

sdk_ret_t
pds_device_read (_Out_ pds_device_info_t *info)
{
    device_entry *entry;

    if (info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_device_find()) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_device_update (_In_ pds_device_spec_t *device, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_device_api_handle(bctxt, API_OP_UPDATE, device);
}

sdk_ret_t
pds_device_delete (_In_ pds_batch_ctxt_t bctxt)
{
    return pds_device_api_handle(bctxt, API_OP_DELETE, NULL);
}
