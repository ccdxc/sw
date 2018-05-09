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
#include "port_list.hpp"

namespace hal {

static inline port_list_elem_t *
port_list_elem_alloc ()
{
    return ((port_list_elem_t *)g_hal_state->
                port_list_elem_slab()->alloc());
}

static inline void
port_list_elem_free (port_list_elem_t *port)
{
    hal::delay_delete_to_slab(HAL_SLAB_PORT_LIST_ELEM, port);
}

static inline void
port_list_elem_init (port_list_elem_t *port)
{
    dllist_reset(&port->list_ctxt);
}

static inline void
port_list_elem_uninit (port_list_elem_t *port)
{
}

static inline port_list_elem_t *
port_list_elem_alloc_init ()
{
    port_list_elem_t *port;

    if ((port = port_list_elem_alloc()) ==  NULL)
        return NULL;

    port_list_elem_init(port);
    return port;
}

static inline void
port_list_elem_uninit_free (port_list_elem_t *port)
{
    if (port) {
        port_list_elem_uninit(port);
        port_list_elem_free(port);
    }
}

static inline void
port_list_elem_db_add (dllist_ctxt_t *head, port_list_elem_t *port)
{
    dllist_add_tail(head, &port->list_ctxt);
}

static inline void
port_list_elem_db_del (port_list_elem_t *port)
{
    dllist_del(&port->list_ctxt);
}

static inline hal_ret_t
port_list_elem_l4portrange_spec_extract (const types::L4PortRange& port,
                                         port_list_elem_t *port_lelem)
{
    port_lelem->port_range.port_hi = port.port_high();
    port_lelem->port_range.port_lo = port.port_low();
    return HAL_RET_OK;
}

hal_ret_t
port_list_elem_l4portrange_spec_handle (const types::L4PortRange& port,
                                        dllist_ctxt_t *head)
{
    hal_ret_t ret;
    port_list_elem_t *port_lelem;

    if ((port_lelem = port_list_elem_alloc_init()) == NULL)
        return HAL_RET_OOM;

    if ((ret = port_list_elem_l4portrange_spec_extract(
            port, port_lelem)) != HAL_RET_OK) {
        port_list_elem_uninit_free(port_lelem);
        return ret;
    }

    port_list_elem_db_add(head, port_lelem);
    return HAL_RET_OK;
}

hal_ret_t
port_list_elem_dst_port_spec_build (dllist_ctxt_t *head,
                                    types::RuleMatch_L4PortAppInfo *port_info)
{
    dllist_ctxt_t *entry;
    port_list_elem_t *port;
    types::L4PortRange *port_range;

    dllist_for_each(entry, head) {
        port = dllist_entry(entry, port_list_elem_t, list_ctxt);
        port_range = port_info->add_dst_port_range();
        port_range->set_port_high(port->port_range.port_hi);
        port_range->set_port_low(port->port_range.port_lo);
    }

    return HAL_RET_OK;
}

hal_ret_t
port_list_elem_src_port_spec_build (dllist_ctxt_t *head,
                                    types::RuleMatch_L4PortAppInfo *port_info)
{
    dllist_ctxt_t *entry;
    port_list_elem_t *port;
    types::L4PortRange *port_range;

    dllist_for_each(entry, head) {
        port = dllist_entry(entry, port_list_elem_t, list_ctxt);
        port_range = port_info->add_src_port_range();
        port_range->set_port_high(port->port_range.port_hi);
        port_range->set_port_low(port->port_range.port_lo);
    }

    return HAL_RET_OK;
}

void
port_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t *curr, *next;
    port_list_elem_t *port;

    dllist_for_each_safe(curr, next, head) {
        port = dllist_entry(curr, port_list_elem_t, list_ctxt);
        port_list_elem_db_del(port);
        port_list_elem_free(port);
    }
}

} // namespace hal
