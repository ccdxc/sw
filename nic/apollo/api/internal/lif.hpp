//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements internal lif related helper APIs
///
//----------------------------------------------------------------------------

#ifndef __INTERNAL_LIF_HPP__
#define __INTERNAL_LIF_HPP__

//#include "nic/sdk/include/sdk/if.hpp"
//#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
//#include "nic/apollo/api/include/pds_lif.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"

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

static inline sdk_ret_t
pds_lif_to_lif_status (pds_lif_status_t *status, api::impl::lif_impl *lif)
{
    strncpy(status->name, lif->name(), SDK_MAX_NAME_LEN);
    status->state = lif_state_to_if_state(lif->state());
    status->ifindex = lif->ifindex();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_lif_to_lif_spec (pds_lif_spec_t *spec, api::impl::lif_impl *lif)
{
    spec->key = lif->key();
    spec->id = lif->id();
    spec->pinned_ifidx = lif->pinned_ifindex();
    spec->type = lif->type();
    memcpy(spec->mac, lif->mac(), ETH_ADDR_LEN);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_lif_to_lif_info (pds_lif_info_t *info, api::impl::lif_impl *lif) {
    pds_lif_to_lif_spec(&info->spec, lif);
    pds_lif_to_lif_status(&info->status, lif);

    return SDK_RET_OK;
}

#endif    // __INTERNAL_LIF_HPP__
