//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_DHCP_RELAY_H__
#define __VPP_IMPL_APULU_DHCP_RELAY_H__

#include <nic/apollo/api/impl/apulu/nacl_data.h>
#include <api.h>

#ifdef __cplusplus
extern "C" {
#endif

always_inline int
pds_dhcp_relay_clfy_buffer_advance_offset (vlib_buffer_t *b)
{
    return (VPP_P4_TO_ARM_HDR_SZ +
            vnet_buffer(b)->l4_hdr_offset - vnet_buffer (b)->l2_hdr_offset +
            sizeof(udp_header_t));
}

always_inline void
pds_dhcp_relay_clfy_fill_next (u16 *next, p4_rx_cpu_hdr_t *hdr, u32 *counter)
{
    if(1) {
       /* TODO cehck the source IP in dhcp server db if not send to server*/
        *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_SERVER;
        counter[DHCP_RELAY_CLFY_COUNTER_TO_SERVER]++;

    } else if (0) {
	// TODO if source is  dhcp server send to client
        *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_CLIENT;
        counter[DHCP_RELAY_CLFY_COUNTER_TO_CLIENT]++;
    }

    return;
}

always_inline void
pds_dhcp_relay_fill_data (vlib_buffer_t *p, p4_rx_cpu_hdr_t *hdr)
{
    vnet_buffer(p)->pds_dhcp_data.vnic_id = hdr->vnic_id;
    vnet_buffer(p)->pds_dhcp_data.vpc_id = hdr->vpc_id;
    vnet_buffer(p)->pds_dhcp_data.lif = hdr->lif;
    vnet_buffer(p)->pds_dhcp_data.bd_id = hdr->egress_bd_id;
    return;
}

always_inline void
pds_dhcp_relay_clfy_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);

    vnet_buffer (p0)->l2_hdr_offset = hdr0->l2_offset;
    vnet_buffer (p0)->l3_hdr_offset =
         hdr0->l3_inner_offset ? hdr0->l3_inner_offset : hdr0->l3_offset;
    vnet_buffer (p0)->l4_hdr_offset =
        hdr0->l4_inner_offset ? hdr0->l4_inner_offset : hdr0->l4_offset;

    vnet_buffer (p1)->l2_hdr_offset = hdr1->l2_offset;
    vnet_buffer (p1)->l3_hdr_offset =
         hdr1->l3_inner_offset ? hdr1->l3_inner_offset : hdr1->l3_offset;
    vnet_buffer (p1)->l4_hdr_offset =
        hdr1->l4_inner_offset ? hdr1->l4_inner_offset : hdr1->l4_offset;


    pds_dhcp_relay_fill_data(p0, hdr0);
    pds_dhcp_relay_fill_data(p1, hdr1);

    vlib_buffer_advance(p0, pds_dhcp_relay_clfy_buffer_advance_offset(p0));
    vlib_buffer_advance(p1, pds_dhcp_relay_clfy_buffer_advance_offset(p1));

    pds_dhcp_relay_clfy_fill_next(next0, hdr0, counter);
    pds_dhcp_relay_clfy_fill_next(next1, hdr1, counter);

    return;
}

always_inline void
pds_dhcp_relay_clfy_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);

    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset =
         hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset =
        hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    pds_dhcp_relay_fill_data(p, hdr);

    vlib_buffer_advance(p, pds_dhcp_relay_clfy_buffer_advance_offset(p));

    pds_dhcp_relay_clfy_fill_next(next, hdr, counter);

    return;
}

always_inline void
pds_dhcp_relay_pipeline_init (void)
{
    pds_infra_api_reg_t params = {0};

    params.nacl_id = NACL_DATA_ID_FLOW_MISS_DHCP;
    params.node = format(0, "pds-dhcp-relay-classify");
    params.frame_queue_index = ~0;
    params.handoff_thread = ~0;
    params.offset = 0;
    params.unreg = 0;

    if (0 != pds_register_nacl_id_to_node(&params)) {
        ASSERT(0);
    }
    return;
}

#ifdef __cplusplus
}
#endif
#endif      //__VPP_IMPL_APULU_DHCP_RELAY_H__
