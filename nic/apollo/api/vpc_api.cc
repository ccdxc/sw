//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements VPC CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/vpc_state.hpp"

static sdk_ret_t
pds_vpc_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                    pds_obj_key_t *key, pds_vpc_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(OBJ_ID_VPC, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->vpc_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline vpc_entry *
pds_vpc_entry_find (pds_obj_key_t *key)
{
    return (vpc_db()->find(key));
}

//----------------------------------------------------------------------------
// VPC API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_vpc_create (_In_ pds_vpc_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_vpc_read (_In_ pds_obj_key_t *key, _Out_ pds_vpc_info_t *info)
{
    vpc_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_vpc_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_vpc_read_args_s {
    vpc_read_cb_t cb;
    void *ctxt;
} pds_vpc_read_args_t;

bool
pds_vpc_info_from_entry (void *entry, void *ctxt)
{
    vpc_entry *vpc = (vpc_entry *)entry;
    pds_vpc_read_args_t *args = (pds_vpc_read_args_t *)ctxt;
    pds_vpc_info_t info;

    memset(&info, 0, sizeof(pds_vpc_info_t));

    // call entry read
    vpc->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);

    return false;
}

sdk_ret_t
pds_vpc_read_all (vpc_read_cb_t vpc_read_cb, void *ctxt)
{
    pds_vpc_read_args_t args = { 0 };

    args.ctxt = ctxt;
    args.cb = vpc_read_cb;

    return vpc_db()->walk(pds_vpc_info_from_entry, &args);
}

sdk_ret_t
pds_vpc_update (_In_ pds_vpc_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_vpc_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

static sdk_ret_t
pds_vpc_peer_api_handle (pds_batch_ctxt_t bctxt,
                         api_op_t op, pds_obj_key_t *key,
                         pds_vpc_peer_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_VPC_PEER, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->vpc_peer_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline vpc_peer_entry *
pds_vpc_peer_entry_find (pds_obj_key_t *key)
{
    // vpc peer objects don't have any entry database as they are
    // stateless objects, we can use static entry
    static vpc_peer_entry *vpc_peer;
    pds_vpc_peer_spec_t spec = { 0 };
    spec.key = *key;

    if (vpc_peer == NULL) {
        vpc_peer = vpc_peer_entry::factory(&spec);
    }
    return vpc_peer;
}

//----------------------------------------------------------------------------
// VPC peer API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_vpc_peer_create (_In_ pds_vpc_peer_spec_t *spec,
                     _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_peer_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_vpc_peer_read (_In_ pds_obj_key_t *key,
                   _Out_ pds_vpc_peer_info_t *info)
{
    vpc_peer_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_vpc_peer_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_vpc_peer_read_all (vpc_peer_read_cb_t vpc_peer_read_cb, void *ctxt)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_vpc_peer_update (_In_ pds_vpc_peer_spec_t *spec,
                     _In_ pds_batch_ctxt_t bctxt)
{
    return (pds_vpc_peer_api_handle(bctxt, API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
pds_vpc_peer_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return (pds_vpc_peer_api_handle(bctxt, API_OP_DELETE, key, NULL));
}
