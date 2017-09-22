/*
 * 	Implements the processing of Packet Descriptor
 *  Stage 3, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s3_t0_k k;
struct phv_             p;
struct tx_table_s3_t0_d d;

#define D   d.u.tls_serq_process_d
	
%%
	.param		tls_dec_bld_barco_req_process
    .param      tls_dec_read_header_process
        
tls_dec_rx_serq_process:

	/* ENQ_DESC(*dtlsp, dec, md->seqe.desc); */
	/* if (TAIL_DESC(cb,name) == NULL) { */
	seq		c1, D.qtail, r0
	/*    TAIL_DESC(cb, name) = desc; */
	bcf		[c1], no_dma_cmd
	nop
dma_cmd_dec_desc_entry_last:
	/* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
	
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		    r5, r5, D.qtail
	phvwr		p.dma_cmd0_dma_cmd_addr, r5

	phvwr		p.odesc_next_addr, k.to_s3_idesc

    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(odesc_next_addr)
	phvwri		p.dma_cmd0_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(odesc_next_addr)


	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_eop, 0

no_dma_cmd:	
	tblwr.c1	D.qtail, k.to_s3_idesc
	tblwr.c1	D.qhead, k.to_s3_idesc
	
	/* dtlsp->dec_nxt.desc = TAIL_DESC(*dtlsp, dec); */
	tblwr		D.nxt_desc, k.to_s3_idesc

	/* Check if the next tls header is starting in this descriptor
	 * data, if so trigger a read to get that tls header and the 
	 * offset for the next tls header in the data stream 
	 */
	/* if (dtlsp->next_tls_hdr_offset < md->desc.data_len) { */
    add         r5, r0, k.{s2_s3_t0_phv_idesc_aol0_len_sbit0_ebit7...s2_s3_t0_phv_idesc_aol0_len_sbit8_ebit31}
	slt		    c1, D.next_tls_hdr_offset, r5
	bcf		    [!c1], tls_no_read_header
	nop
	phvwr		p.tls_global_phv_next_tls_hdr_offset, D.next_tls_hdr_offset
	add		    r2, r0, k.s2_s3_t0_phv_idesc_aol0_addr
	add		    r3, r0, k.s2_s3_t0_phv_idesc_aol0_offset
	add		    r3, r3, k.tls_global_phv_next_tls_hdr_offset
	add		    r2, r2, r3
#if 1
    /* FIXME: Workaround to DoL packet injection issue */
    addi        r2, r2, 4
#endif

table_read_tls_header:	
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_header_process, r2, TABLE_SIZE_512_BITS)
	b		    tls_dec_rx_serq_process_done
	nop
	/* md->next_tls_hdr_offset = dtlsp->next_tls_hdr_offset; */
	/* tls_read_header_process(&phv); */

tls_no_read_header:
	/* if (dtlsp->next_tls_hdr_offset == md->desc.data_len) { */
	seq		    c1, D.next_tls_hdr_offset, r5
	
	bcf 		[!c1], tls_dec_rx_serq_process_done
	nop

table_read_BLD_BARCO_DEC_REQ:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_bld_barco_req_process,
                           k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                           TABLE_SIZE_512_BITS)


tls_dec_rx_serq_process_done:
	nop.e
	nop


