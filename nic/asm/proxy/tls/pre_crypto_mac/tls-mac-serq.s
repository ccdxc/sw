/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 *  Implements the reading of SERQ to pick up the new descriptor to process
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

	.param		tls_mac_read_serq_entry_process
	
tls_mac_pre_crypto_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)
    phvwr       p.to_s5_debug_dol, d.u.read_tls_stg0_d.debug_dol
    phvwr       p.to_s4_debug_dol, d.u.read_tls_stg0_d.debug_dol
    phvwr       p.to_s3_debug_dol, d.u.read_tls_stg0_d.debug_dol
    CAPRI_OPERAND_DEBUG(d.u.read_tls_stg0_d.debug_dol)
	
    phvwr	p.tls_global_phv_fid, k.p4_txdma_intr_qid
    add         r3, r0, d.{u.read_tls_stg0_d.ci_0}.hx
    sll         r3, r3, NIC_SERQ_ENTRY_SIZE_SHIFT

    /* Increment CI in stage 0 */
    tblmincri   d.{u.read_tls_stg0_d.ci_0}.hx, ASIC_SERQ_RING_SLOTS_SHIFT, 1
	
    seq         c4, d.{u.read_tls_stg0_d.ci_0}.hx, d.{u.read_tls_stg0_d.pi_0}.hx
    phvwri.c4   p.tls_global_phv_pending_rx_serq, 1
	
    #add        r3, r0, r0
    add         r3, r3, d.u.read_tls_stg0_d.serq_base

    phvwr       p.tls_global_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

table_read_DESC:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_mac_read_serq_entry_process, r3, TABLE_SIZE_64_BITS)
    nop.e
    nop
