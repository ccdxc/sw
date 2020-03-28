//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_UTILS_H__
#define __VPP_IMPL_UTILS_H__

#include <vlib/vlib.h>
#include <vnet/vnet.h>

__clib_unused static u32
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

#endif    // __VPP_IMPL_UTILS_H__
