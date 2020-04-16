//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements service mapping APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/pds_state.hpp"

static sdk_ret_t
pds_svc_mapping_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                            pds_obj_key_t *key,
                            pds_svc_mapping_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_SVC_MAPPING, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->svc_mapping_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline svc_mapping *
pds_svc_mapping_entry_find (pds_obj_key_t *key)
{
    // mapping does not have any entry database, as the calls are single thread,
    // we can use static entry
    static svc_mapping *svc_mapping;
    pds_svc_mapping_spec_t spec = { 0 };
    spec.key = *key;

    if (svc_mapping == NULL) {
        svc_mapping  = svc_mapping::factory(&spec);
    }
    return svc_mapping;
}

sdk_ret_t
pds_svc_mapping_create (_In_ pds_svc_mapping_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_svc_mapping_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_svc_mapping_read (pds_obj_key_t *key, pds_svc_mapping_info_t *info)
{
    sdk_ret_t rv;
    svc_mapping *entry = NULL;
    pds_svc_mapping_key_t skey;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    rv = svc_mapping_db()->skey(key, &skey);
    if (rv != SDK_RET_OK) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    entry = svc_mapping::build(&skey);
    if (entry == NULL) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    memset(info, 0, sizeof(pds_svc_mapping_info_t));
    info->spec.key = *key;

    return entry->read(key, info);
}

typedef struct svc_mapping_read_all_args_s {
    svc_mapping_read_cb_t cb;
    void *ctxt;
} svc_mapping_read_all_args_t;

void
pds_svc_mapping_info_from_entry (svc_mapping *entry, void *ctxt)
{
    sdk_ret_t rv;
    pds_svc_mapping_info_t info;
    svc_mapping_read_all_args_t *args = (svc_mapping_read_all_args_t *)ctxt;

    info.spec.key = entry->key();
    info.spec.skey = entry->skey();
    rv = entry->read(&entry->skey(), &info);
    if (rv == SDK_RET_OK) {
        args->cb(&info, args->ctxt);
    }
}

sdk_ret_t
pds_svc_mapping_read_all (svc_mapping_read_cb_t cb, void *ctxt)
{
    svc_mapping_read_all_args_t args;
    args.cb = cb;
    args.ctxt = ctxt;

    svc_mapping_db()->kvstore_iterate(pds_svc_mapping_info_from_entry, &args);
    return SDK_RET_OK;
}

sdk_ret_t
pds_svc_mapping_update (_In_ pds_svc_mapping_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_svc_mapping_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_svc_mapping_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_svc_mapping_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
