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
    phvwr.c2    p.control_metadata_p4i_drop_reason[DROP_TEP_RX_DST_IP_MISMATCH], TRUE
    bcf         [!c1], local_vnic_info_rx_miss
    phvwr.!c1   p.control_metadata_p4i_drop_reason[DROP_DST_SLOT_ID_MISS], TRUE
    phvwr       p.vnic_metadata_src_slot_id, k.{mpls_1_label_sbit0_ebit15, \
                                                mpls_1_label_sbit16_ebit19}
    or          r5, d.local_vnic_info_rx_d.resource_group_2_sbit1_ebit9, \
                    d.local_vnic_info_rx_d.resource_group_2_sbit0_ebit0, 9
    LOCAL_VNIC_INFO_COMMON_END(d.local_vnic_info_rx_d.local_vnic_tag,
                               d.local_vnic_info_rx_d.vcn_id,
                               d.local_vnic_info_rx_d.skip_src_dst_check,
                               d.local_vnic_info_rx_d.resource_group_1,
                               0,
                               d.local_vnic_info_rx_d.slacl_addr_1,
                               d.local_vnic_info_rx_d.epoch1,
                               r5,
                               0,
                               d.local_vnic_info_rx_d.slacl_addr_2,
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
    nop.e
    nop
