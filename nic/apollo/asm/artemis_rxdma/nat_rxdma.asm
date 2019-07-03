#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct nat_rxdma_k  k;
struct nat_rxdma_d  d;
struct phv_       p;

%%

nat_rxdma:
    // Disable this lookup for further passes
    phvwr       p.p4_to_rxdma_vnic_info_en, FALSE

    add         r1, d.find_local_ip_d.nat_ip, r0
    beq         r1, r0, not_rxdma_skip_nat
    seq         c1, k.p4_to_rxdma_direction, TX_FROM_HOST

    // In Tx dir, Policy evaluation should happen based on pre-nat src addr,
    // sport // so dont override the flow keys, but NAT sip/sport is reqd for rflow
    // keys derivation.  Carry this nat ip and xlate_port in rx_to_tx header.
    phvwr.c1    p.rx_to_tx_hdr_nat_ip[127:64], d.find_local_ip_d.nat_ip[127:64]
    phvwr.c1    p.rx_to_tx_hdr_nat_ip[63:0], d.find_local_ip_d.nat_ip[63:0]
    phvwr.c1    p.rx_to_tx_hdr_xlate_port, k.p4_to_rxdma2_xlate_port

    // Overwrite FlowDst with this Local IP address derived from NAT table
    phvwr.!c1   p.p4_to_rxdma_flow_dst, d.find_local_ip_d.nat_ip
    // Update FlowDst with this Xlate_port received from P4
    phvwr.!c1   p.p4_to_rxdma_flow_dport, k.p4_to_rxdma2_xlate_port

not_rxdma_skip_nat:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nat_rxdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
