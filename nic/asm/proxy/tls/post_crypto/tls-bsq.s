/*
 * 	Implements the reading of BSQ to pick up barco completion
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

	.param		tls_rx_bsq_enc_process
   	.param		tls_rx_bsq_dec_process
	
tls_post_crypto_process:
    phvwr       p.to_s4_debug_dol, d.u.read_tls_stg0_d.debug_dol
	phvwr	    p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow
	
	phvwr		p.tls_global_phv_fid, k.p4_txdma_intr_qid

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    add         r3, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    phvwr       p.to_s4_debug_dol, d.u.read_tls_stg0_d.debug_dol
table_read_rx_bsq:
    seq         c1, r0, d.u.read_tls_stg0_d.dec_flow
    bcf         [!c1], table_read_rx_bsq_dec
    nop
table_read_rx_bsq_enc: 
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_rx_bsq_enc_process,
	                      r3, TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
    b tls_post_crypto_process_done
    nop
table_read_rx_bsq_dec: 
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_rx_bsq_dec_process,
	                       r3, TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
tls_post_crypto_process_done:   
	nop.e
    nop
