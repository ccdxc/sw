// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __DEBUG_PD_HPP__
#define __DBEUG_PD_HPP__

#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

using hal::pd::utils::acl_tcam_entry_handle_t;

namespace hal {
namespace pd {

struct pd_fte_span_s {
    acl_tcam_entry_handle_t handle;
    uint32_t         stats_index;
    uint32_t         mirr_sess_id;

    fte_span_t      *fte_span;
} __PACK__;

static inline pd_fte_span_t *
fte_span_pd_alloc (void)
{
    pd_fte_span_t    *fte_span_pd;

    fte_span_pd = (pd_fte_span_t *)g_hal_state_pd->fte_span_slab()->alloc();
    if (fte_span_pd == NULL) {
        return NULL;
    }

    return fte_span_pd;
}

static inline pd_fte_span_t *
fte_span_pd_init (pd_fte_span_t *fte_span_pd)
{
    if (!fte_span_pd) {
        return NULL;
    }

    fte_span_pd->handle = INVALID_INDEXER_INDEX;
    fte_span_pd->mirr_sess_id = INVALID_INDEXER_INDEX;
    fte_span_pd->fte_span = NULL;

    return fte_span_pd;
}

static inline pd_fte_span_t *
fte_span_pd_alloc_init (void)
{
    return fte_span_pd_init(fte_span_pd_alloc());
}

static inline hal_ret_t
fte_span_pd_mem_free (pd_fte_span_t *fte_span)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_FTE_SPAN_PD, fte_span);
    return HAL_RET_OK;
}

static inline hal_ret_t
fte_span_pd_free (pd_fte_span_t *fte_span)
{
    fte_span_pd_mem_free(fte_span);
    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal

#endif    // __DEBUG_PD_HPP__
