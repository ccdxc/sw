//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module implements Switchport API
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/api/switchport.hpp"
#include "nic/apollo/api/impl/oci_impl_state.hpp"

sdk_ret_t
oci_switchport_create (oci_switchport_spec_t *switchport)
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params =
        api::api_params_alloc(api::OBJ_ID_SWITCHPORT, api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_SWITCHPORT;
        api_ctxt.api_params->switchport_spec = *switchport;
        return api::g_api_engine.process_api(&api_ctxt);
    }
    return sdk::SDK_RET_OOM;
}

static inline void
oci_switchport_spec_fill (switchport_entry *entry, oci_switchport_spec_t *spec)
{
    spec->gateway_ip_addr = entry->gw_ip_addr();
}

sdk_ret_t
oci_switchport_read (oci_switchport_info_t *info)
{
    switchport_entry *entry;
    api::impl::switchport_impl *impl;

    if (info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    entry = switchport_entry::find_in_db();
    if (entry == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    // Fill from software state
    oci_switchport_spec_fill(entry, &info->spec);

    impl = dynamic_cast<api::impl::switchport_impl*>(entry->impl());
    return impl->read_hw(info);
}

sdk_ret_t
oci_switchport_delete ()
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params =
        api::api_params_alloc(api::OBJ_ID_SWITCHPORT, api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_SWITCHPORT;
        return api::g_api_engine.process_api(&api_ctxt);
    }
    return sdk::SDK_RET_OOM;
}
