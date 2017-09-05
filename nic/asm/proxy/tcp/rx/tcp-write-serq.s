/*
 *	Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_rx_write_serq_k k;
struct tcp_rx_write_serq_write_serq_d d;

%%
        .param          tcp_rx_stats_stage7_start
	.align	
tcp_rx_write_serq_stage6_start:
        CAPRI_CLEAR_TABLE0_VALID
	/* r4 is loaded at the beginning of the stage with current timestamp value */
	tblwr		d.curr_ts, r4
	/* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
	sne		c1, k.common_phv_write_serq, r0
	bcf		[!c1], flow_write_serq_process_done
	nop

dma_cmd_data:
	addi		r5, r0, TCP_PHV_DMA_COMMANDS_START
	add		r4, r5, r0
	phvwr		p.p4_rxdma_intr_dma_cmd_ptr, r4

	/* Set the DMA_WRITE CMD for data */
	add		r1, r0, k.to_s6_page
	addi		r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
	
	phvwr		p.dma_cmd0_dma_cmd_addr, r3
	phvwr		p.dma_cmd0_dma_cmd_size, k.to_s6_payload_len
	
	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
        phvwri          p.dma_cmd0_dma_cmd_eop, 0

	addi		r4, r4, CAPRI_DMA_COMMAND_SIZE
dma_cmd_descr:	
	/* Set the DMA_WRITE CMD for descr */
	add		r5, k.to_s6_descr, r0
	addi		r1, r5, NIC_DESC_ENTRY_0_OFFSET
	phvwr		p.dma_cmd1_dma_cmd_addr, r1

	phvwr		p.aol_A0, k.to_s6_page
	phvwr		p.aol_O0, r3
	phvwr		p.aol_L0, k.to_s6_payload_len
	phvwr		p.aol_A1, r0
	phvwr		p.aol_O1, r0
	phvwr		p.aol_L1, r0
	phvwr		p.aol_A2, r0
	phvwr		p.aol_O2, r0
	phvwr		p.aol_L2, r0
	phvwr		p.aol_next_addr, r0

        phvwri          p.dma_cmd1_dma_cmd_phv_start_addr, TCP_PHV_AOL_DESC_START
	phvwri		p.dma_cmd1_dma_cmd_phv_end_addr, TCP_PHV_AOL_DESC_END

	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
        phvwri          p.dma_cmd1_dma_cmd_eop, 0
	
	addi		r4, r4, CAPRI_DMA_COMMAND_SIZE

dma_cmd_serq_slot:
	add		r5, r0, k.to_s6_serq_pidx
	sll		r5, r5, NIC_SERQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SERQ slot */
	add		r1, r5, k.to_s6_serq_base

	phvwr		p.dma_cmd2_dma_cmd_addr, r1
	phvwr		p.ring_entry_descr_addr, k.to_s6_descr
	phvwri		p.dma_cmd2_dma_cmd_phv_start_addr, TCP_PHV_RING_ENTRY_DESC_ADDR_START
	phvwri		p.dma_cmd2_dma_cmd_phv_end_addr, TCP_PHV_RING_ENTRY_DESC_ADDR_END
	phvwri		p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
        phvwri          p.dma_cmd2_dma_cmd_eop, 0
	addi		r4, r4, CAPRI_DMA_COMMAND_SIZE
dma_cmd_write_rx2tx_shared:
	/* Set the DMA_WRITE CMD for copying rx2tx shared data from phv to mem */
#if 0
        // TODO : dma not happening correctly to non-64 byte aligned memory
	add		r5, r0, k.common_phv_qstate_addr
	add		r6, r0, k.common_phv_fid
	sll		r6, r6, TCP_TCB_TABLE_ENTRY_SIZE_SHFT
	add		r5, r5, r6
	add		r6, r0, TCP_TCB_RX2TX_SHARED_WRITE_OFFSET
	add		r5, r5, r6
	phvwr		p.dma_cmd3_dma_cmd_addr, r5
	phvwri		p.dma_cmd3_dma_cmd_phv_start_addr, TCP_PHV_RX2TX_SHARED_START
	phvwri		p.dma_cmd3_dma_cmd_phv_end_addr, TCP_PHV_RX2TX_SHARED_END
	phvwri		p.dma_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
        phvwri          p.dma_cmd3_dma_cmd_eop, 0
	addi		r4, r4, CAPRI_DMA_COMMAND_SIZE
#endif

tcp_serq_produce:
	sne		c1, k.common_phv_debug_dol, r0
	bcf		[!c1], ring_doorbell
	nop
        phvwri          p.dma_cmd3_dma_cmd_eop, 1
        phvwri          p.dma_cmd3_dma_cmd_wr_fence, 1
        b 		flow_write_serq_process_done
	nop
ring_doorbell:
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid, 0, k.to_s6_serq_pidx)
	
	phvwr		p.dma_cmd4_dma_cmd_addr, r4
        phvwr           p.db_data_index, k.to_s6_serq_pidx

        phvwr           p.db_data_qid, k.common_phv_fid

	phvwri		p.dma_cmd4_dma_cmd_phv_start_addr, TCP_PHV_DB_DATA_START
	phvwri		p.dma_cmd4_dma_cmd_phv_end_addr, TCP_PHV_DB_DATA_END
	phvwri		p.dma_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

        phvwri          p.dma_cmd4_dma_cmd_eop, 1
        phvwri          p.dma_cmd4_dma_cmd_wr_fence, 1
	
flow_write_serq_process_done:
        CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_rx_stats_stage7_start)
	nop.e
	nop

