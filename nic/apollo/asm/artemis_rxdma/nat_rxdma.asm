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

    // Pass ca6_xlate_idx to txdma for local_46_mapping
    // reqduired for rflow derivation
    phvwr       p.rx_to_tx_hdr_ca6_xlate_idx, k.p4_to_rxdma2_ca6_xlate_idx

    seq         c1, d.find_local_ip_d.nat_ip[63:0], r0
    seq         c2, d.find_local_ip_d.nat_ip[127:64], r0
    bcf         [c1&c2], nat_rxdma_skip_nat
    add         r2, k.p4_to_rxdma_direction, r0 // BD Slot
    bnei        r2, TX_FROM_HOST, nat_rx_to_host
    add         r3, k.p4_to_rxdma2_service_xlate_port, r0 // BD Slot

    // In Tx dir, Policy evaluation should happen based on pre-nat src addr,
    // sport // so dont override the flow keys, but NAT sip/sport is reqd for rflow
    // keys derivation.  Carry this nat ip and xlate_port in rx_to_tx header.
    phvwr       p.rx_to_tx_hdr_nat_ip[127:64], d.find_local_ip_d.nat_ip[127:64]
    beq         r3, r0, skip_xlate_port
    phvwr       p.rx_to_tx_hdr_nat_ip[63:0], d.find_local_ip_d.nat_ip[63:0] // BD Slot
    phvwr.e     p.rx_to_tx_hdr_xlate_port, k.p4_to_rxdma2_service_xlate_port
    nop

nat_rx_to_host:
    // Overwrite FlowDst with this Local IP address derived from NAT table
    // Update FlowDst with this Xlate_port received from P4
    beq         r3, r0, skip_xlate_port
    phvwr       p.p4_to_rxdma_flow_dst, d.find_local_ip_d.nat_ip // BD Slot
    phvwr.e     p.p4_to_rxdma_flow_dport, k.p4_to_rxdma2_service_xlate_port
    nop

nat_rxdma_skip_nat:
skip_xlate_port:
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
