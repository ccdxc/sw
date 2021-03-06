//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements VNIC CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"

static inline sdk_ret_t
pds_vnic_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                     pds_obj_key_t *key, pds_vnic_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_VNIC, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->vnic_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline vnic_entry *
pds_vnic_entry_find (pds_obj_key_t *key)
{
    return (vnic_db()->find(key));
}

//----------------------------------------------------------------------------
// VNIC API entry point implementation
//----------------------------------------------------------------------------
sdk_ret_t
pds_vnic_create (_In_ pds_vnic_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vnic_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_vnic_read (_In_ pds_obj_key_t *key, _Out_ pds_vnic_info_t *info)
{
    vnic_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_vnic_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_vnic_read_args_s {
    vnic_read_cb_t cb;
    void *ctxt;
} pds_vnic_read_args_t;

static bool
pds_vnic_info_from_entry (void *entry, void *ctxt)
{
    vnic_entry *vnic = (vnic_entry *)entry;
    pds_vnic_read_args_t *args = (pds_vnic_read_args_t *)ctxt;
    pds_vnic_info_t info;

    memset(&info, 0, sizeof(pds_vnic_info_t));

    // call entry read
    vnic->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);

    return false;
}

sdk_ret_t
pds_vnic_read_all (vnic_read_cb_t vnic_read_cb, void *ctxt)
{
    pds_vnic_read_args_t args = { 0 };

    args.ctxt = ctxt;
    args.cb = vnic_read_cb;

    return vnic_db()->walk(pds_vnic_info_from_entry, &args);
}

sdk_ret_t
pds_vnic_update (_In_ pds_vnic_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vnic_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_vnic_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vnic_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

static bool
vnic_stats_reset_cb (void *entry, void *ctxt)
{
    vnic_entry *vnic = (vnic_entry *)entry;

    vnic->reset_stats();
    // continue the walk
    return false;
}

sdk_ret_t
pds_vnic_stats_reset (_In_ pds_obj_key_t *key)
{
    vnic_entry *vnic;

    if (key) {
        vnic = vnic_db()->find(key);
        if (vnic) {
            return vnic->reset_stats();
        }
        PDS_TRACE_ERR("Failed to reset vnic %s stats, vnic not found",
                      key->str());
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return vnic_db()->walk(vnic_stats_reset_cb, NULL);
}
