/*
 *      Read the explicit IV from the Barco DRBG (random number generator) crypto ram
 * Stage 4, Table 1
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
    random_num  : 64;
    pad         : 448;
};
        
struct tx_table_s4_t1_k k;
struct phv_ p;
struct d_struct d;
        
%%
        
tls_enc_read_random_iv:        

table_read_RANDOM_IV:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s6_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_1)
    CAPRI_CLEAR_TABLE1_VALID

    /*
     * We're supposed to use a random-number for the explicit-IV field for the TLS encrypttion, fill
     * the explicit IV for the barco-request with the pre-generated random number from the DRBG crypto-ram.
     * The rest of the barco-request data is being filled in by the Stage-4-table-0 program
     * (tls-enc-bld-barco-req.s), and the barco request in the phv will be DMAed later into BRQ slot.
     */
    seq       c1, k.to_s4_do_pre_ccm_enc, 1
    phvwr.!c1 p.crypto_iv_explicit_iv, d.random_num
    phvwr.c1  p.ccm_header_with_aad_B_0_nonce_explicit_iv, d.random_num
        
    CAPRI_OPERAND_DEBUG(d.random_num)

    /* Setup the explicit-IV field in the AAD too*/
    /* AAD length already setup in Stage 2, Table 3 */
    phvwr     p.s2_s5_t0_phv_aad_seq_num, d.random_num

    nop.e
    nop
