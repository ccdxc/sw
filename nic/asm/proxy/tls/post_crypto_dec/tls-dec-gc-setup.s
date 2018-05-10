/*
 * 	Implements the setting up of the GC related DMA commands
 *  Stage 7, Table 2
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s7_t2_k     k;
struct phv_                 p;
struct tx_table_s7_t2_d     d;

	
%%
    .align
    .param          RNMDR_TLS_GC_TABLE_BASE

tls_dec_gc_setup:
    CAPRI_CLEAR_TABLE_VALID(3)

    phvwr           p.gc_ring_entry_descr_addr , k.s2s_t2_idesc

    and             r2, d.{u.tls_gc_setup_d.token}.wx, CAPRI_HBM_GC_PER_PRODUCER_RING_MASK

    addui           r1, r0, hiword(RNMDR_TLS_GC_TABLE_BASE)
    addi            r1, r1, loword(RNMDR_TLS_GC_TABLE_BASE)
    add             r1, r1, r2, RNMDR_TABLE_ENTRY_SIZE_SHFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_gc_slot_dma_cmd, r1, gc_ring_entry_descr_addr, gc_ring_entry_descr_addr)
    mincr          r2, CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT, 1
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(dma_cmd_gc_dbell_dma_cmd, LIF_GC,
                    CAPRI_HBM_GC_RNMDR_QTYPE,
                    CAPRI_RNMDR_GC_TLS_RING_PRODUCER, 0,
                    r2, gc_db_data_pid, gc_db_data_index)
    CAPRI_DMA_CMD_FENCE(dma_cmd_gc_dbell_dma_cmd)
    nop.e
    nop


tls_enc_gc_setup_done:
	nop.e
	nop
