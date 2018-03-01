/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Implements the reading of BSQ to pick up barco completion
 *  Stage 0, Table 0
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

	.param		tls_mac_rx_bsq_mac_process
	
tls_mac_post_crypto_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)
    phvwr       p.to_s4_debug_dol, d.u.read_tls_stg0_d.debug_dol
    phvwr	p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow
	
    phvwr	p.tls_global_phv_fid, k.p4_txdma_intr_qid

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    add         r3, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    /* Increment CI for the BSQ-2PASS ring that we got doorbell on, here in stage 0 */
    tbladd      d.{u.read_tls_stg0_d.ci_2}.hx, 1

    /*
     * Check if pi == ci, in which case we need to ring the BSQ doorbell later.
     */
    seq         c4, d.{u.read_tls_stg0_d.ci_2}.hx, d.{u.read_tls_stg0_d.pi_2}.hx
    phvwri.c4   p.tls_global_phv_pending_rx_bsq, 1

table_read_rx_bsq_mac: 
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_mac_rx_bsq_mac_process,
	                         r3, TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
