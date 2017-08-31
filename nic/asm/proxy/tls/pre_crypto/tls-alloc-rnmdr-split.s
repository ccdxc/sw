/*
 *	Implements the desc alloc stage of the RxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-constants.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	odesc				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				 : 32 ;
	RNMDR_alloc_idx			 : RING_INDEX_WIDTH ;
	write_serq			 : 1		   ;

	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;

	desc                             : ADDRESS_WIDTH ;
	desc_aol0_addr                   : 30 ;
	desc_aol0_offset                 : 16 ;
	desc_aol0_len                    : 16 ;

	desc_aol1_addr                   : 30 ;
	desc_aol1_offset                 : 16 ;
	desc_aol1_len                    : 16 ;
	
	enc_flow                         : 1 ;

};
struct p_struct p;
struct k_struct k;
struct d_struct d;

%%
	
tls_alloc_rnmdr_split_process_start:
	phvwr		p.pending_rx_serq, k.pending_rx_serq
	phvwr		p.pending_rx_brq, k.pending_rx_brq
	phvwr		p.enc_flow, k.enc_flow
	

	phvwr		p.sdesc, d.odesc

dma_cmd_sdesc_entry0:
	addi		r5, r0, NIC_DESC_ENTRY_0_OFFSET
	add		r5, r5, d.odesc
	phvwr		p.dma_cmd3_addr, r5

	phvwr		p.desc_aol1_addr, k.desc_aol0_addr
	phvwr		p.desc_aol1_offset, k.desc_aol1_offset
	phvwr		p.desc_aol1_len, k.desc_aol1_len

	phvwri		p.dma_cmd3_pad, TLS_PHV_AOL1_START
	phvwri		p.dma_cmd3_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd3_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

dma_cmd_idesc_entry0:
	addi		r5, r0, NIC_DESC_ENTRY_0_OFFSET
	add		r5, r5, k.desc
	phvwr		p.dma_cmd2_addr, r5

	phvwr		p.desc_aol0_addr, k.desc_aol0_addr
	phvwr		p.desc_aol0_offset, k.desc_aol0_offset
	phvwr		p.desc_aol0_len, k.desc_aol0_len

	phvwri		p.dma_cmd2_pad, TLS_PHV_AOL_START
	phvwri		p.dma_cmd2_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd2_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

/* 
dma_cmd_page_refcnt_inc:
	
table_read_alloc_rnmpr:
	TLS_READ_IDX(RNMPR_ALLOC_IDX, TABLE_TYPE_RAW, tls_alloc_rnmpr_process)
*/
table_read_page_ctl:
	add		r2, r0, k.desc_aol0_addr
	TLS_READ_ADDR(r2, TABLE_TYPE_RAW, tls_update_page_ctl_process)

tls_alloc_rnmdr_process_done:
	nop.e
	nop
