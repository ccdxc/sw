#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_vnic_by_slot_rx_k  k;
struct local_vnic_by_slot_rx_d  d;
struct phv_ p;

#define DATA                    d.local_vnic_info_rx_d

%%

local_vnic_info_rx:
    sne             c2, k.ipv4_1_dstAddr, r5
    bcf             [c2], local_vnic_info_rx_error
    phvwr.c2        p.control_metadata_p4i_drop_reason[P4I_DROP_TEP_RX_DST_IP_MISMATCH], 1
    bcf             [!c1], local_vnic_info_rx_error
    phvwr.!c1       p.control_metadata_p4i_drop_reason[P4I_DROP_VNIC_INFO_RX_MISS], 1
    phvwr           p.vnic_metadata_src_slot_id, k.{mpls_src_label_sbit0_ebit15, \
                                                    mpls_src_label_sbit16_ebit19}
    phvwr           p.vnic_metadata_local_vnic_tag, DATA.local_vnic_tag
    phvwr           p.vnic_metadata_vpc_id, DATA.vpc_id
    phvwr           p.control_metadata_tunneled_packet, TRUE
    /* c2 will be set if using epoch1, else will be reset */
    seq             c1, k.service_header_valid, TRUE
    seq.c1          c2, k.service_header_epoch, DATA.epoch1
    sle.!c1         c2, DATA.epoch2, DATA.epoch1
    bcf             [!c2], __use_epoch2
    seq             c3, DATA.epoch2_valid, TRUE
__use_epoch1:
    seq             c3, DATA.epoch1_valid, TRUE
    bcf             [!c3], local_vnic_info_rx_error
    phvwr.!c3       p.control_metadata_p4i_drop_reason[P4I_DROP_VNIC_INFO_RX_MISS], 1

    phvwr           p.vnic_metadata_skip_src_dst_check, DATA.skip_src_dst_check1
    phvwr           p.policer_metadata_resource_group, DATA.resource_group1
    add             r1, DATA.lpm_v4addr1_sbit10_ebit33, \
                        DATA.lpm_v4addr1_sbit0_ebit9, 24
    phvwr           p.p4_to_txdma_header_lpm_addr, r1
    phvwr           p.control_metadata_lpm_v6addr, DATA.lpm_v6addr1
    phvwr           p.p4_to_rxdma_header_sacl_base_addr, DATA.sacl_v4addr1
    phvwr.e         p.control_metadata_sacl_v6addr, DATA.sacl_v6addr1
    phvwr           p.service_header_epoch, DATA.epoch1
__use_epoch2:
    bcf             [!c3], local_vnic_info_rx_error
    phvwr.!c3       p.control_metadata_p4i_drop_reason[P4I_DROP_VNIC_INFO_RX_MISS], 1
    phvwr           p.vnic_metadata_skip_src_dst_check, DATA.skip_src_dst_check2
    phvwr           p.policer_metadata_resource_group, DATA.resource_group2
    phvwr           p.p4_to_txdma_header_lpm_addr, DATA.lpm_v4addr2
    phvwr           p.control_metadata_lpm_v6addr, DATA.lpm_v6addr2
    phvwr           p.p4_to_rxdma_header_sacl_base_addr, DATA.sacl_v4addr2
    phvwr.e         p.control_metadata_sacl_v6addr, DATA.sacl_v6addr2
    phvwr           p.service_header_epoch, DATA.epoch2

local_vnic_info_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_vnic_by_slot_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
