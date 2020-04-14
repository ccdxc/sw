//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_DHCP_RELAY_H__
#define __VPP_IMPL_APULU_DHCP_RELAY_H__

#include <nic/apollo/api/impl/apulu/nacl_data.h>
#include <nic/apollo/p4/include/apulu_defines.h>
#include <nic/vpp/infra/api/intf.h>
#include <api.h>
#include <impl_db.h>

#ifdef __cplusplus
extern "C" {
#endif

// FIXME : temp until reinject to linux comes in
#define PDS_DHCP_SERVER_NEXTHOP 9

always_inline void
pds_dhcp_relay_fill_data (vlib_buffer_t *p, p4_rx_cpu_hdr_t *hdr)
{
    vnet_buffer(p)->pds_dhcp_data.vnic_id = hdr->vnic_id;
    vnet_buffer(p)->pds_dhcp_data.vpc_id = hdr->vpc_id;
    vnet_buffer(p)->pds_dhcp_data.lif = hdr->lif;
    vnet_buffer(p)->pds_dhcp_data.bd_id = hdr->ingress_bd_id;
    return;
}

always_inline void
pds_dhcp_relay_svr_fill_tx_hdr_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1)
{
    p4_tx_cpu_hdr_t *tx0, *tx1;

    tx0 = vlib_buffer_get_current(b0);
    tx1 = vlib_buffer_get_current(b1);

    tx0->lif_flags = 0;
    tx0->lif_flags = 0;

    tx0->nexthop_valid = 1;
    tx1->nexthop_valid = 1;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);
    tx1->lif_flags = clib_host_to_net_u16(tx1->lif_flags);

    tx0->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    tx1->nexthop_type = NEXTHOP_TYPE_NEXTHOP;

    tx0->nexthop_id = PDS_DHCP_SERVER_NEXTHOP;
    tx1->nexthop_id = PDS_DHCP_SERVER_NEXTHOP;
    tx0->nexthop_id = clib_host_to_net_u16(tx0->nexthop_id);
    tx1->nexthop_id = clib_host_to_net_u16(tx1->nexthop_id);

    vnet_buffer(b0)->sw_if_index[VLIB_TX] =
        vnet_buffer(b0)->sw_if_index[VLIB_RX];
    vnet_buffer(b1)->sw_if_index[VLIB_TX] =
        vnet_buffer(b1)->sw_if_index[VLIB_RX];

    return;
}

always_inline void
pds_dhcp_relay_svr_fill_tx_hdr_x1 (vlib_buffer_t *b0)
{
    p4_tx_cpu_hdr_t *tx0;

    tx0 = vlib_buffer_get_current(b0);


    tx0->lif_flags = 0;

    tx0->nexthop_valid = 1;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);

    tx0->nexthop_type = NEXTHOP_TYPE_NEXTHOP;

    tx0->nexthop_id = PDS_DHCP_SERVER_NEXTHOP;
    tx0->nexthop_id = clib_host_to_net_u16(tx0->nexthop_id);

    vnet_buffer(b0)->sw_if_index[VLIB_TX] =
        vnet_buffer(b0)->sw_if_index[VLIB_RX];

    return;
}

