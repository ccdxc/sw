#include "../../p4/include/apulu_sacl_defines.h"

#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_setup_rfc_k.h"

struct phv_            p;
struct setup_rfc_k_    k;

%%

setup_rfc:
    // Is the current combination done?
    seq              c1, k.txdma_control_recirc_count[0:0], r0
    // No. Stop.
    nop.c1.e
    nop

    // Done for combination. Initialize for the next combination.
    seq              c1, k.txdma_control_stag_count, 0
    add.c1           r1, k.rx_to_tx_hdr_stag1_classid, r0
    seq              c1, k.txdma_control_stag_count, 1
    add.c1           r1, k.rx_to_tx_hdr_stag2_classid, r0
    seq              c1, k.txdma_control_stag_count, 2
    add.c1           r1, k.rx_to_tx_hdr_stag3_classid, r0
    seq              c1, k.txdma_control_stag_count, 3
    add.c1           r1, k.rx_to_tx_hdr_stag4_classid, r0
    seq              c1, k.txdma_control_stag_count, 4
    add.c1           r1, r0, 0x3FF

    // Is the next STAG is valid.
    sne              c1, r1, 0x3FF
    // Yes. Increment STAG count, and copy the new STAG to PHV and Stop.
    add.c1           r3, k.txdma_control_stag_count, 1
    phvwr.c1.e       p.txdma_control_stag_count, r3
    phvwr.c1         p.txdma_control_stag_classid, r1

    // Else (STAG is invalid): Reinitialize STAG and Find the next DTAG
    phvwr            p.txdma_control_stag_classid, k.rx_to_tx_hdr_stag0_classid
    phvwr            p.txdma_control_stag_count, r0
    seq              c1, k.txdma_control_dtag_count, 0
    add.c1           r2, k.rx_to_tx_hdr_dtag1_classid, r0
    seq              c1, k.txdma_control_dtag_count, 1
    add.c1           r2, k.rx_to_tx_hdr_dtag2_classid, r0
    seq              c1, k.txdma_control_dtag_count, 2
    add.c1           r2, k.rx_to_tx_hdr_dtag3_classid, r0
    seq              c1, k.txdma_control_dtag_count, 3
    add.c1           r2, k.rx_to_tx_hdr_dtag4_classid, r0
    seq              c1, k.txdma_control_dtag_count, 4
    add.c1           r2, r0, 0x3FF

    // Is the next DTAG is valid.
    sne              c1, r2, 0x3FF
    // Yes. Increment DTAG count, and copy the new DTAG to PHV and Stop.
    add.c1           r3, k.txdma_control_dtag_count, 1
    phvwr.c1.e       p.txdma_control_dtag_count, r3
    phvwr.c1         p.txdma_control_dtag_classid, r2

    // Else (DTAG is invalid): Reinitialize DTAG
    phvwr            p.txdma_control_dtag_classid, k.rx_to_tx_hdr_dtag0_classid
    phvwr            p.txdma_control_dtag_count, r0
    // Done for policy root. Inrement root count and initialize next policy root
    add              r3, k.txdma_control_root_count, 1
    phvwr            p.txdma_control_root_count, r3
    seq              c1, k.txdma_control_root_count, 0
    bcf              [c1], load1
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr1
    seq              c1, k.txdma_control_root_count, 1
    bcf              [c1], load2
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr2
    seq              c1, k.txdma_control_root_count, 2
    bcf              [c1], load3
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr3
    seq              c1, k.txdma_control_root_count, 3
    bcf              [c1], load4
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr4
    seq              c1, k.txdma_control_root_count, 4
    bcf              [c1], load5
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr5
    seq              c1, k.txdma_control_root_count, 5
    bcf              [c1], clearall
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, 0
    nop.e
    nop

load1:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid1
    add              r3, k.rx_to_tx_hdr_dip_classid1_s6_e9, \
                         k.rx_to_tx_hdr_dip_classid1_s0_e5, 4
    phvwr            p.rx_to_tx_hdr_dip_classid0, r3
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid1
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid1

load2:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid2
    add              r3, k.rx_to_tx_hdr_dip_classid2_s6_e9, \
                         k.rx_to_tx_hdr_dip_classid2_s0_e5, 4
    phvwr            p.rx_to_tx_hdr_dip_classid0, r3
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid2
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid2

load3:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid3
    add              r3, k.rx_to_tx_hdr_dip_classid3_s6_e7, \
                         k.rx_to_tx_hdr_dip_classid3_s0_e5, 2
    add              r3, k.rx_to_tx_hdr_dip_classid3_s8_e9, r3, 2
    phvwr            p.rx_to_tx_hdr_dip_classid0, r3
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid3
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid3

load4:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid4
    add              r3, k.rx_to_tx_hdr_dip_classid4_s6_e9, \
                         k.rx_to_tx_hdr_dip_classid4_s0_e5, 4
    phvwr            p.rx_to_tx_hdr_dip_classid0, r3
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid4
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid4

load5:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid5
    add              r3, k.rx_to_tx_hdr_dip_classid5_s6_e9, \
                         k.rx_to_tx_hdr_dip_classid5_s0_e5, 4
    phvwr            p.rx_to_tx_hdr_dip_classid0, r3
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid5
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid5

clearall:
    phvwr            p.rx_to_tx_hdr_sip_classid0, r0
    phvwr            p.rx_to_tx_hdr_dip_classid0, r0
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, r0
    phvwr            p.rx_to_tx_hdr_dport_classid0, r0

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
setup_rfc_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
