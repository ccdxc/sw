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

	.param		tls_dec_read_serq_entry_process
	
tls_dec_pre_crypto_process:
    phvwr       p.to_s5_debug_dol, d.u.read_tls_stg0_d.debug_dol
    CAPRI_OPERAND_DEBUG(d.u.read_tls_stg0_d.debug_dol)

	phvwr		p.tls_global_phv_fid, k.p4_txdma_intr_qid
    add         r3, r0, d.u.read_tls_stg0_d.ci_0
    sll         r3, r3, NIC_SERQ_ENTRY_SIZE_SHIFT
    # TBD  : move to updated ci
    add         r3, r0, r0
    add         r3, r3, d.u.read_tls_stg0_d.serq_base

	phvwri		p.tls_global_phv_pending_rx_serq, 1

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

table_read_DESC:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_serq_entry_process, r3, TABLE_SIZE_64_BITS)
	nop.e
    nop
