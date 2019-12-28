//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements interface CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/if_state.hpp"
#include "nic/sdk/include/sdk/if.hpp"

static sdk_ret_t
pds_if_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                   pds_if_key_t *key, pds_if_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_IF, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->if_key = *key;
        } else {
            api_ctxt->api_params->if_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_if_stats_fill (pds_if_stats_t *stats, if_entry *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_if_status_fill (pds_if_status_t *status, if_entry *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_if_spec_fill (pds_if_spec_t *spec, if_entry *entry)
{
    spec->key.id = entry->key().id;
    spec->type = entry->type();
    spec->admin_state = entry->admin_state();
    spec->type = entry->type();
    switch (spec->type) {
    case PDS_IF_TYPE_UPLINK:
        spec->uplink_info.port_num = entry->port();
        break;
    case PDS_IF_TYPE_L3:
        spec->l3_if_info.eth_ifindex = entry->port();
        spec->l3_if_info.vpc = entry->l3_vpc();
        spec->l3_if_info.ip_prefix = entry->l3_ip_prefix();
        spec->l3_if_info.encap = entry->l3_encap();
        memcpy(spec->l3_if_info.mac_addr, entry->l3_mac(),
               ETH_ADDR_LEN);
        break;
    default:
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static inline if_entry *
pds_if_entry_find (pds_if_key_t *key)
{
    return (if_db()->find(key));
}

//----------------------------------------------------------------------------
// interface API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_if_create (_In_ pds_if_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_if_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_if_read (_In_ pds_if_key_t *key, _Out_ pds_if_info_t *info)
{
    if_entry *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_if_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

typedef struct pds_if_read_args_s {
    if_read_cb_t cb;
    void *ctxt;
} pds_if_read_args_t;

static bool
pds_if_info_from_entry (void *entry, void *ctxt)
{
    pds_if_info_t info = { 0 };
    if_entry *intf = (if_entry *)entry;
    pds_if_read_args_t *args = (pds_if_read_args_t *)ctxt;

    if (intf->type() != PDS_IF_TYPE_NONE) {
        intf->read(&info);
        args->cb(&info, args->ctxt);
    }

    return false;
}

sdk_ret_t
pds_if_read_all (if_read_cb_t cb, void *ctxt)
{
    pds_if_read_args_t args = {0};
    args.ctxt = ctxt;
    args.cb = cb;

    return if_db()->walk(IF_TYPE_NONE, pds_if_info_from_entry, &args);
}

sdk_ret_t
pds_if_update (_In_ pds_if_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_if_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_if_delete (_In_ pds_if_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_if_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
