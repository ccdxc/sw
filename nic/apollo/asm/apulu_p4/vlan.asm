#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vlan_k.h"

struct vlan_k_          k;
struct vlan_d           d;
struct phv_             p;

%%

vlan_info:
    sne             c1, d.vlan_info_d.vnic_id, r0
    phvwr.c1        p.vnic_metadata_vnic_id, d.vlan_info_d.vnic_id
    sne             c1, d.vlan_info_d.bd_id, r0
    phvwr.c1        p.vnic_metadata_bd_id, d.vlan_info_d.bd_id
    cmov            r5, c1, d.vlan_info_d.rmac, k.vnic_metadata_vrmac
    cmov            r6, c1, d.vlan_info_d.bd_id, k.vnic_metadata_bd_id
    seq.c1          c1, k.arm_to_p4i_flow_lkp_id_override, FALSE
    phvwr.c1        p.key_metadata_flow_lkp_id, d.vlan_info_d.bd_id
    sne             c1, d.vlan_info_d.vpc_id, r0
    phvwr.c1        p.vnic_metadata_vpc_id, d.vlan_info_d.vpc_id
    cmov            r7, c1, d.vlan_info_d.vpc_id, k.vnic_metadata_vpc_id

vlan_local_mapping_key:
    bbeq            k.control_metadata_rx_packet, TRUE, vlan_local_mapping_key_rx
    seq             c7, k.ipv4_1_valid, TRUE
vlan_local_mapping_key_tx:
    bcf             [!c7], vlan_local_mapping_key_tx_non_ipv4
    phvwr.c7        p.key_metadata_local_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ipv4_1_srcAddr
    b               vlan_mapping_key
    phvwr           p.key_metadata_local_mapping_lkp_id, r7

vlan_local_mapping_key_tx_non_ipv4:
    bbeq            k.arp_valid, TRUE, vlan_local_mapping_key_tx_arp
    phvwr           p.key_metadata_local_mapping_lkp_id, r6
    phvwr           p.key_metadata_local_mapping_lkp_type, KEY_TYPE_MAC
    b               vlan_mapping_key
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ethernet_1_srcAddr

vlan_local_mapping_key_tx_arp:
    phvwr           p.key_metadata_local_mapping_lkp_type, KEY_TYPE_IPV4
    b               vlan_mapping_key
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.arp_senderIpAddr

vlan_local_mapping_key_rx:
    bcf             [!c7], vlan_local_mapping_key_rx_non_ipv4
    phvwr.c7        p.key_metadata_local_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ipv4_1_dstAddr
    b               vlan_mapping_key
    phvwr           p.key_metadata_local_mapping_lkp_id, r7

vlan_local_mapping_key_rx_non_ipv4:
    phvwr           p.key_metadata_local_mapping_lkp_type, KEY_TYPE_MAC
    phvwr           p.key_metadata_local_mapping_lkp_addr, k.ethernet_1_dstAddr
    phvwr           p.key_metadata_local_mapping_lkp_id, r6

vlan_mapping_key:
    seq             c1, k.control_metadata_l2_enabled, FALSE
    seq.c7          c7, r5, k.ethernet_1_dstAddr
    orcf            c1, [c7]
    bcf             [!c1], vlan_mapping_key_non_ipv4
    phvwr.c1        p.p4i_i2e_mapping_lkp_type, KEY_TYPE_IPV4
    phvwr.e         p.p4i_i2e_mapping_lkp_addr, k.ipv4_1_dstAddr
    phvwr.f         p.p4i_i2e_mapping_lkp_id, r7

vlan_mapping_key_non_ipv4:
    phvwr           p.p4i_i2e_mapping_lkp_type, KEY_TYPE_MAC
    phvwr.e         p.p4i_i2e_mapping_lkp_addr, k.ethernet_1_dstAddr
    phvwr.f         p.p4i_i2e_mapping_lkp_id, r6

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vlan_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
