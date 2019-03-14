#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_vnic_by_slot_rx_k k;
struct local_vnic_by_slot_rx_d d;
struct phv_ p;

%%

local_vnic_info_rx:
    sne         c2, k.ipv4_1_dstAddr, r5
    bcf         [c2], local_vnic_info_rx_dipo_miss
    phvwr.c2    p.control_metadata_p4i_drop_reason[P4I_DROP_TEP_RX_DST_IP_MISMATCH], 1
    bcf         [!c1], local_vnic_info_rx_miss
    phvwr.!c1   p.control_metadata_p4i_drop_reason[P4I_DROP_DST_SLOT_ID_MISS], 1
    phvwr       p.vnic_metadata_src_slot_id, k.{mpls_src_label_sbit0_ebit15, \
                                                mpls_src_label_sbit16_ebit19}
    add         r1, d.local_vnic_info_rx_d.sacl_v4addr_1_sbit23_ebit33, \
                    d.local_vnic_info_rx_d.sacl_v4addr_1_sbit0_ebit22, 11
    LOCAL_VNIC_INFO_COMMON_END(d.local_vnic_info_rx_d.local_vnic_tag,
                               d.local_vnic_info_rx_d.vcn_id,
                               d.local_vnic_info_rx_d.skip_src_dst_check,
                               d.local_vnic_info_rx_d.resource_group_1,
                               0,
                               0,
                               r1,
                               d.local_vnic_info_rx_d.sacl_v6addr_1,
                               d.local_vnic_info_rx_d.epoch1,
                               d.local_vnic_info_rx_d.resource_group_2,
                               0,
                               0,
                               d.local_vnic_info_rx_d.sacl_v4addr_2,
                               d.local_vnic_info_rx_d.sacl_v6addr_2,
                               d.local_vnic_info_rx_d.epoch2)

local_vnic_info_rx_dipo_miss:
local_vnic_info_rx_miss:
    phvwr.e     p.capri_intrinsic_drop, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_vnic_by_slot_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
