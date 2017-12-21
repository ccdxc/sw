/*
 * 	Implements the reading of SERQ to pick up the new descriptor to process
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct phv_ p	;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

	.param		tls_enc_pre_crypto_process
   	.param		tls_dec_pre_crypto_process
        .param          tls_dec_pre_crypto_aesgcm_newseg_process
	.param          tls_mac_pre_crypto_process
	
tls_pre_crypto_process:

    /*
     * The barco-command[31:24] is checked for AES-CBC-HMAC-SHA2 cipher for
     * MAC-then-encrypt case, in which case we need to do 2-pass with Barco
     * with 1st pass for HMAC request, and 2nd pass for AES-CBC encryption.
     * (endian-swapped) */
    CAPRI_OPERAND_DEBUG(d.u.read_tls_stg0_d.barco_command)
    add         r3, d.u.read_tls_stg0_d.barco_command[7:0], r0
    indexb      r2, r3, [0x73, 0x74], 0
    bnei.s      r2, -1, tls_pre_crypto_mac
    nop
	
    phvwr       p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow
    seq         c1, r0, d.u.read_tls_stg0_d.dec_flow
    bcf         [!c1], tls_pre_crypto_dec
    nop
    j           tls_enc_pre_crypto_process
    nop
    nop.e
    nop
tls_pre_crypto_dec:     
    smeqb       c1, d.u.read_tls_stg0_d.debug_dol, TLS_DDOL_DEC_REASM_PATH, TLS_DDOL_DEC_REASM_PATH 
    bcf         [c1], tls_pre_crypto_dec_reasm
    nop
    j           tls_dec_pre_crypto_process
    nop
    nop.e
    nop
tls_pre_crypto_dec_reasm:
    j           tls_dec_pre_crypto_aesgcm_newseg_process
    nop
    nop.e
    nop
tls_pre_crypto_mac:
    j           tls_mac_pre_crypto_process
    nop
    nop.e
    nop
