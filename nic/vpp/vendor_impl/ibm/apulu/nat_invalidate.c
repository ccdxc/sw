//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vppinfra/clib.h>
#include <vlib/vlib.h>
#include <vnet/plugin/plugin.h>
#include <vnet/vxlan/vxlan_packet.h>
#include <nic/vpp/infra/utils.h>
#include "nat/nat_api.h"
#include "impl_db.h"

#define IBM_NAT_INVALIDATE_RESERVED_VNID 23

static void (*nat_invalidate_register_fn)(nat_vendor_invalidate_cb cb);

static int
ibm_nat_invalidate(vlib_buffer_t *p, u16 *next)
{
    ip4_header_t *ip4;
    udp_header_t *udp;
    vxlan_header_t *vxlan;
    ip4_address_t temp_ip;
    u32 vni;

    // We are pointing to inner ip header, rewind to go to outer
    vlib_buffer_advance(p, vnet_buffer(p)->l3_hdr_offset);
    ip4 = vlib_buffer_get_current(p);

    temp_ip = ip4->src_address;
    ip4->src_address = ip4->dst_address;
    ip4->dst_address = temp_ip;

    udp = (udp_header_t *)(ip4 + 1);
    vxlan = (vxlan_header_t *)(udp + 1);
    vni = clib_net_to_host_u32(vxlan->vni_reserved) >> 8;
    if (vni == IBM_NAT_INVALIDATE_RESERVED_VNID) {
        // This is a returned packet, drop it so there isn't a loop
        return -1;
    }
    vxlan->vni_reserved = clib_host_to_net_u32(IBM_NAT_INVALIDATE_RESERVED_VNID << 8);
    return 0;
}

static clib_error_t *
ibm_nat_invalidate_init (vlib_main_t *vm)
{
    nat_invalidate_register_fn = vlib_get_plugin_symbol(
            "nat.so",
            "nat_register_vendor_invalidate_cb");
    nat_invalidate_register_fn(ibm_nat_invalidate);
    return 0;
}

VLIB_INIT_FUNCTION(ibm_nat_invalidate_init) =
{
    .runs_after = VLIB_INITS("vpp_nat_init"),
};

