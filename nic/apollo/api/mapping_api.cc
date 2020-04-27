//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements mapping API
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/mapping_state.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"

static sdk_ret_t
pds_mapping_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                        pds_obj_key_t *key, pds_mapping_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_MAPPING, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->mapping_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline mapping_entry *
pds_mapping_entry_find (pds_obj_key_t *key)
{
    // mapping does not have any entry database, as the calls are single thread,
    // we can use static entry
    static mapping_entry *mapping;
    pds_mapping_spec_t spec;
    memset(&spec, 0, sizeof(spec));
    spec.key = *key;

    if (mapping == NULL) {
        mapping  = mapping_entry::factory(&spec);
    }
    return mapping;
}

//----------------------------------------------------------------------------
// mapping create routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_create (_In_ pds_local_mapping_spec_t *local_spec,
                          _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    pds_local_spec_to_mapping_spec(&spec, local_spec);
    return pds_mapping_api_handle(bctxt, API_OP_CREATE,
                                  (pds_obj_key_t *)NULL, &spec);
}

sdk_ret_t
pds_remote_mapping_create (_In_ pds_remote_mapping_spec_t *remote_spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    if ((remote_spec->nh_type != PDS_NH_TYPE_OVERLAY) &&
        (remote_spec->nh_type != PDS_NH_TYPE_OVERLAY_ECMP)) {
        return SDK_RET_INVALID_ARG;
    }
    pds_remote_spec_to_mapping_spec(&spec, remote_spec);
    return pds_mapping_api_handle(bctxt, API_OP_CREATE,
                                  (pds_obj_key_t *)NULL, &spec);
}

//----------------------------------------------------------------------------
// mapping read routines
//----------------------------------------------------------------------------

typedef struct mapping_read_all_args_s {
    mapping_read_cb_t cb;
    void *ctxt;
    bool is_local;
} mapping_read_all_args_t;

void
pds_mapping_info_from_entry (mapping_entry *entry, void *ctxt)
{
    sdk_ret_t rv;
    pds_mapping_info_t info;
    mapping_read_all_args_t *args = (mapping_read_all_args_t *)ctxt;

    if (entry->is_local() != args->is_local) {
        return;
    }

    info.spec.key = entry->key();
    info.spec.skey = entry->skey();
    rv = entry->read(&entry->skey(), &info);
    if (rv == SDK_RET_OK) {
        if (entry->is_local()) {
            pds_local_mapping_info_t local_info;
            memset(&local_info, 0, sizeof(pds_local_mapping_info_t));
            pds_mapping_spec_to_local_spec(&local_info.spec, &info.spec);
            args->cb(&local_info, args->ctxt);
        } else {
            pds_remote_mapping_info_t remote_info;
            memset(&remote_info, 0, sizeof(pds_remote_mapping_info_t));
            pds_mapping_spec_to_remote_spec(&remote_info.spec, &info.spec);
            args->cb(&remote_info, args->ctxt);
        }
    }
}

sdk_ret_t
pds_local_mapping_read_all (mapping_read_cb_t cb, void *ctxt)
{
    mapping_read_all_args_t args;
    args.cb = cb;
    args.ctxt = ctxt;
    args.is_local = 1;

    mapping_db()->kvstore_iterate(pds_mapping_info_from_entry, &args);
    return SDK_RET_OK;
}

sdk_ret_t
pds_remote_mapping_read_all (mapping_read_cb_t cb, void *ctxt)
{
    mapping_read_all_args_t args;
    args.cb = cb;
    args.ctxt = ctxt;
    args.is_local = 0;

    mapping_db()->kvstore_iterate(pds_mapping_info_from_entry, &args);
    return SDK_RET_OK;
}

sdk_ret_t
pds_local_mapping_read (pds_obj_key_t *key,
                        pds_local_mapping_info_t *local_info)
{
    mapping_entry *entry;
    pds_mapping_key_t skey;
    pds_mapping_info_t info;
    sdk_ret_t rv = SDK_RET_OK;

    if ((key == NULL) || (local_info == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    rv = mapping_db()->skey(key, &skey);
    if (rv != SDK_RET_OK) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    entry = mapping_entry::build(&skey);
    if (entry == NULL) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    memset(&info, 0, sizeof(pds_mapping_info_t));
    info.spec.key = *key;
    rv = entry->read(key, &info);
    if (rv == SDK_RET_OK) {
        pds_mapping_spec_to_local_spec(&local_info->spec, &info.spec);
    }
    mapping_entry::soft_delete(entry);
    return rv;
}

sdk_ret_t
pds_remote_mapping_read (pds_obj_key_t *key,
                         pds_remote_mapping_info_t *remote_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    pds_mapping_key_t skey;
    sdk_ret_t rv = SDK_RET_OK;

    if ((key == NULL) || (remote_info == NULL)) {
        return SDK_RET_INVALID_ARG;
    }
    rv = mapping_db()->skey(key, &skey);
    if (rv != SDK_RET_OK) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    entry = mapping_entry::build(&skey);
    memset(&info, 0, sizeof(pds_mapping_info_t));
    info.spec.key = *key;
    rv = entry->read(key, &info);
    if (rv == SDK_RET_OK) {
        pds_mapping_spec_to_remote_spec(&remote_info->spec, &info.spec);
    }
    mapping_entry::soft_delete(entry);
    return rv;
}

//----------------------------------------------------------------------------
// mapping update routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_update (_In_ pds_local_mapping_spec_t *local_spec,
                          _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    pds_local_spec_to_mapping_spec(&spec, local_spec);
    return pds_mapping_api_handle(bctxt, API_OP_UPDATE,
                                  (pds_obj_key_t *)NULL, &spec);
}

sdk_ret_t
pds_remote_mapping_update (_In_ pds_remote_mapping_spec_t *remote_spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    pds_mapping_spec_t spec;

    pds_remote_spec_to_mapping_spec(&spec, remote_spec);
    return pds_mapping_api_handle(bctxt, API_OP_UPDATE,
                                  (pds_obj_key_t *)NULL, &spec);
}

//----------------------------------------------------------------------------
// mapping delete routines
//----------------------------------------------------------------------------

sdk_ret_t
pds_local_mapping_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mapping_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

sdk_ret_t
pds_remote_mapping_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mapping_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
