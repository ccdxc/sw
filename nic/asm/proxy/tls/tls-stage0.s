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

	.param		tls_pre_crypto_process
   	.param		tls_post_crypto_process
   	.param		tls_mac_post_crypto_process
	
tls_stage0:
     CAPRI_OPERAND_DEBUG(r7)
	.brbegin
	    brpri		r7[2:0], [1,0,2]
	    nop
	        .brcase 0
	            j tls_pre_crypto_process
	            nop
	        .brcase 1
	            j tls_post_crypto_process
	            nop
	        .brcase 2
	            // BSQ_2PASS ring, for post-MAC stage o MAC-then-Encrypt 2 pass pipeline
	            j tls_mac_post_crypto_process
	            nop
	        .brcase 3
	            b tls_prog_abort
	            nop
	.brend

tls_prog_abort:
    phvwri          p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop
