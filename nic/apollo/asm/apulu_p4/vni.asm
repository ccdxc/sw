#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vni_k.h"

struct vni_k_           k;
struct vni_d            d;
struct phv_             p;

%%

vni_info:
    bcf             [!c1], vni_miss
    sne             c1, d.vni_info_d.vnic_id, r0
    phvwr.c1        p.vnic_metadata_vnic_id, d.vni_info_d.vnic_id
    sne             c1, d.vni_info_d.bd_id, r0
    phvwr.c1        p.vnic_metadata_bd_id, d.vni_info_d.bd_id
    cmov            r6, c1, d.vni_info_d.bd_id, k.vnic_metadata_bd_id
    sne             c1, d.vni_info_d.vpc_id, r0
    phvwr.c1        p.vnic_metadata_vpc_id, d.vni_info_d.vpc_id
    cmov            r7, c1, d.vni_info_d.vpc_id, k.vnic_metadata_vpc_id
    phvwr           p.control_metadata_tunneled_packet, TRUE
    phvwr           p.p4i_to_arm_is_l3_vnid, d.vni_info_d.is_l3_vnid

    seq             c7, k.ipv4_2_valid, TRUE
    bcf             [!c7], vni_local_mapping_key_non_ipv4
vni_local_mapping_key_ipv4:
    phvwr.c7        p.key_metadata_local_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ipv4_2_dstAddr
    b               vni_mapping_key
    phvwr           p.key_metadata_local_mapping_lkp_id, r7

vni_local_mapping_key_non_ipv4:
    phvwr           p.key_metadata_local_mapping_lkp_type, KEY_TYPE_MAC
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ethernet_2_dstAddr
    b               vni_mapping_key
    phvwr           p.key_metadata_local_mapping_lkp_id, r6

vni_mapping_key:
    seq             c1, k.control_metadata_l2_enabled, FALSE
    seq.c7          c7, d.vni_info_d.rmac, k.ethernet_2_dstAddr
    orcf            c1, [c7]
    bcf             [!c1], vni_mapping_key_non_ipv4
    phvwr.c1        p.p4i_i2e_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr.e         p.p4i_i2e_mapping_lkp_addr, k.ipv4_2_dstAddr
    phvwr.f         p.p4i_i2e_mapping_lkp_id, r7

vni_mapping_key_non_ipv4:
    phvwr           p.p4i_i2e_mapping_lkp_type, KEY_TYPE_MAC
    phvwr.e         p.p4i_i2e_mapping_lkp_addr, k.ethernet_2_dstAddr
    phvwr.f         p.p4i_i2e_mapping_lkp_id, r6

vni_miss:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr.f         p.control_metadata_p4i_drop_reason[P4I_DROP_VNI_INVALID], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vni_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
