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
	
tls_pre_crypto_process:
	phvwr	    p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow 
    seq         c1, r0, d.u.read_tls_stg0_d.dec_flow
    bcf         [!c1], tls_pre_crypto_dec
    nop
    j           tls_enc_pre_crypto_process
    nop
    nop.e
    nop
tls_pre_crypto_dec:     
    j           tls_dec_pre_crypto_process
    nop
    nop.e
    nop
