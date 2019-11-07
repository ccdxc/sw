//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements LIF Read APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/include/pds_lif.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/if.hpp"

static inline pds_if_state_t
lif_state_to_if_state (lif_state_t state)
{
    switch (state) {
    case sdk::types::LIF_STATE_DOWN:
        return PDS_IF_STATE_DOWN;
        break;
    case sdk::types::LIF_STATE_UP:
        return PDS_IF_STATE_UP;
        break;
    default:
        return PDS_IF_STATE_NONE;
        break;
    }
}

sdk_ret_t
pds_lif_read (pds_lif_key_t *key, pds_lif_info_t *info)
{
    pds_lif_spec_t *spec = &info->spec;
    pds_lif_status_t *status = &info->status;

    api::impl::lif_impl *lif = (api::impl::lif_impl *)(lif_db()->find(key));
    if (lif == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    spec->key = lif->key();
    spec->pinned_ifidx = lif->pinned_ifindex();
    spec->type = lif->type();
    strncpy(status->name, lif->name(), SDK_MAX_NAME_LEN);
    status->state = lif_state_to_if_state(lif->state());
    status->ifindex = LIF_IFINDEX(spec->key);
   
   return SDK_RET_OK;
}

typedef struct pds_lif_read_args_s {
    lif_read_cb_t cb;
    void *ctxt;
} pds_lif_read_args_t;

bool
pds_lif_spec_from_impl (void *entry, void *ctxt)
{
    api::impl::lif_impl *lif = (api::impl::lif_impl *)entry;
    pds_lif_read_args_t *args = (pds_lif_read_args_t *)ctxt;
    pds_lif_info_t info = {0};

    info.spec.key = lif->key();
    info.spec.pinned_ifidx = lif->pinned_ifindex();
    info.spec.type = lif->type();
    strncpy(info.status.name, lif->name(), SDK_MAX_NAME_LEN);
    info.status.state = lif_state_to_if_state(lif->state());
    info.status.ifindex = LIF_IFINDEX(info.spec.key);
    
    args->cb(&info, args->ctxt);
    return false;
}

sdk_ret_t
pds_lif_read_all (lif_read_cb_t cb, void *ctxt)
{
    pds_lif_read_args_t args = {0};
    args.ctxt = ctxt;
    args.cb = cb;

    return lif_db()->walk(pds_lif_spec_from_impl, &args);
}
