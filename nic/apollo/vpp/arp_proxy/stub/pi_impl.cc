/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <nic/sdk/include/sdk/table.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <nic/apollo/vpp/arp_proxy/pi_impl.h>

extern "C" {
int
egress_vnic_read(int vnic_id, vnic_t *vnic)
{
    return 0;
}

bool
is_vlan_pkt(uint32_t vnic_id)
{
    return 0;
}

int
vr_mac_read(uint32_t vnic_id, uint8_t * mac)
{
    return 0;
}

}
