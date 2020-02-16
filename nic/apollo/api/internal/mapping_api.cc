//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines mapping APIs for internal module interactions
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"

namespace api {

// NOTE:
// after this call it is possible that spec is populated with uuid of the
// mapping (in case of create & update operations)
static sdk_ret_t
pds_mapping_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                        pds_mapping_key_t *skey, pds_mapping_spec_t *spec)
{
    api_ctxt_t *api_ctxt;
    pds_obj_key_t key = { 0 };

    if ((op == API_OP_DELETE) && skey) {
        return SDK_RET_OK;
    }

    if (((op == API_OP_CREATE) || (op == API_OP_UPDATE)) && spec) {
        return SDK_RET_OK;
    }

    // TODO:
    // 1. do 2nd-ary key to primary key lookup
    // 2. use primary key to delete
    api_ctxt = api::api_ctxt_alloc(OBJ_ID_MAPPING, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->mapping_key = key;
        } else {
            spec->key = key;
            api_ctxt->api_params->mapping_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline mapping_entry *
pds_mapping_entry_find (pds_mapping_key_t *skey)
{
    // mapping does not have any entry database, as the calls are single thread,
    // we can use static entry
    pds_mapping_spec_t spec;
    static mapping_entry *mapping;

    memset(&spec, 0, sizeof(spec));
    // TODO:
    // 1. do 2nd-ary key to primary key lookup (copy pkey to spec.key directly)
    // 2. return SDK_RET_ENTRY_NOT_FOUND, if not found
    spec.skey = *skey;
    if (mapping == NULL) {
        mapping  = mapping_entry::factory(&spec);
    }
    return mapping;
}

sdk_ret_t
pds_local_mapping_read (pds_mapping_key_t *skey,
                        pds_local_mapping_info_t *local_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    if ((skey == NULL) || (local_info == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mapping_entry_find(skey)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memset(&info, 0, sizeof(pds_mapping_info_t));
    info.spec.skey = *skey;
    entry->set_local(true);
    ret = entry->read(skey, &info);
    if (ret == SDK_RET_OK) {
        pds_mapping_spec_to_local_spec(&local_info->spec, &info.spec);
    }
    return ret;
}

sdk_ret_t
pds_remote_mapping_read (pds_mapping_key_t *skey,
                         pds_remote_mapping_info_t *remote_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    if ((skey == NULL) || (remote_info == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mapping_entry_find(skey)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memset(&info, 0, sizeof(pds_mapping_info_t));
    info.spec.skey = *skey;
    entry->set_local(false);
    ret = entry->read(skey, &info);
    if (ret == SDK_RET_OK) {
        pds_mapping_spec_to_remote_spec(&remote_info->spec, &info.spec);
    }
    return ret;
}

sdk_ret_t
pds_local_mapping_delete (_In_ pds_mapping_key_t *skey,
                          _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mapping_api_handle(bctxt, API_OP_DELETE, skey, NULL);
}

sdk_ret_t
pds_remote_mapping_delete (_In_ pds_mapping_key_t *skey,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_mapping_api_handle(bctxt, API_OP_DELETE, skey, NULL);
}

}    // namespace api
