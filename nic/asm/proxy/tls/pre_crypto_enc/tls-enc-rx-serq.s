/*
 *      Implements the receipt of tls encrypt request from SERQ 
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
        

struct tx_table_s2_t0_k k       ;
struct phv_ p   ;
struct tx_table_s2_t0_tls_rx_serq_d d   ;
        
%%
    .param      tls_enc_serq_consume_process
        
tls_enc_rx_serq_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /* Part of the configuration read and setup */
    seq         c1, k.tls_global_phv_flags_do_pre_ccm_enc, 1
    phvwr.!c1   p.crypto_iv_salt, d.salt
    phvwr.c1    p.ccm_header_with_aad_B_0_nonce_salt, d.salt
    CAPRI_OPERAND_DEBUG(d.salt)

    phvwr       p.barco_desc_key_desc_index, d.barco_key_desc_index


table_read_serq_consume:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tls_enc_serq_consume_process)
        
tls_rx_serq_process_enc_done:
        nop.e
        nop


