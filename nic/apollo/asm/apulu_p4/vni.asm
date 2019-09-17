#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vni_k.h"

struct vni_k_           k;
struct vni_d            d;
struct phv_             p;

%%

vni_info:
    sne             c1, d.vni_info_d.vnic_id, r0
    phvwr.c1        p.vnic_metadata_vnic_id, d.vni_info_d.vnic_id
    sne             c1, d.vni_info_d.bd_id, r0
    phvwr.c1        p.vnic_metadata_bd_id, d.vni_info_d.bd_id
    cmov            r6, c1, d.vni_info_d.bd_id, k.vnic_metadata_bd_id
    sne             c1, d.vni_info_d.vpc_id, r0
    phvwr.c1        p.vnic_metadata_vpc_id, d.vni_info_d.vpc_id
    cmov            r7, c1, d.vni_info_d.vpc_id, k.vnic_metadata_vpc_id
    phvwr           p.control_metadata_tunneled_packet, TRUE

vni_mapping_key:
    bbeq            k.ipv4_2_valid, TRUE, vni_mapping_key_non_ipv4
    nop
    phvwr           p.key_metadata_local_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ipv4_2_dstAddr
    phvwr           p.key_metadata_local_mapping_lkp_id, r7
    phvwr.e         p.p4i_i2e_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr.f         p.p4i_i2e_mapping_lkp_addr, k.ipv4_2_dstAddr

vni_mapping_key_non_ipv4:
    phvwr           p.key_metadata_local_mapping_lkp_type, KEY_TYPE_MAC
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ethernet_2_dstAddr
    phvwr           p.key_metadata_local_mapping_lkp_id, r6
    phvwr.e         p.p4i_i2e_mapping_lkp_type, KEY_TYPE_MAC
    phvwr.f         p.p4i_i2e_mapping_lkp_addr, k.ethernet_2_dstAddr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vni_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
