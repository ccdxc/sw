//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::Address & types::IPAddressObj object
//-----------------------------------------------------------------------------

//#include <google/protobuf/util/json_util.h>
#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "utils.hpp"
#include "sg_list.hpp"

namespace hal {

static inline sg_list_elem_t *
sg_list_elem_alloc ()
{
    return ((sg_list_elem_t *)g_hal_state->
                sg_list_elem_slab()->alloc());
}

static inline void
sg_list_elem_free (sg_list_elem_t *sg)
{
    hal::delay_delete_to_slab(HAL_SLAB_SG_LIST_ELEM, sg);
}

static inline void
sg_list_elem_init (sg_list_elem_t *sg)
{
    dllist_reset(&sg->list_ctxt);
}

static inline sg_list_elem_t *
sg_list_elem_alloc_init ()
{
    sg_list_elem_t *sg;

    if ((sg = sg_list_elem_alloc()) ==  NULL)
        return NULL;

    sg_list_elem_init(sg);
    return sg;
}

static inline void
sg_list_elem_db_add (dllist_ctxt_t *head, sg_list_elem_t *sg)
{
    dllist_add_tail(head, &sg->list_ctxt);
}

static inline void
sg_list_elem_db_del (sg_list_elem_t *sg)
{
    dllist_del(&sg->list_ctxt);
}

hal_ret_t
sg_list_elem_sg_spec_handle (const uint32_t sg_id,
                                   dllist_ctxt_t *head)
{
    sg_list_elem_t *sg_lelem;

    if ((sg_lelem = sg_list_elem_alloc_init()) == NULL)
        return HAL_RET_OOM;

    sg_lelem->sg_id = sg_id;

    sg_list_elem_db_add(head, sg_lelem);
    return HAL_RET_OK;
}

#if 0
hal_ret_t
sg_list_elem_dst_sg_spec_build (dllist_ctxt_t *head,
                                    types::RuleMatch_L4PortAppInfo *sg_info)
{
    dllist_ctxt_t *entry;
    sg_list_elem_t *sg;
    types::L4PortRange *sg_range;

    dllist_for_each(entry, head) {
        sg = dllist_entry(entry, sg_list_elem_t, list_ctxt);
        sg_range = sg_info->add_dst_sg_range();
        sg_range->set_sg_high(sg->sg_range.sg_hi);
        sg_range->set_sg_low(sg->sg_range.sg_lo);
    }

    return HAL_RET_OK;
}

hal_ret_t
sg_list_elem_src_sg_spec_build (dllist_ctxt_t *head,
                                    types::RuleMatch_L4PortAppInfo *sg_info)
{
    dllist_ctxt_t  *entry;
    sg_list_elem_t *sg;
    types::L4PortRange *sg_range;

    dllist_for_each(entry, head) {
        sg = dllist_entry(entry, sg_list_elem_t, list_ctxt);
        sg_range = sg_info->add_src_sg_range();
        sg_range->set_sg_high(sg->sg_range.sg_hi);
        sg_range->set_sg_low(sg->sg_range.sg_lo);
    }

    return HAL_RET_OK;
}
#endif

void
sg_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t *curr, *next;
    sg_list_elem_t *sg;

    dllist_for_each_safe(curr, next, head) {
        sg = dllist_entry(curr, sg_list_elem_t, list_ctxt);
        sg_list_elem_db_del(sg);
        sg_list_elem_free(sg);
    }
}

} // namespace hal
