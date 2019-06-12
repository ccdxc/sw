#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct vnic_info_txdma_k  k;
struct vnic_info_txdma_d  d;
struct phv_             p;

%%

vnic_info_txdma:
    // Copy the LPM roots to PHV based on AF
    seq          c1, k.rx_to_tx_hdr_iptype, IPTYPE_IPV4
    phvwr.c1     p.txdma_control_lpm1_base_addr, d.vnic_info_txdma_d.lpm_base1
    phvwr.!c1    p.txdma_control_lpm1_base_addr, d.vnic_info_txdma_d.lpm_base2

    //meter_idx is formed into r2
    or          r2, k.rx_to_tx_hdr_meter_result_sbit8_ebit9, \
                    k.rx_to_tx_hdr_meter_result_sbit0_ebit7, 2

    // Fill meter_idx into session info
    phvwr        p.session_info_hint_meter_idx, r2
    // Tx: always rewrite dmac
    phvwr        p.session_info_hint_tx_rewrite_flags_dmac, 1
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_info_txdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