always_inline void
pds_dhcp_relay_client_fill_tx_hdr_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1,
                                      bool *error0, bool *error1)
{
    p4_tx_cpu_hdr_t *tx0, *tx1;
    static u32 cpu_mnic_if_index = ~0;

    *error0 = false;
    *error1 = false;

    pds_impl_db_vnic_entry_t *vnic_info0, *vnic_info1;

    vnic_info0 = pds_impl_db_vnic_get(vnet_buffer(b0)->pds_dhcp_data.vnic_id);
    if (!vnic_info0) {
        *error0 = true;
    }
    vnic_info1 = pds_impl_db_vnic_get(vnet_buffer(b1)->pds_dhcp_data.vnic_id);
    if (!vnic_info1) {
        *error1 = true;
    }

    tx0 = vlib_buffer_get_current(b0);
    tx1 = vlib_buffer_get_current(b1);

    tx0->lif_flags = 0;
    tx1->lif_flags = 0;

    tx0->nexthop_valid = 1;
    tx1->nexthop_valid = 1;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);
    tx1->lif_flags = clib_host_to_net_u16(tx1->lif_flags);

    tx0->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    tx1->nexthop_type = NEXTHOP_TYPE_NEXTHOP;

    if (vnic_info0) {
        tx0->nexthop_id = vnic_info0->nh_hw_id;
    }
    if (vnic_info1) {
        tx1->nexthop_id = vnic_info1->nh_hw_id;
    }

    tx0->nexthop_id = clib_host_to_net_u16(tx0->nexthop_id);
    tx1->nexthop_id = clib_host_to_net_u16(tx1->nexthop_id);

    if (((u32) ~0) == cpu_mnic_if_index) {
        cpu_mnic_if_index = pds_infra_get_sw_ifindex_by_name((u8*)"cpu_mnic0");
    }
    vnet_buffer(b0)->sw_if_index[VLIB_TX] = cpu_mnic_if_index;
    vnet_buffer(b1)->sw_if_index[VLIB_TX] = cpu_mnic_if_index;
    return;
}

always_inline void
pds_dhcp_relay_client_fill_tx_hdr_x1 (vlib_buffer_t *b0, bool *error0)
{
    p4_tx_cpu_hdr_t *tx0;
    static u32 cpu_mnic_if_index = ~0;
    pds_impl_db_vnic_entry_t *vnic_info0;

    vnic_info0 = pds_impl_db_vnic_get(vnet_buffer(b0)->pds_dhcp_data.vnic_id);
    if (!vnic_info0) {
        *error0 = true;
        return;
    }

    *error0 = false;

    tx0 = vlib_buffer_get_current(b0);

    tx0->lif_flags = 0;

    tx0->nexthop_valid = 1;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);

    tx0->nexthop_type = NEXTHOP_TYPE_NEXTHOP;

    tx0->nexthop_id = vnic_info0->nh_hw_id;

    tx0->nexthop_id = clib_host_to_net_u16(tx0->nexthop_id);

    if (((u32) ~0) == cpu_mnic_if_index) {
        cpu_mnic_if_index = pds_infra_get_sw_ifindex_by_name((u8*)"cpu_mnic0");
    }
    vnet_buffer(b0)->sw_if_index[VLIB_TX] = cpu_mnic_if_index;
    return;
}


always_inline void
pds_dhcp_relay_fill_subnet_info(uint32_t vnic_id, uint32_t *subnet_pfx,
                                uint32_t *subnet_ip)
{
    pds_impl_db_vnic_entry_t *vnic_info;
    pds_impl_db_subnet_entry_t *subnet_info;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if (vnic_info) {
        u32 subnet_id = vnic_info->subnet_hw_id;

        subnet_info = pds_impl_db_subnet_get(subnet_id);
        if (subnet_info) {
            *subnet_ip = subnet_info->vr_ip.ip4.as_u32;
            *subnet_pfx = *subnet_ip & (0xffffffff << (32 - subnet_info->prefix_len) );

            *subnet_pfx = htonl(*subnet_pfx);
            *subnet_ip = htonl(*subnet_ip);
        }
    }

    return;
}

always_inline void
pds_dhcp_relay_pipeline_init (void)
{
    pds_infra_api_reg_t params = {0};

    params.nacl_id = NACL_DATA_ID_FLOW_MISS_DHCP_HOST;
    params.node = format(0, "pds-dhcp-relay-host-classify");
    params.frame_queue_index = ~0;
    params.handoff_thread = ~0;
    params.offset = 0;
    params.unreg = 0;

    if (0 != pds_register_nacl_id_to_node(&params)) {
        ASSERT(0);
    }

    params.nacl_id = NACL_DATA_ID_FLOW_MISS_DHCP_UPLINK;
    params.node = format(0, "pds-dhcp-relay-uplink-classify");
    if (0 != pds_register_nacl_id_to_node(&params)) {
        ASSERT(0);
    }
    return;
}

#ifdef __cplusplus
}
#endif
#endif      //__VPP_IMPL_APULU_DHCP_RELAY_H__
