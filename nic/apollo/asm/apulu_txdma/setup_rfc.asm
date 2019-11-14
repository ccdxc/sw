#include "../../p4/include/apulu_sacl_defines.h"

#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_setup_rfc_k.h"

struct phv_            p;
struct setup_rfc_k_    k;

%%

setup_rfc:
    // Load the correct sacl_base_address into r1 based on recirc count
    seq              c1, k.txdma_control_recirc_count, 1
    bcf              [c1], load1
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr1
    seq              c1, k.txdma_control_recirc_count, 3
    bcf              [c1], load2
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr2
    seq              c1, k.txdma_control_recirc_count, 5
    bcf              [c1], load3
    phvwrpair.c1     p.rx_to_tx_hdr_sacl_base_addr0[39:8], \
                        k.rx_to_tx_hdr_sacl_base_addr3_s0_e31, \
                        p.rx_to_tx_hdr_sacl_base_addr0[7:0], \
                        k.rx_to_tx_hdr_sacl_base_addr3_s32_e39
    seq              c1, k.txdma_control_recirc_count, 7
    bcf              [c1], load4
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr4
    seq              c1, k.txdma_control_recirc_count, 9
    bcf              [c1], load5
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, k.rx_to_tx_hdr_sacl_base_addr5
    seq              c1, k.txdma_control_recirc_count, 11
    bcf              [c1], clearall
    phvwr.c1         p.rx_to_tx_hdr_sacl_base_addr0, 0
    nop.e
    nop

load1:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid1
    phvwr            p.rx_to_tx_hdr_dip_classid0, k.rx_to_tx_hdr_dip_classid1
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid1
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid1

load2:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid2
    phvwr            p.rx_to_tx_hdr_dip_classid0, k.rx_to_tx_hdr_dip_classid2
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid2
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid2

load3:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid3
    phvwr            p.rx_to_tx_hdr_dip_classid0, k.rx_to_tx_hdr_dip_classid3
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid3
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid3

load4:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid4
    phvwr            p.rx_to_tx_hdr_dip_classid0, k.rx_to_tx_hdr_dip_classid4
    phvwr.e          p.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sport_classid4
    phvwr            p.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dport_classid4

load5:
    phvwr            p.rx_to_tx_hdr_sip_classid0, k.rx_to_tx_hdr_sip_classid5
    phvwr            p.rx_to_tx_hdr_dip_classid0, k.rx_to_tx_hdr_dip_classid5
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
