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

sdk_ret_t
pds_lif_read (pds_lif_key_t *key, pds_lif_spec_t *spec)
{
    api::impl::lif_impl *lif = (api::impl::lif_impl *)(lif_db()->find(key));
    if (lif == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    spec->key = lif->key();
    spec->pinned_ifidx = lif->pinned_ifindex();
    spec->type = lif->type();
    return SDK_RET_OK;
}

typedef struct pds_lif_read_args_s
{
    lif_read_cb_t cb;
    void *ctxt;
} pds_lif_read_args_t;

bool
pds_lif_spec_from_impl (void *entry, void *ctxt)
{
    api::impl::lif_impl *lif = (api::impl::lif_impl *)entry;
    pds_lif_read_args_t *args = (pds_lif_read_args_t *)ctxt;
    pds_lif_spec_t spec = {0};

    spec.key = lif->key();
    spec.pinned_ifidx = lif->pinned_ifindex();
    spec.type = lif->type();

    args->cb(&spec, args->ctxt);
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
