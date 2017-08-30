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

	.param		tls_read_serq_entry_process_start
	
tls_stage0:
	phvwr	    p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow
	
	phvwr		p.tls_global_phv_fid, k.p4_txdma_intr_qid
    add         r3, r0, d.u.read_tls_stg0_d.ci_0
    sll         r3, r3, NIC_SERQ_ENTRY_SIZE_SHIFT
    # TBD  : move to updated ci
    add         r3, r0, r0
    add         r3, r3, d.u.read_tls_stg0_d.serq_base

	phvwr		p.to_s2_idesc, r3
   	phvwr		p.to_s5_idesc, r3
	phvwri		p.tls_global_phv_pending_rx_serq, 1

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

table_read_DESC:
    CAPRI_NEXT_IDX0_READ(TABLE_LOCK_DIS, tls_read_serq_entry_process_start, r3, TABLE_SIZE_64_BITS)
	nop.e
    nop
