#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_vnic_by_vlan_tx_k  k;
struct local_vnic_by_vlan_tx_d  d;
struct phv_ p;

#define DATA                    d.local_vnic_info_tx_d

%%

local_vnic_info_tx:
    bcf             [!c1], local_vnic_info_tx_error
    phvwr.!c1       p.control_metadata_p4i_drop_reason[P4I_DROP_VNIC_INFO_TX_MISS], 1
    seq             c1, k.ethernet_1_srcAddr, r0
    bcf             [c1], local_vnic_info_tx_error
    phvwr.c1        p.control_metadata_p4i_drop_reason[P4I_DROP_SRC_MAC_ZERO], 1
    phvwr           p.vnic_metadata_local_vnic_tag, DATA.local_vnic_tag
    phvwr           p.vnic_metadata_vpc_id, DATA.vpc_id
    seq             c1, DATA.mode, APOLLO_MODE_DEFAULT
    cmov            r1, c1, DATA.local_vnic_tag, DATA.vpc_id
    phvwr           p.key_metadata_lkp_id, r1
    phvwr           p.control_metadata_mode, DATA.mode
    seq             c1, DATA.mirror_en, TRUE
    phvwr.c1        p.control_metadata_mirror_session, DATA.mirror_session

    /* c2 will be set if using epoch1, else will be reset */
    seq             c1, k.service_header_valid, TRUE
    seq.c1          c2, k.service_header_epoch, DATA.epoch1
    sle.!c1         c2, DATA.epoch2, DATA.epoch1
    bcf             [!c2], __use_epoch2
    seq             c3, DATA.epoch2_valid, TRUE
__use_epoch1:
    seq             c3, DATA.epoch1_valid, TRUE
    bcf             [!c3], local_vnic_info_tx_error
    phvwr.!c3       p.control_metadata_p4i_drop_reason[P4I_DROP_VNIC_INFO_TX_MISS], 1
    seq             c1, k.ethernet_1_srcAddr, DATA.overlay_mac1
    bcf             [!c1], local_vnic_info_tx_error
    phvwr.!c1       p.control_metadata_p4i_drop_reason[P4I_DROP_SRC_MAC_MISMATCH], 1

    phvwr           p.vnic_metadata_skip_src_dst_check, DATA.skip_src_dst_check1
    phvwr           p.policer_metadata_resource_group, DATA.resource_group1
    phvwr           p.p4_to_txdma_header_lpm_addr, DATA.lpm_v4addr1
    phvwr           p.control_metadata_lpm_v6addr, DATA.lpm_v6addr1
    phvwr           p.p4_to_rxdma_header_sacl_base_addr, DATA.sacl_v4addr1
    phvwr.e         p.control_metadata_sacl_v6addr, DATA.sacl_v6addr1
    phvwr           p.service_header_epoch, DATA.epoch1
__use_epoch2:
    bcf             [!c3], local_vnic_info_tx_error
    phvwr.!c3       p.control_metadata_p4i_drop_reason[P4I_DROP_VNIC_INFO_TX_MISS], 1
    seq             c1, k.ethernet_1_srcAddr, DATA.overlay_mac2
    bcf             [!c1], local_vnic_info_tx_error
    phvwr.!c1       p.control_metadata_p4i_drop_reason[P4I_DROP_SRC_MAC_MISMATCH], 1

    phvwr           p.vnic_metadata_skip_src_dst_check, DATA.skip_src_dst_check2
    phvwr           p.policer_metadata_resource_group, DATA.resource_group2
    phvwr           p.p4_to_txdma_header_lpm_addr, DATA.lpm_v4addr2
    phvwr           p.control_metadata_lpm_v6addr, DATA.lpm_v6addr2
    phvwr           p.p4_to_rxdma_header_sacl_base_addr, DATA.sacl_v4addr2
    phvwr.e         p.control_metadata_sacl_v6addr, DATA.sacl_v6addr2
    phvwr           p.service_header_epoch, DATA.epoch2

local_vnic_info_tx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_vnic_by_vlan_tx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
