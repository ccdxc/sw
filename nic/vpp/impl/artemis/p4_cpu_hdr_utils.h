//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// This file contains p4 header interface for rx and tx packets

#ifndef __VPP_IMPL_ARTEMIS_P4_CPU_HDR_UTILS_H__
#define __VPP_IMPL_ARTEMIS_P4_CPU_HDR_UTILS_H___

#include <nic/apollo/p4/include/artemis_defines.h>
#include <nic/apollo/packet/artemis/p4_cpu_hdr.h>
#include <vppinfra/format.h>

#define VPP_CPU_FLAGS_VLAN_VALID           ARTEMIS_CPU_FLAGS_VLAN_VALID
#define VPP_CPU_FLAGS_IPV4_1_VALID         ARTEMIS_CPU_FLAGS_IPV4_1_VALID
#define VPP_CPU_FLAGS_IPV6_1_VALID         ARTEMIS_CPU_FLAGS_IPV6_1_VALID
#define VPP_CPU_FLAGS_ETH_2_VALID          ARTEMIS_CPU_FLAGS_ETH_2_VALID
#define VPP_CPU_FLAGS_IPV4_2_VALID         ARTEMIS_CPU_FLAGS_IPV4_2_VALID
#define VPP_CPU_FLAGS_IPV6_2_VALID         ARTEMIS_CPU_FLAGS_IPV6_2_VALID
#define VPP_CPU_FLAGS_DIRECTION            ARTEMIS_CPU_FLAGS_DIRECTION
#define VPP_ARM_TO_P4_HDR_SZ               ARTEMIS_PREDICATE_HDR_SZ
#define VPP_P4_TO_ARM_HDR_SZ               ARTEMIS_P4_TO_ARM_HDR_SZ

u8 *
format_pds_p4_rx_cpu_hdr (u8 * s, va_list * args)
{
    p4_rx_cpu_hdr_t *t = va_arg (*args, p4_rx_cpu_hdr_t *);

    s = format(s, "PacketLen[%u], Flags[0x%x], vnic[%d], flow_hash[0x%x], l2_offset[%d], "
               "l3_offset[%d] l4_offset[%d], l2_inner_offset[%d], l3_inner_offset[%d], "
               "l4_inner_offset[%d], payload_offset[%d]",
               t->packet_len, t->flags, t->local_vnic_tag, t->flow_hash, t->l2_offset,
               t->l3_offset, t->l4_offset, t->l2_inner_offset, t->l3_inner_offset,
               t->l4_inner_offset, t->payload_offset);
    return s;
}

u8 *
format_pds_p4_tx_cpu_hdr (u8 * s, va_list * args)
{
    p4_tx_cpu_hdr_t *t = va_arg (*args, p4_tx_cpu_hdr_t *);

    s = format (s, "flags_octet[0x%x]", t->flags_octet);
    return s;
}

always_inline void
pds_get_nacl_data_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                      u16 *nacl_data0, u16 *nacl_data1)
{
    *nacl_data0 = ~0;
    *nacl_data1 = ~0;
    return;
}

always_inline void
pds_get_nacl_data_x1 (vlib_buffer_t *p, u16 *nacl_data)
{
    *nacl_data = ~0;
}

#endif     // __VPP_IMPL_ARTEMIS_P4_CPU_HDR_UTILS_H__
