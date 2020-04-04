//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <stddef.h>
#include <vlib/vlib.h>
#include "impl_db.h"
#include "cli.h"

int
vpc_impl_db_dump ()
{
    vlib_main_t *vm = vlib_get_main();
    pds_impl_db_vpc_entry_t *vpc_info;
    u16 offset;
    u32 itr;

    PRINT_BUF_LINE(25);
    vlib_cli_output(vm, "%=5s%=10s%=10s\n","HwId", "HwBdId","Flags");
    PRINT_BUF_LINE(25);

    //loop through vpc db and print all
    vec_foreach_index(itr, impl_db_ctx.vpc_pool_idx)
    {
        offset = vec_elt(impl_db_ctx.vpc_pool_idx, itr);
        if (offset != 0xffff) {
            vpc_info = pool_elt_at_index(impl_db_ctx.vpc_pool_base, offset);
            vlib_cli_output(vm, "%=5d%=10d%=10f\n", itr, vpc_info->hw_bd_id,
                            vpc_info->flags);
        }
    }
    vlib_cli_output(vm, "VpcCount : %d", pool_elts(impl_db_ctx.vpc_pool_base));
    return 0;
}

int
vnic_impl_db_dump ()
{
    vlib_main_t *vm = vlib_get_main();
    pds_impl_db_vnic_entry_t *vnic_info;
    u16 offset;
    u32 itr;

    PRINT_BUF_LINE(113);
    vlib_cli_output(vm, "%=5s%=21s%=15s%=17s%=10s%=10s%=10s%=10s%=7s%=8s","HwId",
                    "MAC", "MaxSessions", "ActiveSesCount", "SubnethwId",
                    "FlowLogEn", "EncapType", "EncapLen", "VlanId",
                    "NhHwId");
    PRINT_BUF_LINE(113);

    vec_foreach_index(itr, impl_db_ctx.vnic_pool_idx)
    {
        offset = vec_elt(impl_db_ctx.vnic_pool_idx, itr);
        if (offset != 0xffff) {
            vnic_info = pool_elt_at_index(impl_db_ctx.vnic_pool_base, offset);
        vlib_cli_output(vm, "%=5d%=21U%=15d%=17d%=10d%=10d%=10d%=10d%=7d%=8d\n",
                        itr, format_ethernet_address, vnic_info->mac,
                        vnic_info->max_sessions, vnic_info->active_ses_count,
                        vnic_info->subnet_hw_id, vnic_info->flow_log_en,
                        vnic_info->enacp_type, vnic_info->l2_encap_len,
                        vnic_info->vlan_id, vnic_info->nh_hw_id);
        }
    }
    vlib_cli_output(vm, "VnicCount : %d", pool_elts(impl_db_ctx.vnic_pool_base));
    return 0;
}

int
subnet_impl_db_dump ()
{
    vlib_main_t *vm = vlib_get_main();
    pds_impl_db_subnet_entry_t *subnet_info;
    ip46_address_t ip;
    u16 offset;
    u32 itr;

    PRINT_BUF_LINE(62);
    vlib_cli_output(vm, "%=5s%=21s%=10s%=16s%=10s","HwId","MAC","PrefixLen", "VrIP", "Vnid");
    PRINT_BUF_LINE(62);

    vec_foreach_index(itr, impl_db_ctx.subnet_pool_idx)
    {
        offset = vec_elt(impl_db_ctx.subnet_pool_idx, itr);
        if (offset != 0xffff) {
            subnet_info = pool_elt_at_index(impl_db_ctx.subnet_pool_base,
                                            offset);
            ip = subnet_info->vr_ip;
            ip.ip4.as_u32 = clib_net_to_host_u32(ip.ip4.as_u32);
            vlib_cli_output(vm, "%=5d%=21U%=8d%=16U%=10d\n", itr,
                            format_ethernet_address, subnet_info->mac,
                            subnet_info->prefix_len,
                            format_ip46_address, &ip, IP46_TYPE_ANY,
                            (clib_net_to_host_u32(subnet_info->vnid) >> 8));
        }
    }
    vlib_cli_output(vm, "SubnetCount : %d",pool_elts(impl_db_ctx.subnet_pool_base));
    return 0;
}

int
device_impl_db_dump ()
{
    vlib_main_t *vm = vlib_get_main();
    pds_impl_db_device_entry_t *device;
    ip46_address_t ip;

    device = &impl_db_ctx.device;

    PRINT_BUF_LINE(57);
    vlib_cli_output(vm, "%=21s%=16s%=20s", "MAC","DeviceIp", "Flags");
    PRINT_BUF_LINE(57);

    ip = device->device_ip;
    ip.ip4.as_u32 = clib_net_to_host_u32(ip.ip4.as_u32);
    vlib_cli_output(vm, "%=21U%=16U%=20s\n",
                    format_ethernet_address, device->device_mac,
                    format_ip46_address, &ip, IP46_TYPE_ANY,
                    (device->overlay_routing_en ? "Overlay-Routing" : "-"));
    return 0;
}
