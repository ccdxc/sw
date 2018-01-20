/*
 * 	Implements the reading of BSQ to pick up barco completion
 *  Stage 0, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_tls_stg0_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

   	.param		tls_dec_post_crypto_aesgcm_newseg_get_tls_record_ref
	
tls_dec_post_crypto_aesgcm_newseg_process:
	
    phvwr	p.tls_global_phv_fid, k.p4_txdma_intr_qid

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}


   /* Increment CI in stage 0 and acknowledge the BSQ doorbell */
    tbladd      D(ci_1).hx, 1
	
    /* Acknowledge BSQ Doorbell */
    /* address will be in r4 */
    addi        r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add		r1, k.p4_txdma_intr_qid, r0

    /* data will be in r3
     *
     * We have incremented CI above, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */
    add        r2, D(ci_1).hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_BSQ, r2)
    memwr.dx   r4, r3

    add        r3, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

tls_dec_post_crypto_aesgcm_newseg_process_done: 
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                            tls_dec_post_crypto_aesgcm_newseg_get_tls_record_ref,
	                    r3, TLS_TCB_OPER_DATA_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
    nop
