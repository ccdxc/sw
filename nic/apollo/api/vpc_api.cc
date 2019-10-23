//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements VPC CRUD API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/vpc_state.hpp"

static sdk_ret_t
pds_vpc_api_handle (pds_batch_ctxt_t bctxt, api::api_op_t op,
                    pds_vpc_key_t *key, pds_vpc_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(api::OBJ_ID_VPC, op);
    if (likely(api_ctxt != NULL)) {
        if (op == api::API_OP_DELETE) {
            api_ctxt->api_params->vpc_key = *key;
        } else {
            api_ctxt->api_params->vpc_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_vpc_stats_fill (pds_vpc_stats_t *stats, vpc_entry *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_vpc_status_fill (pds_vpc_status_t *status, vpc_entry *entry)
{
    status->hw_id = entry->hw_id();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_vpc_spec_fill (pds_vpc_spec_t *spec, vpc_entry *entry)
{
    spec->type = entry->type();
    spec->fabric_encap = entry->fabric_encap();
    if (entry->nat46_prefix_valid()) {
        memcpy(&spec->nat46_prefix, &entry->nat46_prefix(),
               sizeof(ip_prefix_t));
    }
    return SDK_RET_OK;
}

static inline vpc_entry *
pds_vpc_entry_find (pds_vpc_key_t *key)
{
    return (vpc_db()->find(key));
}

//----------------------------------------------------------------------------
// VPC API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_vpc_create (_In_ pds_vpc_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_api_handle(bctxt, api::API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_vpc_read (pds_vpc_key_t *key, pds_vpc_info_t *info)
{
    sdk_ret_t rv;
    vpc_entry *entry = NULL;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_vpc_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = pds_vpc_spec_fill(&info->spec, entry)) != SDK_RET_OK) {
        return rv;
    }

    if ((rv = entry->read(key, info)) != sdk::SDK_RET_OK) {
        return rv;
    }

    info->spec.key = *key;

    if ((rv = pds_vpc_status_fill(&info->status, entry)) != SDK_RET_OK) {
        return rv;
    }

    if ((rv = pds_vpc_stats_fill(&info->stats, entry)) != SDK_RET_OK) {
        return rv;
    }

    return SDK_RET_OK;
}

sdk_ret_t
pds_vpc_update (_In_ pds_vpc_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_api_handle(bctxt, api::API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_vpc_delete (_In_ pds_vpc_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_vpc_api_handle(bctxt, api::API_OP_DELETE, key, NULL);
}

static sdk_ret_t
pds_vpc_peer_api_handle (pds_batch_ctxt_t bctxt,
                         api::api_op_t op, pds_vpc_peer_key_t *key,
                         pds_vpc_peer_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(api::OBJ_ID_VPC_PEER, op);
    if (likely(api_ctxt != NULL)) {
        if (op == api::API_OP_DELETE) {
            api_ctxt->api_params->vpc_peer_key = *key;
        } else {
            api_ctxt->api_params->vpc_peer_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_vpc_peer_stats_fill (pds_vpc_peer_stats_t *stats, vpc_peer_entry *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_vpc_peer_status_fill (pds_vpc_peer_status_t *status, vpc_peer_entry *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_vpc_peer_spec_fill (pds_vpc_peer_spec_t *spec, vpc_peer_entry *entry)
{
    return SDK_RET_OK;
}

static inline vpc_peer_entry *
pds_vpc_peer_entry_find (pds_vpc_peer_key_t *key)
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
    return pds_vpc_peer_api_handle(bctxt, api::API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_vpc_peer_read (pds_vpc_peer_key_t *key, pds_vpc_peer_info_t *info)
{
    sdk_ret_t rv;
    vpc_peer_entry *entry = NULL;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_vpc_peer_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = pds_vpc_peer_spec_fill(&info->spec, entry)) != SDK_RET_OK) {
        return rv;
    }
    info->spec.key = *key;

    if ((rv = pds_vpc_peer_status_fill(&info->status, entry)) != SDK_RET_OK) {
        return rv;
    }

    if ((rv = pds_vpc_peer_stats_fill(&info->stats, entry)) != SDK_RET_OK) {
        return rv;
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_vpc_peer_update (_In_ pds_vpc_peer_spec_t *spec,
                     _In_ pds_batch_ctxt_t bctxt)
{
    return (pds_vpc_peer_api_handle(bctxt, api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
pds_vpc_peer_delete (_In_ pds_vpc_peer_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return (pds_vpc_peer_api_handle(bctxt, api::API_OP_DELETE, key, NULL));
}
