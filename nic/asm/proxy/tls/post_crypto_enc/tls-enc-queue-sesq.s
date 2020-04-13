/*
 * 	Implements the queuing of barco post completion output descriptor to TCP SESQ
 *  Stage 7, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s7_t0_k     k;
struct phv_                 p;
struct tx_table_s7_t0_d     d;

	
%%

tls_enc_queue_sesq_process:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_7, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, (CAPRI_PHV_START_OFFSET(dma_cmd_gc_slot_dma_cmd_type) / 16)

    smeqb       c5, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO

    /* Barco bypass: Skip TLS header DMA command */
    phvwri.c5   p.dma_cmd_tls_hdr_dma_cmd_type, 0

dma_cmd_odesc:

    add         r1, r0, k.to_s7_odesc        
    //addi        r1, r1, PKT_DESC_AOL_OFFSET
    
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_odesc_dma_cmd, r1, odesc_A0, odesc_next_pkt)

    /* Bypass Barco: Skip odesc rewrite DMA command */
    phvwri.c5   p.dma_cmd_odesc_dma_cmd_type, 0

dma_cmd_sesq_slot:
	add		    r5, r0, k.tls_global_phv_sesq_pi
	sll		    r5, r5, NIC_SESQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SESQ slot */
	add		    r4, r5, d.u.tls_queue_sesq_d.sesq_base
    phvwr       p.ring_entry_descr_addr, k.to_s7_odesc
    CAPRI_OPERAND_DEBUG(k.to_s7_odesc)
    CAPRI_OPERAND_DEBUG(k.to_s7_other_fid)

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_sesq_slot_dma_cmd, r4, ring_entry_pad, ring_entry_descr_addr)

    smeqb       c1, k.tls_global_phv_debug_dol, TLS_DDOL_SESQ_STOP, TLS_DDOL_SESQ_STOP
    bcf         [c1], tls_sesq_produce_skip
    nop

tls_sesq_produce:

    smeqb       c1, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_PROXY, TLS_DDOL_BYPASS_PROXY
    add.c1      r7, k.tls_global_phv_fid, r0
    add.!c1     r7, k.to_s7_other_fid, r0

    add         r1, r0, k.tls_global_phv_sesq_pi
    mincr       r1, ASIC_SESQ_RING_SLOTS_SHIFT, 1
    CAPRI_DMA_CMD_RING_DOORBELL_SET_PI(dma_cmd_sesq_dbell_dma_cmd, LIF_TCP, 0, r7, TCP_SCHED_RING_SESQ,
                                r1, db_data_data)

    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_sesq_dbell_dma_cmd)
    b           tls_queue_sesq_process_done
    nop
tls_sesq_produce_skip:
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_sesq_slot_dma_cmd)
        
tls_queue_sesq_process_done:
	nop.e
	nop
