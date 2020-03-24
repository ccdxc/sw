//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vppinfra/clib.h>
#include <vlib/vlib.h>
#include <vnet/plugin/plugin.h>
#include <nic/vpp/infra/utils.h>
#include "arp_proxy/arp_proxy.h"
#include "impl_db.h"

#define ARP_PROXY_IBM_RACK_SHIFT        14
#define ARP_PROXY_IBM_RACK_MASK         0x3ff
#define ARP_PROXY_IBM_SLED_SHIFT        8
#define ARP_PROXY_IBM_SLED_MASK         0x3f
#define ARP_PROXY_IBM_HOST_SHIFT        0
#define ARP_PROXY_IBM_HOST_MASK         0xff

#define ARP_PROXY_IBM_RACK_NUM(ip) \
    (((ip) >> ARP_PROXY_IBM_RACK_SHIFT) & ARP_PROXY_IBM_RACK_MASK)
#define ARP_PROXY_IBM_SLED_NUM(ip) \
    (((ip) >> ARP_PROXY_IBM_SLED_SHIFT) & ARP_PROXY_IBM_SLED_MASK)
#define ARP_PROXY_IBM_HOST_NUM(ip) \
    (((ip) >> ARP_PROXY_IBM_HOST_SHIFT) & ARP_PROXY_IBM_HOST_MASK)

#define ARP_PROXY_IBM_SET_MAC(mac, vnid, rack, sled, host) \
    mac_addr[0] = 0xfe;                                    \
    mac_addr[1] = 0xff;                                    \
    mac_addr[2] = vnid;                                    \
    mac_addr[3] = (rack >> 2) & 0xff;                      \
    mac_addr[4] = ((rack & 0x3) << 6) | (sled & 0x3f);     \
    mac_addr[5] = host;

static int
ibm_dst_mac_get(u16 vpc_id, u16 bd_id, mac_addr_t mac_addr,
                uint32_t dst_addr)
{
    pds_impl_db_subnet_entry_t *entry = pds_impl_db_subnet_get(bd_id);
    u32 vnid;
    u16 rack, sled, host;

    if (!entry) {
        return -1;
    }

    if (entry->vr_ip.ip4.data_u32 == dst_addr) {
        clib_memcpy(mac_addr, entry->mac, ETH_ADDR_LEN);
    } else {
        // vnid is shifted by 8 and then stored in big endian format
        // shift by 2 octets to get LSB
        // example 
        //      vnid = 00 00 00 0b
        //      vnid << 8 = 00 00 0b 00
        //      htonl(vnid << 8) = 00 0b 00 00
        vnid = entry->vnid >> 16;
        rack = ARP_PROXY_IBM_RACK_NUM(dst_addr);
        sled = ARP_PROXY_IBM_SLED_NUM(dst_addr);
        host = ARP_PROXY_IBM_HOST_NUM(dst_addr);

        ARP_PROXY_IBM_SET_MAC(mac_addr, vnid, rack, sled, host);
    }

    return 0;
}

static void (*dst_mac_get_register_fn)(arp_proxy_vendor_dst_mac_get_cb cb);

static clib_error_t *
ibm_arp_proxy_init (vlib_main_t *vm)
{
    dst_mac_get_register_fn = vlib_get_plugin_symbol(
            "arp_proxy.so",
            "arp_proxy_register_vendor_dst_mac_get_cb");
    dst_mac_get_register_fn(ibm_dst_mac_get);
    return 0;
}

VLIB_INIT_FUNCTION(ibm_arp_proxy_init) =
{
    .runs_after = VLIB_INITS("arp_proxy_init"),
};

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "IBM arp proxy plugin",
};
// *INDENT-ON*

