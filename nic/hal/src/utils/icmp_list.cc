//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::Address & types::IPAddressObj object
//-----------------------------------------------------------------------------

//#include <google/protobuf/util/json_util.h>
#include "nic/include/base.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "utils.hpp"
#include "icmp_list.hpp"

namespace hal {

static inline icmp_list_elem_t *
icmp_list_elem_alloc ()
{
    return ((icmp_list_elem_t *)g_hal_state->
                icmp_list_elem_slab()->alloc());
}

static inline void
icmp_list_elem_free (icmp_list_elem_t *icmp)
{
    hal::delay_delete_to_slab(HAL_SLAB_SG_LIST_ELEM, icmp);
}

static inline void
icmp_list_elem_init (icmp_list_elem_t *icmp)
{
    dllist_reset(&icmp->list_ctxt);
}

static inline icmp_list_elem_t *
icmp_list_elem_alloc_init ()
{
    icmp_list_elem_t *icmp;

    if ((icmp = icmp_list_elem_alloc()) ==  NULL)
        return NULL;

    icmp_list_elem_init(icmp);
    return icmp;
}

static inline void
icmp_list_elem_db_add (dllist_ctxt_t *head, icmp_list_elem_t *icmp)
{
    dllist_add_tail(head, &icmp->list_ctxt);
}

static inline void
icmp_list_elem_db_del (icmp_list_elem_t *icmp)
{
    dllist_del(&icmp->list_ctxt);
}

hal_ret_t
icmp_list_elem_icmp_spec_handle (const uint32_t icmp_type,
                                 const uint32_t icmp_code,
                                 dllist_ctxt_t  *head)
{
    icmp_list_elem_t *icmp_lelem;

    if ((icmp_lelem = icmp_list_elem_alloc_init()) == NULL)
        return HAL_RET_OOM;

    icmp_lelem->icmp_type = icmp_type;
    icmp_lelem->icmp_code = icmp_code;

    icmp_list_elem_db_add(head, icmp_lelem);
    return HAL_RET_OK;
}

#if 0
hal_ret_t
icmp_list_elem_dst_icmp_spec_build (dllist_ctxt_t *head,
                                    types::RuleMatch_L4PortAppInfo *icmp_info)
{
    dllist_ctxt_t *entry;
    icmp_list_elem_t *icmp;
    types::L4PortRange *icmp_range;

    dllist_for_each(entry, head) {
        icmp = dllist_entry(entry, icmp_list_elem_t, list_ctxt);
        icmp_range = icmp_info->add_dst_icmp_range();
        icmp_range->set_icmp_high(icmp->icmp_range.icmp_hi);
        icmp_range->set_icmp_low(icmp->icmp_range.icmp_lo);
    }

    return HAL_RET_OK;
}

hal_ret_t
icmp_list_elem_src_icmp_spec_build (dllist_ctxt_t *head,
                                    types::RuleMatch_L4PortAppInfo *icmp_info)
{
    dllist_ctxt_t  *entry;
    icmp_list_elem_t *icmp;
    types::L4PortRange *icmp_range;

    dllist_for_each(entry, head) {
        icmp = dllist_entry(entry, icmp_list_elem_t, list_ctxt);
        icmp_range = icmp_info->add_src_icmp_range();
        icmp_range->set_icmp_high(icmp->icmp_range.icmp_hi);
        icmp_range->set_icmp_low(icmp->icmp_range.icmp_lo);
    }

    return HAL_RET_OK;
}
#endif

void
icmp_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t *curr, *next;
    icmp_list_elem_t *icmp;

    dllist_for_each_safe(curr, next, head) {
        icmp = dllist_entry(curr, icmp_list_elem_t, list_ctxt);
        icmp_list_elem_db_del(icmp);
        icmp_list_elem_free(icmp);
    }
}

} // namespace hal
