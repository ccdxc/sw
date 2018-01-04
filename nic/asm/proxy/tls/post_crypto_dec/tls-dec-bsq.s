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

   	.param		tls_dec_rx_bsq_process
	
tls_dec_post_crypto_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)
    phvwr       p.to_s5_debug_dol, d.u.read_tls_stg0_d.debug_dol
    phvwr	p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow

    phvwr	p.tls_global_phv_fid, k.p4_txdma_intr_qid

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    add         r3, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    /* Increment CI in stage 0 */
    tbladd      d.{u.read_tls_stg0_d.ci_1}.hx, 1

    phvwr       p.to_s5_debug_dol, d.u.read_tls_stg0_d.debug_dol
    sne         c1, d.u.read_tls_stg0_d.l7_proxy_type, L7_PROXY_TYPE_NONE
    phvwri.c1   p.tls_global_phv_l7_proxy_en, 1
    seq         c2, d.u.read_tls_stg0_d.l7_proxy_type, L7_PROXY_TYPE_SPAN
    phvwri.c2   p.tls_global_phv_l7_proxy_type_span, 1

    /*
     * Check if this is AES-CCM decrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    seq         c1, d.u.read_tls_stg0_d.barco_command[7:0], 0x05
    phvwri.c1   p.to_s4_do_post_ccm_dec, 1

table_read_rx_bsq_dec: 
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_bsq_process,
	                       r3, TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
    nop
