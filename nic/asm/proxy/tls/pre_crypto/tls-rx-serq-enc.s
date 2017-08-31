/*
 * 	Implements the receipt of tls encrypt request from SERQ 
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct tx_table_s2_t0_k k       ;
struct phv_ p	;
struct tx_table_s2_t0_tls_rx_serq_d d	;
	
%%
    .param      tls_serq_consume_process_start
        
tls_rx_serq_enc_process_start:
	CAPRI_CLEAR_TABLE0_VALID

tls_rx_serq_enc_process:
	/* ENQ_DESC(*etlsp, enc, md->seqe.desc); */
	/* if (TAIL_DESC(cb,name) == NULL) { */
	seq		    c1, d.qtail, r0
	/*    TAIL_DESC(cb, name) = desc; */
	bcf		[c1], no_dma_cmd_enc
	nop
	/*    TAIL_DESC(cb, name) = desc; */
	tblwr.c1	d.qtail, k.to_s2_idesc
    add         r1, r0, k.to_s2_idesc
dma_cmd_enc_desc_entry_last:
	/* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
	
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		    r5, r5, d.qtail
	phvwr		p.dma_cmd0_dma_cmd_addr, r5

	phvwr		p.aol_next_addr, k.to_s2_idesc

    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(aol_next_addr)
	phvwri		p.dma_cmd0_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(aol_next_addr)
        
	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_eop, 0

no_dma_cmd_enc:        
	/* etlsp->enc_nxt.desc = TAIL_DESC(*etlsp, enc); */
	tblwr		d.nxt_desc, k.to_s2_idesc
    phvwr       p.to_s5_cur_tls_data_len, d.cur_tls_data_len

   	tblwr.c1	d.qtail, k.to_s2_idesc
	tblwr.c1	d.qhead, k.to_s2_idesc


table_read_serq_consume:
	CAPRI_NEXT_TABLE0_READ(k.tls_global_phv_fid, TABLE_LOCK_DIS, tls_serq_consume_process_start,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)
        


tls_rx_serq_process_enc_done:
	nop.e
	nop


