/*
 * Implements the reading of the input packet descriptor.
 * This is needed to derive the AAD and the output length from the 
 * length of the input plain-text
 * Stage 2, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"        
#include "tls-sched.h"


struct phv_             p;
struct tx_table_s2_t3_k k;
struct tx_table_s2_t3_d d;

%%


tls_enc_pkt_descriptor_process:
    CAPRI_CLEAR_TABLE3_VALID

    /* Setup idesc */
    phvwr       p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    add         r2, r0, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    subi        r2, r2, NTLS_AAD_SIZE
    phvwr       p.idesc_O0, r2.wx
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    addi        r1, r1, NTLS_AAD_SIZE
    phvwr       p.idesc_L0, r1.wx


    /* Setup PHV2MEM for AAD */
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    add         r1, r1, r2
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r1,
                                s2_s5_t0_phv_aad_seq_num, s2_s5_t0_phv_aad_length)

    /* Setup odesc length */
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    addi        r1, r1, (NTLS_AAD_SIZE)
    phvwr       p.odesc_L0, r1.wx

    phvwr       p.s2_s5_t0_phv_aad_length, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

    nop.e
    nop

