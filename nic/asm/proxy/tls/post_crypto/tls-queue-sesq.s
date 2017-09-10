/*
 * 	Implements the queuing of barco post completion output descriptor to TCP SESQ
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s4_t0_k k                  ;
struct phv_ p	;
//struct tx_table_s4_t0_tls_queue_sesq_d d;
struct tx_table_s0_t0_d d;

	
%%
    .param      tls_post_crypto_stats_process

tls_queue_sesq_process:
    CAPRI_CLEAR_TABLE0_VALID
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4


dma_cmd_sesq_slot:
	add		    r5, r0, d.u.read_tls_stg0_d.sw_sesq_pi
	sll		    r5, r5, NIC_SESQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SESQ slot */
	add		    r1, r5, d.u.read_tls_stg0_d.sesq_base

	phvwr		p.dma_cmd0_dma_cmd_addr, r1
    add         r3, r0, k.to_s4_odesc
    phvwr       p.ring_entry_descr_addr, r3.dx
    CAPRI_OPERAND_DEBUG(k.to_s4_odesc)

    phvwri		p.dma_cmd0_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
	phvwri		p.dma_cmd0_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)
	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_eop, 0

    addi        r1, r0, TLS_DDOL_SESQ_STOP
    seq         c1, r1, k.to_s4_debug_dol
    bcf         [c1], tls_sesq_produce_skip
    nop

tls_sesq_produce:

	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TCP)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, k.tls_global_phv_fid, TCP_SCHED_RING_SESQ, d.u.read_tls_stg0_d.sw_sesq_pi)

	phvwr		p.dma_cmd1_dma_cmd_addr, r4
	phvwr		p.db_data_data, r3

	phvwri		p.dma_cmd1_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(db_data_data)
	phvwri		p.dma_cmd1_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(db_data_data)
	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    phvwri      p.dma_cmd1_dma_cmd_eop, 1
    phvwri      p.dma_cmd1_dma_cmd_wr_fence, 1
    b           tls_queue_sesq_process_done
    nop
tls_sesq_produce_skip:
    phvwri      p.dma_cmd0_dma_cmd_eop, 1
    phvwri      p.dma_cmd0_dma_cmd_wr_fence, 1
        
tls_queue_sesq_process_done:
	CAPRI_NEXT_TABLE0_READ(k.tls_global_phv_fid, TABLE_LOCK_DIS, tls_post_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_POST_CRYPTO_STATS_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
	nop
