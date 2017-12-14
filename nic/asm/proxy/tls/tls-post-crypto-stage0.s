/*
 * 	Implements the processing of the Barco response
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

	.param		tls_enc_post_crypto_process
   	.param		tls_dec_post_crypto_process
        .param          tls_dec_post_crypto_aesgcm_newseg_process
	.param          tls_mac_post_crypto_process
	
tls_post_crypto_process:
	phvwr	    p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow 
    seq         c1, r0, d.u.read_tls_stg0_d.dec_flow
    bcf         [!c1], tls_post_crypto_dec
    nop

    /*
     * NOTE: If this is AES-CBC case, in order to distinguish the 2 post-barco-crypto stages of
     * AES-CBC-HMAC-SHA2 2 pass pipeline (one for HMAC-SHA2 mac generation and one for
     * AES-CBC encrypt), we currently keep state in the TLSCB barco-command field (we do +1 in
     * pre-mac stage and -1 here in post-mac stage). This assumes only one outstanding request
     * to barco per TLSCB, which needs to be removed -- We'll use a different BSQ ring-id eventually
     * for barco to ring response doorbell on, to distinguish this case.
     */
    add         r3, d.u.read_tls_stg0_d.barco_command[7:0], r0
    indexb      r2, r3, [0x73, 0x74], 0
    seq.s       c2, r2, -1
    seq.!c2     c3, d.u.read_tls_stg0_d.barco_command[31:24], 1
    bcf         [c3], tls_post_crypto_mac
    nop
    j           tls_enc_post_crypto_process
    nop
    nop.e
    nop
tls_post_crypto_dec:     
    smeqb       c1, d.u.read_tls_stg0_d.debug_dol, TLS_DDOL_DEC_REASM_PATH, TLS_DDOL_DEC_REASM_PATH 
    bcf         [c1], tls_post_crypto_dec_reasm
    nop
    j           tls_dec_post_crypto_process
    nop
tls_post_crypto_dec_reasm:
    j           tls_dec_post_crypto_aesgcm_newseg_process
    nop
    nop.e
    nop
tls_post_crypto_mac:
    j          tls_mac_post_crypto_process
    nop
    nop.e
    nop
