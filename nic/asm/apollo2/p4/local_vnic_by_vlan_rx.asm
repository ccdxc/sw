#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_vnic_by_slot_rx_k k;
struct local_vnic_by_slot_rx_d d;
struct phv_ p;

%%

local_vnic_info_rx:
    phvwr           p.vnic_metadata_src_slot_id, d.local_vnic_info_rx_d.src_slot_id
    sne             c1, k.ipv4_1_dstAddr, r5
    phvwr.c1        p.capri_intrinsic_drop, 1
    or              r5, d.local_vnic_info_rx_d.resource_group_2_sbit1_ebit9, \
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

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_error:
    nop.e
    nop
