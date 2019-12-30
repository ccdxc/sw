//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// This file contains p4 header interface for rx and tx packets

#ifndef __VPP_IMPL_APULU_P4_CPU_HDR_UTILS_H__
#define __VPP_IMPL_APULU_P4_CPU_HDR_UTILS_H__

#include <nic/apollo/packet/apulu/p4_cpu_hdr.h>
#include <nic/apollo/p4/include/apulu_defines.h>
#include <vppinfra/format.h>

#define VPP_CPU_FLAGS_VLAN_VALID           APULU_CPU_FLAGS_VLAN_VALID
#define VPP_CPU_FLAGS_IPV4_1_VALID         APULU_CPU_FLAGS_IPV4_1_VALID
#define VPP_CPU_FLAGS_IPV6_1_VALID         APULU_CPU_FLAGS_IPV6_1_VALID
#define VPP_CPU_FLAGS_ETH_2_VALID          APULU_CPU_FLAGS_ETH_2_VALID
#define VPP_CPU_FLAGS_IPV4_2_VALID         APULU_CPU_FLAGS_IPV4_2_VALID
#define VPP_CPU_FLAGS_IPV6_2_VALID         APULU_CPU_FLAGS_IPV6_2_VALID
#define VPP_ARM_TO_P4_HDR_SZ               APULU_ARM_TO_P4_HDR_SZ
#define VPP_P4_TO_ARM_HDR_SZ               APULU_P4_TO_ARM_HDR_SZ
always_inline void
pds_get_nacl_data_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *nacl_data0, u16 *nacl_data1)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);

    *nacl_data0 = hdr0->nacl_data;
    *nacl_data1 = hdr1->nacl_data;
    return;
}

always_inline void
pds_get_nacl_data_x1 (vlib_buffer_t *p, u16 *nacl_data)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);

    *nacl_data = hdr->nacl_data;
    return;
}

u8 *
format_pds_p4_rx_cpu_hdr (u8 * s, va_list * args)
{
    p4_rx_cpu_hdr_t *t = va_arg (*args, p4_rx_cpu_hdr_t *);

    s = format(s, "PacketLen[%u], Flags[0x%x], NACL-data[%d], ingress_bd_id[%d],"
               " flow_hash[0x%x], l2_offset[%d], "
               "\n\tl3_offset[%d] l4_offset[%d], l2_inner_offset[%d],"
               " l3_inner_offset[%d], "
               "\n\tl4_inner_offset[%d], payload_offset[%d], lif[%d],"
               " egress_bd_id[%d], "
               "\n\tservice_xlate_id[%d], mapping_xlate_id[%d],"
               " tx_meter_id[%d],"
               "\n\tnexthop_id[%d], vpc_id[%d], vnic_id[%d],"
               " flags_octet[0x%x]",
               t->packet_len, t->flags, t->nacl_data, t->ingress_bd_id, t->flow_hash,
               t->l2_offset, t->l3_offset, t->l4_offset, t->l2_inner_offset,
               t->l3_inner_offset, t->l4_inner_offset, t->payload_offset,
               t->lif, t->egress_bd_id, t->service_xlate_id, t->mapping_xlate_id,
               t->tx_meter_id, t->nexthop_id, t->vpc_id, t->vnic_id,
               t->flags_octet);
    return s;
}

u8 *
format_pds_p4_tx_cpu_hdr (u8 * s, va_list * args)
{
    p4_tx_cpu_hdr_t *t = va_arg (*args, p4_tx_cpu_hdr_t *);

    s = format(s, "lif[0x%x], nhop_valid[%d], nhop_type[%d], nhop_id[%d]",
               t->lif_sbit0_ebit7 | (t->lif_sbit8_ebit10 << 8),
               t->nexthop_valid, t->nexthop_type, t->nexthop_id);
    return s;
}

#endif     // __VPP_IMPL_APULU_P4_CPU_HDR_UTILS_H__
