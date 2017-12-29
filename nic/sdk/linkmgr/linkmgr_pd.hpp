// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_PD_HPP__
#define __LINKMGR_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

#include "nic/gen/proto/hal/port.pb.h"

using port::PortType;
using port::PortSpeed;
using port::PortAdminState;
using port::PortOperStatus;

namespace linkmgr {
namespace pd {

typedef struct port_args_pd_s {
    void             *pd_p;
    PortType         port_type;               // port type
    PortSpeed        port_speed;              // port speed
    PortAdminState   admin_state;             // admin state of the port
    PortOperStatus   oper_status;             // oper status of the port
    uint32_t         mac_id;                  // mac id associated with the port
    uint32_t         mac_ch;                  // mac channel associated with the port
    uint32_t         num_lanes;               // number of lanes for the port
} __PACK__ port_args_pd_t;

void*     port_create_pd     (port_args_pd_t *port_args_pd);
hal_ret_t port_update_pd     (port_args_pd_t *port_args_pd);
hal_ret_t port_delete_pd     (port_args_pd_t *port_args_pd);
hal_ret_t port_get_pd        (port_args_pd_t *port_args_pd);
hal_ret_t port_mem_free_pd   (port_args_pd_t *port_args_pd);
void*     port_make_clone_pd (void *pd_p);

bool  port_has_speed_changed_pd
                     (port_args_pd_t *port_args_pd);
bool  port_has_admin_state_changed_pd
                     (port_args_pd_t *port_args_pd);

// create linkmgr pd state to allocate port pd structures
hal_ret_t linkmgr_init_pd();

hal_ret_t port_event_timer_pd  (void *ctxt);
hal_ret_t port_event_enable_pd (void *ctxt);
hal_ret_t port_event_disable_pd(void *ctxt);

static inline void
port_args_init_pd (port_args_pd_t *args)
{
    args->pd_p = NULL;
    return;
}

}   // namespace pd
}   // namespace linkmgr

#endif    // __LINKMGR_PD_HPP__
