/*
 * 	Implements the queuing of barco post completion output descriptor to TCP SESQ
 *  Stage 6, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s6_t0_k     k;
struct phv_                 p;
struct tx_table_s6_t0_d     d;

	
%%
    .param      tls_enc_post_crypto_stats_process

tls_enc_queue_sesq_process:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

    smeqb       c5, k.to_s6_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO

    /* Barco bypass: Skip TLS header DMA command */
    phvwri.c5   p.dma_cmd0_dma_cmd_type, 0

dma_cmd_odesc:

    add         r1, r0, k.to_s6_odesc        
    //addi        r1, r1, PKT_DESC_AOL_OFFSET
    
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r1, odesc_A0, odesc_next_pkt)

    /* Bypass Barco: Skip odesc rewrite DMA command */
    phvwri.c5   p.dma_cmd1_dma_cmd_type, 0

dma_cmd_sesq_slot:
	add		    r5, r0, d.u.tls_queue_sesq_d.sw_sesq_pi
	sll		    r5, r5, NIC_SESQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SESQ slot */
	add		    r4, r5, d.u.tls_queue_sesq_d.sesq_base
    phvwr       p.ring_entry_descr_addr, k.to_s6_odesc
    CAPRI_OPERAND_DEBUG(k.to_s6_odesc)
    CAPRI_OPERAND_DEBUG(k.to_s6_other_fid)

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r4, ring_entry_descr_addr, ring_entry_descr_addr)    

    addi        r1, r0, TLS_DDOL_SESQ_STOP
    smeqb       c1, k.to_s6_debug_dol, TLS_DDOL_SESQ_STOP, TLS_DDOL_SESQ_STOP
    bcf         [c1], tls_sesq_produce_skip
    nop

tls_sesq_produce:

    smeqb       c1, k.to_s6_debug_dol, TLS_DDOL_BYPASS_PROXY, TLS_DDOL_BYPASS_PROXY
    add.c1      r7, k.tls_global_phv_fid, r0
    add.!c1     r7, k.to_s6_other_fid, r0

    CAPRI_DMA_CMD_RING_DOORBELL(dma_cmd3_dma_cmd, LIF_TCP, 0, r7, TCP_SCHED_RING_SESQ,
                                d.u.tls_queue_sesq_d.sw_sesq_pi,db_data_data)
    tbladd      d.u.tls_queue_sesq_d.sw_sesq_pi, 1

    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd3_dma_cmd)
    b           tls_queue_sesq_process_done
    nop
tls_sesq_produce_skip:
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd2_dma_cmd)
        
tls_queue_sesq_process_done:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_post_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr,
	                    TLS_TCB_POST_CRYPTO_STATS_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
	nop
