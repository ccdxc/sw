//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/interface_funcs.h>
#include "intf.h"

u32
pds_infra_get_sw_ifindex_by_name (u8 *intf)
{
    uword *p;
    u32 hw_if_index;
    vnet_hw_interface_t *hi;
    vnet_main_t *vnm = vnet_get_main();
    u8 *if_name = format(0, "%s", intf);
    u32 sw_if_index = ~0;

    if (((p = hash_get_mem(vnm->interface_main.hw_interface_by_name,
                           if_name)))) {
        hw_if_index = p[0];
        hi = vnet_get_hw_interface(vnm, hw_if_index);
        sw_if_index = hi->sw_if_index;
    }
    vec_free(if_name);
    return sw_if_index;
}


static walk_rc_t
set_intf_admin_state (vnet_main_t *vnm, u32 hw_if_index, void *args)
{
    u8                  admin_up = *(u8 *)args;
    vnet_hw_interface_t *hi;

    hi = vnet_get_hw_interface(vnm, hw_if_index);
    if (admin_up) {
        vnet_sw_interface_admin_up(vnm, hi->sw_if_index);
    } else {
        vnet_sw_interface_admin_down(vnm, hi->sw_if_index);
    }
    return WALK_CONTINUE;
}

void
pds_infra_set_all_intfs_status (u8 admin_up)
{
    vnet_hw_interface_walk(vnet_get_main(), set_intf_admin_state, &admin_up);
}

