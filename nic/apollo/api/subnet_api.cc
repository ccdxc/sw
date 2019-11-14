//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements subnet CRUD API
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/subnet_state.hpp"

static sdk::sdk_ret_t
pds_subnet_api_handle (pds_batch_ctxt_t bctxt, api::api_op_t op,
                       pds_subnet_key_t *key, pds_subnet_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != sdk::SDK_RET_OK)
        return rv;

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(api::OBJ_ID_SUBNET, op);
    if (likely(api_ctxt != NULL)) {
        if (op == api::API_OP_DELETE) {
            api_ctxt->api_params->subnet_key = *key;
        } else {
            api_ctxt->api_params->subnet_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk::sdk_ret_t
pds_subnet_stats_fill (pds_subnet_stats_t *stats, subnet_entry *entry)
{
    return SDK_RET_OK;
}

static inline sdk::sdk_ret_t
pds_subnet_status_fill (pds_subnet_status_t *status, subnet_entry *entry)
{
    status->hw_id = entry->hw_id();
    return SDK_RET_OK;
}

static inline sdk::sdk_ret_t
pds_subnet_spec_fill (pds_subnet_spec_t *spec, subnet_entry *entry)
{
    pds_vpc_key_t vpc = {};

    vpc = entry->vpc();
    spec->vpc.id = vpc.id;
    spec->v4_route_table = entry->v4_route_table();
    spec->v6_route_table = entry->v6_route_table();
    spec->ing_v4_policy = entry->ing_v4_policy();
    spec->ing_v6_policy = entry->ing_v6_policy();
    spec->egr_v4_policy = entry->egr_v4_policy();
    spec->egr_v6_policy = entry->egr_v6_policy();
    memcpy(&spec->vr_mac, entry->vr_mac(), sizeof(mac_addr_t));
    spec->fabric_encap = entry->fabric_encap();

    return SDK_RET_OK;
}

static inline subnet_entry *
pds_subnet_entry_find (pds_subnet_key_t *key)
{
    return (subnet_db()->find(key));
}

//----------------------------------------------------------------------------
// Subnet API entry point implementation
//----------------------------------------------------------------------------

sdk::sdk_ret_t
pds_subnet_create (_In_ pds_subnet_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_subnet_api_handle(bctxt, api::API_OP_CREATE, NULL, spec);
}

sdk::sdk_ret_t
pds_subnet_read (pds_subnet_key_t *key, pds_subnet_info_t *info)
{
    sdk::sdk_ret_t rv;
    subnet_entry *entry = NULL;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_subnet_entry_find(key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = pds_subnet_spec_fill(&info->spec, entry)) != sdk::SDK_RET_OK) {
        return rv;
    }

    info->spec.key = *key;

    if ((rv = pds_subnet_status_fill(&info->status, entry)) !=
            sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = pds_subnet_stats_fill(&info->stats, entry)) != sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = entry->read(key, info)) != sdk::SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
pds_subnet_update (_In_ pds_subnet_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_subnet_api_handle(bctxt, api::API_OP_UPDATE, NULL, spec);
}

sdk::sdk_ret_t
pds_subnet_delete (_In_ pds_subnet_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_subnet_api_handle(bctxt, api::API_OP_DELETE, key, NULL);
}
