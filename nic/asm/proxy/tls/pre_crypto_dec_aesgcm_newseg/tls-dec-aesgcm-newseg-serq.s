/*
 * 	Implements the reading of SERQ to pick up the new descriptor to process
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

struct phv_ p	;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

	.param      tls_dec_aesgcm_read_serq_entry_process
	
tls_dec_pre_crypto_aesgcm_newseg_process:
//    phvwr       p.to_s5_debug_dol, D(debug_dol)
//    phvwr       p.to_s4_debug_dol, D(debug_dol)
//    CAPRI_OPERAND_DEBUG(D(debug_dol))


    /* If another segment is actively being processed, do not continue */
    sne         c1, D(active_segment), r0
    bcf         [c1], serq_read_done
    nop

    /* Barrier: Mark segment being processed */
    tblwr       D(active_segment), 1

    phvwr       p.tls_global_phv_qstate_addr, \
                K(p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33)
    phvwr       p.to_s6_fid, k.p4_txdma_intr_qid 

    add         r2, r0, D(ci_0).hx
    sll         r2, r2, NIC_SERQ_ENTRY_SIZE_SHIFT
    add         r2, r2, D(serq_base)

    /* Increment CI in stage 0 and acknowledge the doorbell */
    tblmincri.f D(ci_0).hx, ASIC_SERQ_RING_SLOTS_SHIFT, 1
	
    /* Acknowledge the doorbell */
	//CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    addi        r4, r0, \
    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add		r1, k.p4_txdma_intr_qid, r0

    /* data will be in r3
     *
     * We have incremented CI above, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */
    add         r5, D(ci_0).hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_SERQ, r5)

    memwr.dx    r4, r3

    phvwr           p.barco_desc_command, D(barco_command)

table_read_DESC:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_aesgcm_read_serq_entry_process, r2, TABLE_SIZE_256_BITS)

serq_read_done:
	nop.e
    nop
