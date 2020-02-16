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
#include "nic/apollo/api/mapping.hpp"
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
            api_ctxt->api_params->mapping_key = *key;
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

sdk_ret_t
pds_local_mapping_read (pds_obj_key_t *key,
                        pds_local_mapping_info_t *local_info)
{
    pds_mapping_info_t info;
    sdk_ret_t rv = SDK_RET_OK;
    mapping_entry *entry = NULL;

    if ((key == NULL) || (local_info == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mapping_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memset(&info, 0, sizeof(pds_mapping_info_t));
    info.spec.key = *key;
    entry->set_local(true);
    rv = entry->read(key, &info);
    if (rv == SDK_RET_OK) {
        pds_mapping_spec_to_local_spec(&local_info->spec, &info.spec);
    }
    return rv;
}

sdk_ret_t
pds_remote_mapping_read (pds_obj_key_t *key,
                         pds_remote_mapping_info_t *remote_info)
{
    pds_mapping_info_t info;
    mapping_entry *entry = NULL;
    sdk_ret_t rv = SDK_RET_OK;

    if ((key == NULL) || (remote_info == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_mapping_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    memset(&info, 0, sizeof(pds_mapping_info_t));
    info.spec.key = *key;
    entry->set_local(false);
    rv = entry->read(key, &info);
    if (rv == SDK_RET_OK) {
        pds_mapping_spec_to_remote_spec(&remote_info->spec, &info.spec);
    }
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
