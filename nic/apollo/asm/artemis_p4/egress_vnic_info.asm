#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_egress_vnic_info_k.h"

struct egress_vnic_info_k_ k;
struct egress_vnic_info_d  d;
struct phv_ p;

%%

egress_vnic_info:
    phvwr           p.txdma_to_p4e_valid, FALSE
    phvwr           p.predicate_header_valid, FALSE
    phvwr           p.p4e_i2e_valid, FALSE
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    nop.!c1.e
    phvwr           p.rewrite_metadata_device_mac, r5
egress_vnic_rx:
    phvwr           p.ethernet_1_dstAddr, d.egress_vnic_info_d.ca_mac
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_SMAC_BITS], \
                        RX_REWRITE_SMAC_FROM_VRMAC
    phvwr.c1        p.ethernet_1_srcAddr, d.egress_vnic_info_d.vr_mac
    phvwr           p.capri_intrinsic_tm_oport, d.egress_vnic_info_d.port
    seq             c1, d.egress_vnic_info_d.local_vlan, r0
    bcf             [c1], egress_vnic_port_check
egress_vnic_insert_vlan:
    add             r1, k.capri_p4_intrinsic_packet_len, 4
    phvwr           p.ctag_1_valid, TRUE
    phvwr           p.ethernet_1_etherType, ETHERTYPE_VLAN
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    add             r1, k.ethernet_1_etherType, d.egress_vnic_info_d.local_vlan, 16
    phvwr           p.{ctag_1_pcp,ctag_1_dei,ctag_1_vid,ctag_1_etherType}, r1
egress_vnic_port_check:
    seq             c1, d.egress_vnic_info_d.port, TM_PORT_DMA
    nop.!c1.e
    phvwr.c1        p.capri_intrinsic_lif, d.egress_vnic_info_d.lif
    phvwr.e         p.capri_rxdma_intrinsic_qtype, d.egress_vnic_info_d.qtype
    phvwr.f         p.capri_rxdma_intrinsic_qid, d.egress_vnic_info_d.qid

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_vnic_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
